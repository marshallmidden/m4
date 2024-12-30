/* $Id: PI_ClientPersistentData.c 156532 2011-06-24 21:09:44Z m4 $ */
/**
******************************************************************************
**
**  @file       PI_CLIENTPersistentData.c
**
**  @brief      Packet Interface for CLIENT Persistent Data commands
**
**  Allows persistent data from clients to be saved and
**  retrieved from the CCB
**
**  Copyright (c) 2006, 2009 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include "PacketInterface.h"

#include "debug_files.h"
#include "errorCodes.h"
#include "ipc_common.h"
#include "ipc_sendpacket.h"
#include "ipc_packets.h"
#include "logview.h"
#include "quorum_utils.h"
#include "rm_val.h"
#include "LOG_Defs.h"
#include "PI_ClientPersistent.h"
#include "PI_CmdHandlers.h"
#include "slink.h"
#include "XIOPacket.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"
#include "CT_history.h"

/*****************************************************************************
 ** Private defines
 *****************************************************************************/
#define LOCK_RESET            (0)
#define LOCK_SET              (1)
#define INITIAL_RECORD_LOCKFD (0)
#define TRUNC_SET             (1)
#define TRUNC_RESET           (0)
#define NORMAL_PHASE          (0)
#define DEFAULT_LOCK_TIMEOUT  (60)      /* default timeout for a lock */
#define MAX_PERSISTENT_SPACE  (50*1024) /* availabe space in kilo bytes */
#define PDC_IPC_SEND_TMO      120000    /* Timeout for IPC packet */
#define BLK_SIZ               1024      /* use 1K chunks for file copy */
#define FCOPY_SUCCESS         0
#define FCOPY_FAILURE         -1
#define MAX_PATH_NAME_LEN    324        /* length of pathname + record name */
#define PDATA_MGT_FNAME "/opt/xiotech/xiodata/MgtStructure.mmf"
#define PDATA_DIR "/opt/xiotech/xiodata/"
#define SETDEFAULTPATH(fpath) strcpy(fpath,"/opt/xiotech/xiodata/")

/*
** Data structure for lock timers for a record
*/
typedef struct _CLIENT_TIMER
{
    INT32       lockfd;
    UINT16      timeout;        /* timeout value in seconds  */
    UINT16      rsvd;
    UINT32      locktime;       /* time when lock is applied */
} CLIENT_TIMER;

/*
** Data structure for record management
*/
typedef struct nodetype
{
    PI_CLIENT_MGT_STRUCTURE record;
    CLIENT_TIMER client_timer;
} node;

/*****************************************************************************
 ** Private variables
 *****************************************************************************/
static UINT32 latestClientDataTimeStamp = 0;
static UINT32 latestClientDataControllerSN = 0;
static S_LIST *mgtList;
static char timestring[256];

/*****************************************************************************
 ** Public variables - externed in the header file
 *****************************************************************************/
MUTEX       gMgtListMutex;

/*****************************************************************************
 ** Private function prototypes
 *****************************************************************************/
static INT32 StoreMgtStructureList(UINT8);
static INT32 RemoveMgtStructure(char *, UINT8);
static INT32 FindMgtStructure(char *, node **);
static INT32 ListMgtStructures(PI_CLIENT_RECORD_LIST *);
static int  compareRecName(char *, char *);
static INT32 ReadClientRecord(char *, UINT32, UINT32 *, UINT8 *, INT32);
static INT32 DeleteClientRecord(char *);
static INT32 ClientApplyLock(XIO_PACKET *, node *);
static INT32 RemoveRecordLock(node *);
static INT32 ClientRemoveLock(PI_CLIENT_DATA_CONTROL_REQ *);
static INT32 ValidateLock(INT32, node *);
static INT32 IsRecordLocked(XIO_PACKET *, node *);
static void setRecordTimeout(node *, UINT16);
static INT32 TruncateClientRecord(char *, UINT32, UINT32 *);
static void resetRecordTimeout(PI_CLIENT_DATA_CONTROL_REQ *);
static INT32 SendToSlavesPersistentData(UINT32, XIO_PACKET *);
static INT32 ClientPersistentDataPostProcess(UINT32, XIO_PACKET *);
static INT32 CopyMasterClientRecordToSlave(UINT32 , char *, UINT32);
static INT32 DeleteAllClientRecords(void);

static INT32 CreateOptionHandler(XIO_PACKET *, XIO_PACKET *);
static INT32 ReadOptionHandler(XIO_PACKET *, XIO_PACKET *);
static INT32 ValidateClientParameters(XIO_PACKET *, node **);
static INT32 WriteOptionHandler(XIO_PACKET *, XIO_PACKET *);
static INT32 ListOptionHandler(XIO_PACKET *, XIO_PACKET *);
static INT32 NoOptionHandler(XIO_PACKET *, XIO_PACKET *);
static INT32 RemoveOptionHandler(XIO_PACKET *, XIO_PACKET *);
static INT32 BackupRecord(node *, node *);
static INT32 RollbackRecord(node);
static INT32 CleanupRecord(node);
static INT32 CreateMgtStructureList(void);

/*****************************************************************************
 ** Code Start
 *****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    appendDefaultPath
**
** Description: local function appends the defaultpath for the record name
**
** Inputs:      pathname   - pointer to final path after appending defaultpath
**              fname      - file name to be appended after defaultpath
**
** Returns:     none
**
** WARNING:     pathname must have sufficient data store to store the totalpath
**
**--------------------------------------------------------------------------*/
void appendDefaultPath(char *pathname, char *fname)
{
    SETDEFAULTPATH(pathname);
    strcat(pathname, fname);
    return;
}


/*----------------------------------------------------------------------------
** Function:    compareRecName
**
** Description: compares two strings and return the result
**
** Inputs:      a   - constant void pointer
**              b   - constant void pointer
**
** Returns:     strcmp value (-1/0/1)
**
**--------------------------------------------------------------------------*/
static int compareRecName(char *a, char *b)
{
    return (strcmp(a, b));
}


/*----------------------------------------------------------------------------
** Function:    CreateNode
**
** Description: allocates space for a node
**
** Inputs:      newnode - address of the new node is returned here
**
** Returns:     none
**
** WARNING:     modified on 11/1/06
**--------------------------------------------------------------------------*/
static void createNode(node **newnode)
{
    *newnode = MallocWC(sizeof(node));
}


/**
******************************************************************************
**
**  @brief      Creates a linked list from the management structure file
**
**  @param      none
**
**  @return     PI_GOOD on success, PI_ERROR on failure
**
** NOTE: Done at initialization, so no mutex is needed.
**
******************************************************************************
**/
INT32 CreateMgtStructureList(void)
{
    int     fd;
    INT32   numBytes;
    node    *newnode;
    UINT32  timeStamp;

    mgtList = CreateList();     /* Initialize the management structure */

    /* Open the management file for constructing the list */
    fd = open(PDATA_MGT_FNAME, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        dprintf(DPRINTF_DEFAULT, "%s: open not successful\n", __func__);
        return PI_ERROR;
    }

    dprintf(DPRINTF_DEFAULT, "%s: open file success\n", __func__);

    /* Read the timestamp just to advance to the start of record list */
    read(fd, &timeStamp, sizeof(timeStamp));

    for (;;)
    {
        createNode(&newnode);
        numBytes = read(fd, newnode, sizeof(*newnode));
        if (numBytes <= 0)      /* If no node read, exit loop */
        {
            break;
        }

        /* Clear the old lock information and add the record to linked list */
        newnode->client_timer.lockfd = 0;
        newnode->client_timer.timeout = 0;
        newnode->client_timer.rsvd = 0;
        newnode->client_timer.locktime = 0;
        AddElement(mgtList, newnode);
    }

    Free(newnode);          /* Free the last node, which was not used */

    dprintf(DPRINTF_DEFAULT, "%s: i=%d MgtStructure file processed\n",
                __func__, NumberOfItems(mgtList));
    Close(fd);

    return PI_GOOD;
}


/*----------------------------------------------------------------------------
** Function:    StoreMgtStructureList
**
** Description: store the mgtList to MgtStructure file
**
** Inputs:     isResync -- tells the function call is from resync
**                         If so do not update the time stamp
**                         instead get the time stamp of the MgtStructure.mmf
**                         by latestClientTimeStamp global.
**
** Returns:     PI_GOOD if success
**              PI_ERROR otherwise
**
** ATTENTION:   gMgtListMutex must be locked before calling.
**
**--------------------------------------------------------------------------*/
static INT32 StoreMgtStructureList(UINT8 isResync)
{
    int         fd;
    node       *nodedata;
    int         nrecords = 0;
    UINT32      timeStamp = 0;
    INT32       retval = PI_GOOD;

    if ((fd = open(PDATA_MGT_FNAME, (O_RDWR), (S_IRUSR | S_IWUSR))) < 0)
    {
        /* If the file does not already exist, create it. */
        if ((fd = open(PDATA_MGT_FNAME, (O_RDWR | O_CREAT | O_EXCL), (S_IRUSR | S_IWUSR))) > 0)
        {
            dprintf(DPRINTF_DEFAULT, "%s: file created fd = %d\n", __func__, fd);
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "%s: file %s can not be created\n", __func__, PDATA_MGT_FNAME);
            retval = PI_ERROR;
        }
    }

    if (retval == PI_GOOD)
    {
        if (isResync)
        {
            timeStamp = latestClientDataTimeStamp;
            dprintf(DPRINTF_DEFAULT, "%s latestClient = %d\n", __func__, latestClientDataTimeStamp);
        }
        else
        {
            timeStamp = RTC_GetLongTimeStamp();
        }

        write(fd, &timeStamp, sizeof(UINT32));

        /* NOTE: Lock already gotten. */

        /* Write each node to the file */
        SetIterator(mgtList);
        while ((nodedata = Iterate(mgtList)) != NULL)
        {
            /* Write each record to management file */
            write(fd, nodedata, sizeof(*nodedata));
            nrecords++;
        }

        /* Truncate the file to discard extra records left in the previous list. */
        if (nrecords)
        {
            ftruncate(fd, sizeof(UINT32) + (nrecords * sizeof(*nodedata)));
        }

        Close(fd);
    }
    return retval;
}


/*----------------------------------------------------------------------------
** Function:    AddMgtStructure
**
** Description: Add record to Mgt Structure file and list
**
** Inputs:      recname  -- record name
**              reclen   -- record length
**              isResync -- whether called during resync
**
** Returns:     PI_GOOD on success
**              PI_ERROR on failure
**
**--------------------------------------------------------------------------*/
INT32 AddMgtStructure(char *recname, UINT32 reclen, UINT8 isResync)
{
    node       *clientNode;
    INT32       retval = PI_GOOD;

    ccb_assert(recname != NULL, recname);

    /* Copy the record to local variable */
    clientNode = MallocWC(sizeof(*clientNode));
    strcpy((char *)clientNode->record.recordName, recname);
    clientNode->record.recordLength = reclen;
    clientNode->record.timeStamp = RTC_GetLongTimeStamp();
    GetTimeString(&(clientNode->record.timeStamp), timestring);

    /* The record lock fd is initialized to unlock mode */
    clientNode->record.recordLocked = LOCK_RESET;

    /* Initialize the the lockfd to zero */
    clientNode->client_timer.lockfd = INITIAL_RECORD_LOCKFD;

    /* Add the record to the list */
    (void)LockMutex(&gMgtListMutex, MUTEX_WAIT);
    AddElement(mgtList, clientNode);

    /* Update the mgt file */
    retval = StoreMgtStructureList(isResync);
    UnlockMutex(&gMgtListMutex);

    return retval;
}


/*----------------------------------------------------------------------------
** Function:    RemoveMgtStructure
**
** Description: removes a record from Mgt Structure file and list
**
** Inputs:      recname - recordname to be removed from the list
**              isResync -- whether called during resync
**
** Returns:     PI_GOOD on success
**              PI_ERROR on failure
**
**--------------------------------------------------------------------------*/
static INT32 RemoveMgtStructure(char *recname, UINT8 isResync)
{
    node       *ptr;
    INT32       retval = PI_GOOD;

    ccb_assert(recname != NULL, recname);

    (void)LockMutex(&gMgtListMutex, MUTEX_WAIT);
    ptr = RemoveElement(mgtList, recname, (void *)&compareRecName);

    /* If record is successfully removed then update to the file */
    if (ptr)
    {
        /* Update the mgt file */
        retval = StoreMgtStructureList(isResync);
        UnlockMutex(&gMgtListMutex);

        /* Free the removed node */
        Free(ptr);
    }
    else
    {
        UnlockMutex(&gMgtListMutex);

        dprintf(DPRINTF_DEFAULT, "%s: remove node error\n", __func__);
        retval = PI_ERROR;
    }

    return retval;
}


/*----------------------------------------------------------------------------
** Function:    UpdateMgtStructure
**
** Description: Updates the record length field in the management table
**              file
**
** Inputs:      recname  - recordname to be updated in list
**              reclen   - record length to be updated
**              isResync - whether called during resync
**
** Returns:     PI_GOOD on success
**              PI_ERROR on failure
**
**--------------------------------------------------------------------------*/
INT32 UpdateMgtStructure(char *recname, UINT32 reclen, UINT8 isResync)
{
    node       *ptr;
    INT32       retval = PI_GOOD;

    ccb_assert(recname != NULL, recname);

    /* Search the record to update the length in the list */
    (void)LockMutex(&gMgtListMutex, MUTEX_WAIT);
    ptr = FindElement(mgtList, recname, (void *)&compareRecName);
    if (!ptr)
    {
        UnlockMutex(&gMgtListMutex);
        dprintf(DPRINTF_DEFAULT, "%s: No record found\n", __func__);
        return PI_ERROR;
    }

    /* Update the length of the record */
    ptr->record.recordLength = reclen;
    ptr->record.timeStamp = RTC_GetLongTimeStamp();
    GetTimeString(&(ptr->record.timeStamp), timestring);

    retval = StoreMgtStructureList(isResync);
    UnlockMutex(&gMgtListMutex);
    if (retval == PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "%s: RECORD MODIFIED on %s\n", __func__, timestring);
    }
    return retval;
}


/*----------------------------------------------------------------------------
** Function:    FindMgtStructure
**
** Description: Checks whether the record name is in the management list
**              or not
**
** Inputs:      recname - recordname to be found in the list
**              record_node - found record address or NULL if not found
**
** Returns:     PI_GOOD -- if record is found
**              PDATA_RECORD_NOT_FOUND -- record not found
**
** ATTENTION:   gMgtListMutex must be locked before calling.
**
**--------------------------------------------------------------------------*/
static INT32 FindMgtStructure(char *recname, node **record_node)
{
    node       *ptr;
    INT32       retval = PI_GOOD;

    ccb_assert(recname != NULL, recname);

    /* Search the record to update the length in the list */
    ptr = FindElement(mgtList, recname, (void *)&compareRecName);
    if (ptr)
    {
        *record_node = ptr;
        retval = PI_GOOD;
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "%s: No matching record %s found\n", __func__, recname);
        *record_node = NULL;
        retval = PDATA_RECORD_NOT_FOUND;
    }

    return retval;
}


/*----------------------------------------------------------------------------
** Function:    ListMgtStructures
**
** Description: Get the list of all entries in management table
**              !!! gMgtListMutex must be locked before calling !!!
**
** Inputs:      pList - Place holder for the list of records
**
**
** Returns:     PI_GOOD on success
**              PI_ERROR on failure
**
** ATTENTION:   gMgtListMutex must be locked before calling.
**
**--------------------------------------------------------------------------*/
static INT32 ListMgtStructures(PI_CLIENT_RECORD_LIST *pList)
{
    node       *nodedata;
    INT32       i = 0;
    UINT32      totalsize = 0;
    char        pathname[MAX_PATH_NAME_LEN];
    INT32       retval = PI_GOOD;

    ccb_assert(pList != NULL, pList);

    /* Add the size of MgtStrucure file to the total size */
    if (PI_ERROR == GetClientRecordSize(PDATA_MGT_FNAME, &totalsize))
    {
        dprintf(DPRINTF_DEFAULT, "%s: got record size error\n", __func__);
        retval = PI_ERROR;
    }
    else
    {
        /* Traverse all records */
        SetIterator(mgtList);
        i = 0;
        while ((nodedata = Iterate(mgtList)) != NULL)
        {
            /*
             * Get the next record.
             * Copy the record parameters to the list.
             */
            strcpy((char *)pList->records[i].recordName,
                   (char *)nodedata->record.recordName);

            /* Append the default path to the record name for reading from file */
            appendDefaultPath(pathname, (char *)nodedata->record.recordName);

            /*
             * Get each record (file) size and add to totalsize
             * to calculate the freespace
             */
            totalsize += nodedata->record.recordLength;
            pList->records[i].recordLength = nodedata->record.recordLength;
            pList->records[i].recordLocked = nodedata->record.recordLocked;
            pList->records[i].timeStamp = nodedata->record.timeStamp;
            i++;
        }
        pList->freeSpace = (MAX_PERSISTENT_SPACE - (totalsize / 1024));
    }
    return retval;
}


/*----------------------------------------------------------------------------
** Function:    RemoveRecordLock
**
** Description: Removes the lock for the record if there was a lock. If lock is
**              existing and different from the previous, then error is returned.
**
** Inputs:      pRecord - record
**
** Returns:     PI_GOOD on success, PI_ERROR on failure
**
** ATTENTION:   gMgtListMutex must be locked before calling.
**
**--------------------------------------------------------------------------*/
static INT32 RemoveRecordLock(node *pRecord)
{
    INT32       retval = PI_GOOD;

    ccb_assert(pRecord != NULL, pRecord);

    /* Record is locked then unlock */
    if (pRecord->record.recordLocked)
    {
        pRecord->record.recordLocked = LOCK_RESET;
        pRecord->client_timer.lockfd = INITIAL_RECORD_LOCKFD;

        /* Update the mgt table to file */
        retval = StoreMgtStructureList(NORMAL_PHASE);
    }

    return retval;
}


/*----------------------------------------------------------------------------
** Function:    ClientRemoveLock
**
** Description: Removes the lock for the record if there was a lock. If lock is
**              existing and different from the previous, then error is returned.
**
** Inputs:      pDataReqP - PI request packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ClientRemoveLock(PI_CLIENT_DATA_CONTROL_REQ *pDataReqP)
{
    INT32       retval = PI_GOOD;
    node       *pRecord = NULL;

    ccb_assert(pDataReqP != NULL, pDataReqP);

    /* Get the record node in MGT list */
    (void)LockMutex(&gMgtListMutex, MUTEX_WAIT);
    retval = FindMgtStructure((char *)pDataReqP->recordName, &pRecord);

    if (PI_GOOD == retval)
    {
        RemoveRecordLock(pRecord);
    }
    UnlockMutex(&gMgtListMutex);

    return retval;
}


/*----------------------------------------------------------------------------
** Function:    RemoveAllLocks
**
** Description: Removes the locks for all records if there was a lock.
**
** Inputs:      lockfd - unlock request origin client fd
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
void RemoveAllLocks(INT32 lockfd)
{
    node       *nodedata;

    /*
     * Lock the mutex on the management list to prevent others from using
     * the iterator while it is in use here.
     */
    (void)LockMutex(&gMgtListMutex, MUTEX_WAIT);

    /* Get the head pointer of management list */
    SetIterator(mgtList);

    /* Check all the records with the lockfd */
    while ((nodedata = Iterate(mgtList)) != NULL)
    {
        /* Remove the record lock which is same as lockfd */
        if (nodedata->client_timer.lockfd == lockfd)
        {
            nodedata->record.recordLocked = LOCK_RESET;
            nodedata->client_timer.lockfd = INITIAL_RECORD_LOCKFD;
        }
    }

    /* Update the mgt file */
    StoreMgtStructureList(NORMAL_PHASE);

    /* Done using the iterator on the management list so unlock the mutex */
    UnlockMutex(&gMgtListMutex);
}


/*----------------------------------------------------------------------------
** Function:    ValidateLock()
**
** Description: validates lock request
**
** Inputs:      request_lockfd - socket fd of lock requested client
**              pRecord - record node
**
** Returns:     PDATA_LOCK_REJECTED - rejecting the command
**              PI_GOOD
**
**--------------------------------------------------------------------------*/
static INT32 ValidateLock(INT32 request_lockfd, node *pRecord)
{
    INT32       retval = PI_GOOD;

    /*
     * A lock is requested for. See if it is already locked.
     * If the existing lock timed out, then clear it before
     * validating the new lock.
     */
    if (pRecord->client_timer.lockfd)
    {
        if (pRecord->client_timer.timeout <
            (RTC_GetLongTimeStamp() - pRecord->client_timer.locktime))
        {
            /* Lock expired, clear the lock */
            pRecord->record.recordLocked = LOCK_RESET;
            pRecord->client_timer.lockfd = INITIAL_RECORD_LOCKFD;
        }
    }

    /* Now see if lock can be applied */
    if ((pRecord->client_timer.lockfd)
        && (pRecord->client_timer.lockfd != request_lockfd))
    {
        /* reject the lock request */
        dprintf(DPRINTF_DEFAULT, "%s: lock is rejected\n", __func__);
        retval = PDATA_LOCKED_ERROR;
    }
    return retval;
}


/*----------------------------------------------------------------------------
** Function:    ValidateRecordName
**
** Description: Check whether the recordname is valid or not
**
** Inputs:      recname -- record name
**
** Returns:     PDATA_INVALIDATE_RECORD - if not a valid record name
**              PI_GOOD otherwise
**
**--------------------------------------------------------------------------*/
static INT16 ValidateRecordName(char *recname)
{
    char       *name;

    if ((recname) && (*recname != '\0'))
    {
        name = recname;
    }
    else
    {
        return PDATA_INVALID_RECORD;
    }

    while (*name != 0)
    {
        if (((*name >= 'a') && (*name <= 'z')) ||
            ((*name >= 'A') && (*name <= 'Z')) ||
            ((*name >= '0') && (*name <= '9')) ||
            (*name == '-') || (*name == '_') || (*name == '.'))
        {
            name++;
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "%s: found bad character '%c' (0x%x) in filename %s\n",
                    __func__, *name, *name, recname);
            return PDATA_INVALID_RECORD;
        }
    }
    return PI_GOOD;
}


/*----------------------------------------------------------------------------
** Function:    CreateOptionHandler()
**
** Description: Handle the create persistent record option
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD on success
**              PI_ERROR on failure
**              Other Possible Error codes:
**                       PDATA_DUPLICATE_RECORD
**                       PDATA_FILESYSTEM_ERROR
**                       PDATA_INVALID_RECORD
**--------------------------------------------------------------------------*/
static INT32 CreateOptionHandler(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_CLIENT_DATA_CONTROL_REQ *pDataReqP = NULL;
    PI_CLIENT_DATA_CONTROL_RSP *pDataRspP = NULL;
    INT32       errorCode = PI_GOOD;
    UINT8       status = PI_GOOD;
    node       *pRecord;
    char        pathname[MAX_PATH_NAME_LEN];
    INT32       replStatus;

    pDataReqP = (PI_CLIENT_DATA_CONTROL_REQ *)pReqPacket->pPacket;

    ccb_assert(pDataReqP != NULL, pDataReqP);
    /*
     * Validate the record name. The name should be a
     * regular expression [a-zA-Z0-9]+
     */
    if (PI_GOOD == ValidateRecordName((char *)pDataReqP->recordName))
    {
        /* Find and get the record from Mgt table */
        (void)LockMutex(&gMgtListMutex, MUTEX_WAIT);
        if (PI_GOOD != FindMgtStructure((char *)pDataReqP->recordName, &pRecord))
        {
            UnlockMutex(&gMgtListMutex);

            /* Append the default path to the record name for creating a file */
            appendDefaultPath(pathname, (char *)pDataReqP->recordName);

            /* create client record file with the path name */
            errorCode = CreateClientRecord(pathname);
            if (errorCode == PI_GOOD)
            {
                /* Add the record to management structure */
                errorCode = AddMgtStructure((char *)pDataReqP->recordName, 0, NORMAL_PHASE);
                if (errorCode == PI_GOOD)
                {
                    /* Send the async change event for create record */
                    errorCode = ClientPersistentDataPostProcess(IPC_PERSISTENT_CREATE, pReqPacket);
                    if ((errorCode != PI_GOOD) && (errorCode != PDATA_SLAVE_DEAD))
                    {
                        /* Roll back the operation */
                        replStatus = DeleteClientRecord(pathname);

                        /* Remove the record from the management table */
                        RemoveMgtStructure((char *)pDataReqP->recordName, NORMAL_PHASE);
                    }
                }
                else
                {
                    replStatus = DeleteClientRecord(pathname);
                    RemoveMgtStructure((char *)pDataReqP->recordName, NORMAL_PHASE);
                    status = PI_ERROR;
                }
            }
            else
            {
                dprintf(DPRINTF_DEFAULT, "%s: CREAT_CLIENT_RECORD can not create file\n", __func__);
                status = PI_ERROR;
            }
        }
        else
        {
            UnlockMutex(&gMgtListMutex);
            dprintf(DPRINTF_DEFAULT, "%s: CREAT_CLIENT_RECORD record exists\n", __func__);
            errorCode = PDATA_DUPLICATE_RECORD;
            status = PI_ERROR;
        }
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "%s: %s -Not a Valid recordname\n", __func__, pDataReqP->recordName);
        status = PI_ERROR;
        errorCode = PDATA_INVALID_RECORD;
    }

    if (errorCode != PI_GOOD)
    {
        status = PI_ERROR;
    }

    pDataRspP = MallocWC(sizeof(*pDataRspP));
    pRspPacket->pPacket = (UINT8 *)pDataRspP;
    pRspPacket->pHeader->length = sizeof(*pDataRspP);
    pDataRspP->option = pDataReqP->option;

    /* Fill out response packet before returning */
    pRspPacket->pHeader->status = status;
    pRspPacket->pHeader->errorCode = errorCode;

    return status;
}


/*----------------------------------------------------------------------------
** Function:    ReadOptionHandler()
**
** Description: Handle the read persistent record option
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD on success
**              PI_ERROR on failure
**              Possible error codes:
**
**--------------------------------------------------------------------------*/
static INT32 ReadOptionHandler(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_CLIENT_DATA_CONTROL_RSP *pDataRspP = NULL;
    PI_CLIENT_DATA_CONTROL_REQ *pDataReqP = NULL;
    UINT32      readlen = 0;
    UINT8       trunc1 = 0;
    INT32       status = PI_GOOD;
    INT32       retval = PI_GOOD;
    INT32       replStatus = PI_GOOD;
    char        pathname[MAX_PATH_NAME_LEN];
    UINT32      requnlock = 0;
    UINT8       backedup = FALSE;
    node        backupnode;
    node       *pRecord;

    pDataReqP = (PI_CLIENT_DATA_CONTROL_REQ *)pReqPacket->pPacket;

    ccb_assert(pDataReqP != NULL, pDataReqP);
    /*
     * Validate the record name. The name should be a
     * regular expression [a-zA-Z0-9]+
     */
    (void)LockMutex(&gMgtListMutex, MUTEX_WAIT);
    retval = ValidateClientParameters(pReqPacket, &pRecord);

    /* Backup the record before the operation */
    trunc1 = (pDataReqP->flags & CLIENT_FLAG_TRUNC) ? TRUNC_SET : TRUNC_RESET;
    if ((PI_GOOD == retval) && trunc1)
    {
        retval = BackupRecord(pRecord, &backupnode);
        if (PI_GOOD == retval)
        {
            backedup = TRUE;
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "WARNING!! Could Not Backup the record before operation. Cannot rollback in case of failure!\n");
        }
    }
    UnlockMutex(&gMgtListMutex);

    if ((PI_GOOD == retval) || (PDATA_CANNOT_BACKUP == retval))
    {
        appendDefaultPath(pathname, (char *)pDataReqP->recordName);
        requnlock = pDataReqP->flags & CLIENT_FLAG_UNLOCK;

        /* If length was given as 0 then read the entire record */
        if (0 == pDataReqP->length)
        {
            /* Append the default path to the record name for reading from file */

            if (PI_ERROR == GetClientRecordSize(pathname, &readlen))
            {
                readlen = 0;
            }
        }
        else
        {
            readlen = pDataReqP->length;
        }

        /* Allocate the memory for the response.  */
        pDataRspP = MallocWC(sizeof(*pDataRspP) + readlen);

        /* Read Client record from the file */
        retval = ReadClientRecord(pathname, pDataReqP->offset, &readlen, pDataRspP->buffer, trunc1);

        /* If trunc1 option is set, then we have to propagate to slave controller */
        if (trunc1 && (PI_GOOD == retval))
        {
            /* Send the read with trunc1 to slave controller */
            retval = ClientPersistentDataPostProcess(IPC_PERSISTENT_READ, pReqPacket);
            if ((retval != PI_GOOD) && (retval != PDATA_SLAVE_DEAD) && (backedup == TRUE))
            {
                /* Roll back the truc operation */
                replStatus = RollbackRecord(backupnode);
                if (replStatus == PI_ERROR)
                {
                    dprintf(DPRINTF_DEFAULT, "WARNING!! Operation Rollback FAILED! You may have inconsistent data in Client Persistent Store\n");
                }
            }
            else if (backedup == TRUE)
            {
                CleanupRecord(backupnode);
            }
        }
        else if (backedup == TRUE)
        {
            CleanupRecord(backupnode);
        }
    }
    else
    {
        /* if response buffer is not created */
        pDataRspP = MallocWC(sizeof(*pDataRspP));
    }

    pDataRspP->length = readlen;
    pRspPacket->pPacket = (UINT8 *)pDataRspP;
    pRspPacket->pHeader->length = sizeof(*pDataRspP) + readlen;

    pDataRspP->option = pDataReqP->option;
    pDataRspP->offset = pDataReqP->offset;

    if (PI_GOOD != retval)
    {
        status = PI_ERROR;
    }
    else
    {
        status = PI_GOOD;
    }

    /* Fill out response packet before returning */
    pRspPacket->pHeader->status = status;
    pRspPacket->pHeader->errorCode = retval;

    /* Process the unlock request */
    if (requnlock)
    {
        /* Remove the lock for the record */
        ClientRemoveLock(pDataReqP);
    }
    else
    {
        /* Reset the lock timer */
        resetRecordTimeout(pDataReqP);
    }
    return status;
}


/*----------------------------------------------------------------------------
** Function:    WriteOptionHandler()
**
** Description: Handle the write persistent record option
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD -- when success
**              PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 WriteOptionHandler(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_CLIENT_DATA_CONTROL_RSP *pDataRspP = NULL;
    PI_CLIENT_DATA_CONTROL_REQ *pDataReqP = NULL;
    UINT32      writelen = 0;
    UINT32      trunc1 = 0;     /* need to decide UINT8 or 32 */
    INT32       status = PI_GOOD;
    INT32       retval = PI_GOOD;
    INT32       replStatus = PI_GOOD;
    char        pathname[MAX_PATH_NAME_LEN];
    UINT32      filelen = 0;
    UINT32      requnlock = 0;
    UINT8       backedup = FALSE;
    node       *pRecord;
    node        backupnode;

    pDataReqP = (PI_CLIENT_DATA_CONTROL_REQ *)pReqPacket->pPacket;

    ccb_assert(pDataReqP != NULL, pDataReqP);
    /*
     * Validate the record name. The name should be a
     * regular expression [a-zA-Z0-9]+
     */
    (void)LockMutex(&gMgtListMutex, MUTEX_WAIT);
    retval = ValidateClientParameters(pReqPacket, &pRecord);

    /* Backup the record before the operation */
    if (PI_GOOD == retval)
    {
        retval = BackupRecord(pRecord, &backupnode);
        if (PI_GOOD == retval)
        {
            backedup = TRUE;
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "WARNING!! Could Not Backup the record before operation. Cannot rollback in case of failure!\n");
        }
    }
    UnlockMutex(&gMgtListMutex);

    if ((PI_GOOD == retval) || (PDATA_CANNOT_BACKUP == retval))
    {
        requnlock = pDataReqP->flags & CLIENT_FLAG_UNLOCK;

        /* Get the length of the data to be written */
        writelen = pDataReqP->length;

        /* Append the default path to the record name for reading from file */
        appendDefaultPath(pathname, (char *)pDataReqP->recordName);

        trunc1 = (pDataReqP->flags & CLIENT_FLAG_TRUNC) ? TRUNC_SET : TRUNC_RESET;

        /* Write the record from offset to length */
        retval = WriteClientRecord(pathname, pDataReqP->offset,
                                   &writelen, pDataReqP->buffer, trunc1);
        if (PI_GOOD == retval)
        {
            /* Update management list and file */
            retval = GetClientRecordSize(pathname, &filelen);

            if (PI_GOOD == retval)
            {
                UpdateMgtStructure((char *)pDataReqP->recordName, filelen, NORMAL_PHASE);

                /* Call the client persistent data post process */
                retval = ClientPersistentDataPostProcess(IPC_PERSISTENT_WRITE, pReqPacket);
                if ((retval != PI_GOOD) && (retval != PDATA_SLAVE_DEAD) && (backedup == TRUE))
                {
                    /* Roll back the truc operation */
                    replStatus = RollbackRecord(backupnode);
                    if (replStatus == PI_ERROR)
                    {
                        dprintf(DPRINTF_DEFAULT, "WARNING!! Operation Rollback FAILED! You may have inconsistent data in Client Persistent Store\n");
                    }
                }
                else if (backedup == TRUE)
                {
                    CleanupRecord(backupnode);
                }
            }
        }
    }

    /* Create the response buffer */
    pDataRspP = MallocWC(sizeof(*pDataRspP));

//    pDataRspP->length = writelen;
    pRspPacket->pPacket = (UINT8 *)pDataRspP;
    pRspPacket->pHeader->length = sizeof(*pDataRspP);
    pDataRspP->option = pDataReqP->option;
    pDataRspP->offset = pDataReqP->offset;

    if (PI_GOOD != retval)
    {
        status = PI_ERROR;
    }
    else
    {
        status = PI_GOOD;
    }

    /* Fill out response packet before returning */
    pRspPacket->pHeader->status = status;
    pRspPacket->pHeader->errorCode = retval;

    /* Process the unlock request */
    if (requnlock)
    {
        /* Remove the lock for the record */
        ClientRemoveLock(pDataReqP);
    }
    else
    {
        /* Reset the lock timer */
        resetRecordTimeout(pDataReqP);
    }
    return status;
}


/*----------------------------------------------------------------------------
** Function:    ListOptionHandler()
**
** Description: Handle the list records option
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD -- when success
**              PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ListOptionHandler(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_CLIENT_DATA_CONTROL_RSP *pDataRspP = NULL;
    PI_CLIENT_DATA_CONTROL_REQ *pDataReqP = NULL;
    INT32       status = PI_GOOD;
    INT32       retval = PI_GOOD;
    UINT32      nRecords = 0;
    UINT32      totallen = 0;
    PI_CLIENT_RECORD_LIST *pList = NULL;

    pDataReqP = (PI_CLIENT_DATA_CONTROL_REQ *)pReqPacket->pPacket;

    ccb_assert(pDataReqP != NULL, pDataReqP);

    /*
     * Lock the mutex on the management list to prevent others from using
     * the iterator or counts while it is in use here.
     */
    LockMutex(&gMgtListMutex, MUTEX_WAIT);

    /* Allocate the memory for the response. */
    nRecords = NumberOfItems(mgtList);

    /* Calculate the length of the buffer needed */
    totallen = (nRecords * sizeof(PI_CLIENT_MGT_STRUCTURE)) + sizeof(PI_CLIENT_RECORD_LIST);

    /* Create the response buffer */
    pDataRspP = MallocWC(sizeof(*pDataRspP) + totallen);

    pList = (PI_CLIENT_RECORD_LIST *)(pDataRspP->buffer);

    /* Get the number of elements */
    pList->count = nRecords;

    /* Update the number of records and space available */
    pList->maxSpace = MAX_PERSISTENT_SPACE;

    /* Get the list of records in the pList */
    retval = ListMgtStructures(pList);

    /* Done using the iterator on the management list so unlock the mutex */
    UnlockMutex(&gMgtListMutex);

    pDataRspP->length = totallen;
    pRspPacket->pPacket = (UINT8 *)pDataRspP;
    pRspPacket->pHeader->length = sizeof(*pDataRspP) + totallen;
    pDataRspP->option = pDataReqP->option;

    if (PI_GOOD != retval)
    {
        status = PI_ERROR;
    }
    else
    {
        status = PI_GOOD;
    }

    /* Fill out response packet before returning */
    pRspPacket->pHeader->status = status;
    pRspPacket->pHeader->errorCode = retval;

    return status;
}


/*----------------------------------------------------------------------------
** Function:    NoOptionHandler()
**
** Description: Handle the no option persistent command
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD -- when success
**              PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 NoOptionHandler(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_CLIENT_DATA_CONTROL_RSP *pDataRspP = NULL;
    PI_CLIENT_DATA_CONTROL_REQ *pDataReqP = NULL;
    UINT32      recordlen = 0;
    UINT8       truncate_lth = 0;
    INT32       status = PI_GOOD;
    INT32       retval = PI_GOOD;
    INT32       replStatus = PI_GOOD;
    char        pathname[MAX_PATH_NAME_LEN];
    UINT32      requnlock = 0;
    node        backupnode;
    node       *pRecord;
    UINT8       backedup = FALSE;

    pDataReqP = (PI_CLIENT_DATA_CONTROL_REQ *)pReqPacket->pPacket;

    ccb_assert(pDataReqP != NULL, pDataReqP);
    /*
     * Validate the record name. The name should be a
     * regular expression [a-zA-Z0-9]+
     */
    (void)LockMutex(&gMgtListMutex, MUTEX_WAIT);
    retval = ValidateClientParameters(pReqPacket, &pRecord);

    /* Backup the record before the operation */
    if ((PI_GOOD == retval) && (pDataReqP->flags & CLIENT_FLAG_TRUNC))
    {
        retval = BackupRecord(pRecord, &backupnode);
        if (PI_GOOD == retval)
        {
            backedup = TRUE;
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "WARNING!! Could Not Backup the record before operation. Cannot rollback in case of failure!\n");
        }
    }
    UnlockMutex(&gMgtListMutex);

    if ((PI_GOOD == retval) || (PDATA_CANNOT_BACKUP == retval))
    {
        requnlock = pDataReqP->flags & CLIENT_FLAG_UNLOCK;

        /* Append the default path to the record name for writing to file */
        if (truncate_lth)
        {
            appendDefaultPath(pathname, (char *)pDataReqP->recordName);
            recordlen = pDataReqP->length;
            retval = TruncateClientRecord(pathname, pDataReqP->offset, &recordlen);
            if (PI_GOOD == retval)
            {
                /* Update management list and file */
                if (PI_GOOD == GetClientRecordSize(pathname, &recordlen))
                {
                    UpdateMgtStructure((char *)pDataReqP->recordName, recordlen, NORMAL_PHASE);

                    /*
                     * Truncate option is set, so we have to propagate to slave controller
                     */

                    /* Send the NOP with trunc to slave controller */
                    retval = ClientPersistentDataPostProcess(IPC_PERSISTENT_NOP, pReqPacket);

                    if ((retval != PI_GOOD) && (retval != PDATA_SLAVE_DEAD) &&
                        (backedup == TRUE))
                    {
                        /* Roll back the truc operation */
                        replStatus = RollbackRecord(backupnode);

                        if (replStatus == PI_ERROR)
                        {
                            dprintf(DPRINTF_DEFAULT, "WARNING!! Operation Rollback FAILED! You may have inconsistent data in Client Persistent Store\n");
                        }
                    }
                    else if (backedup == TRUE)
                    {
                        CleanupRecord(backupnode);
                    }
                }
            }
        }
        else if (backedup == TRUE)
        {
            CleanupRecord(backupnode);
        }
    }

    /* Create the response buffer */
    pDataRspP = MallocWC(sizeof(*pDataRspP));

    pDataRspP->length = recordlen;
    pRspPacket->pPacket = (UINT8 *)pDataRspP;
    pRspPacket->pHeader->length = sizeof(*pDataRspP);
    pDataRspP->option = pDataReqP->option;
    pDataRspP->offset = pDataReqP->offset;

    if (PI_GOOD != retval)
    {
        status = PI_ERROR;
    }
    else
    {
        status = PI_GOOD;
    }

    /* Fill out response packet before returning */
    pRspPacket->pHeader->status = status;
    pRspPacket->pHeader->errorCode = retval;

    /* Process the unlock request */
    if (requnlock)
    {
        /* Remove the lock for the record */
        ClientRemoveLock(pDataReqP);
    }
    else
    {
        /* Reset the lock timer */
        resetRecordTimeout(pDataReqP);
    }
    return status;
}


/*----------------------------------------------------------------------------
** Function:    RemoveOptionHandler()
**
** Description: Handle the remove record persistent command
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD -- when success
**              PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 RemoveOptionHandler(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_CLIENT_DATA_CONTROL_RSP *pDataRspP = NULL;
    PI_CLIENT_DATA_CONTROL_REQ *pDataReqP = NULL;
    UINT32      recordlen = 0;
    INT32       status = PI_GOOD;
    INT32       retval = PI_GOOD;
    INT32       replStatus = PI_GOOD;
    char        pathname[MAX_PATH_NAME_LEN];
    UINT8       backedup = FALSE;
    node        backupnode;
    node       *pRecord;

    pDataReqP = (PI_CLIENT_DATA_CONTROL_REQ *)pReqPacket->pPacket;

    ccb_assert(pDataReqP != NULL, pDataReqP);
    /*
     * Validate the record name. The name should be a
     * regular expression [a-zA-Z0-9]+
     */
    (void)LockMutex(&gMgtListMutex, MUTEX_WAIT);
    retval = ValidateClientParameters(pReqPacket, &pRecord);

    /* Backup the record before the operation */
    if (PI_GOOD == retval)
    {
        retval = BackupRecord(pRecord, &backupnode);

        if (PI_GOOD == retval)
        {
            backedup = TRUE;
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "WARNING!! Could Not Backup the record before operation. Cannot rollback in case of failure!\n");
        }
    }
    UnlockMutex(&gMgtListMutex);

    if ((PI_GOOD == retval) || (PDATA_CANNOT_BACKUP == retval))
    {
        /* Append the default path to the record name for writing to file */
        appendDefaultPath(pathname, (char *)pDataReqP->recordName);

        /* Delete Client record file name */
        retval = DeleteClientRecord(pathname);
        if (PI_GOOD == retval)
        {
            /* Remove the record from the management table */
            RemoveMgtStructure((char *)pDataReqP->recordName, NORMAL_PHASE);
            retval = ClientPersistentDataPostProcess(IPC_PERSISTENT_REMOVE, pReqPacket);
            if ((retval != PI_GOOD) && (retval != PDATA_SLAVE_DEAD) && (backedup == TRUE))
            {
                /* Roll back the truc operation */
                replStatus = RollbackRecord(backupnode);
                if (replStatus == PI_ERROR)
                {
                    dprintf(DPRINTF_DEFAULT, "WARNING!! Operation Rollback FAILED! You may have inconsistent data in Client Persistent Store\n");
                }
            }
            else if (backedup == TRUE)
            {
                CleanupRecord(backupnode);
            }
        }
    }

    /* Create the response buffer */
    pDataRspP = MallocWC(sizeof(*pDataRspP));

    pDataRspP->length = recordlen;
    pRspPacket->pPacket = (UINT8 *)pDataRspP;
    pRspPacket->pHeader->length = sizeof(*pDataRspP);
    pDataRspP->option = pDataReqP->option;

    if (PI_GOOD != retval)
    {
        status = PI_ERROR;
    }
    else
    {
        status = PI_GOOD;
    }

    /* Fill out response packet before returning */
    pRspPacket->pHeader->status = status;
    pRspPacket->pHeader->errorCode = retval;

    return status;
}


/*----------------------------------------------------------------------------
** Function:    PI_ClientPersistentDataControl()
**
** Description: Write or read to persistent Data
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_ClientPersistentDataControl(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_CLIENT_DATA_CONTROL_REQ *pDataReqP = NULL;
    PI_CLIENT_DATA_CONTROL_RSP *pDataRspP = NULL;
    INT32       rc = PI_GOOD;

    pDataReqP = (PI_CLIENT_DATA_CONTROL_REQ *)pReqPacket->pPacket;

    /* Process the command based on request */
    switch ((pDataReqP->option & 0x0F))
    {
        case CLIENT_OPTION_CREATE_RECORD:
            rc = CreateOptionHandler(pReqPacket, pRspPacket);
            break;

        case CLIENT_OPTION_REMOVE_RECORD:
            rc = RemoveOptionHandler(pReqPacket, pRspPacket);
            break;

        case CLIENT_OPTION_READ_RECORD:
            rc = ReadOptionHandler(pReqPacket, pRspPacket);
            break;

        case CLIENT_OPTION_WRITE_RECORD:
            rc = WriteOptionHandler(pReqPacket, pRspPacket);
            break;

        case CLIENT_OPTION_LIST_RECORDS:
            rc = ListOptionHandler(pReqPacket, pRspPacket);
            break;

        case CLIENT_OPTION_NOP:
            rc = NoOptionHandler(pReqPacket, pRspPacket);
            break;

        default:
            /* Created the response packet */
            pDataRspP = MallocWC(sizeof(*pDataRspP));
            pDataRspP->option = pDataReqP->option;

            /* Fill out response packet before returning */
            pRspPacket->pHeader->status = PI_ERROR;
            pRspPacket->pHeader->errorCode = PDATA_INVALID_OPTION;
            rc = PI_ERROR;
            break;
    }                           /* end of switch */
    return rc;
}


/*----------------------------------------------------------------------------
** Function:    CreateClientRecord
**
** Description:
**              Creates the file with record name.  if the file does not exist
**              it is created.  If the file does exist, then close the file
**              and return -1.
**
** Inputs:      fName   - Name of file(client record name) to use.
**
** Returns:     PI_GOOD
**              PDATA_DUPLICATE_RECORD
**              PDATA_FILESYSTEM_ERROR
**              PDATA_INVALID_RECORD
**
**--------------------------------------------------------------------------*/
INT32 CreateClientRecord(char *fname)
{
    INT32       retval = PI_GOOD;
    INT32       fd = -1;

    if (fname)
    {
        /*
         * Open the file, if the file does not exist this will error
         * and we will take the new file path.
         */
        if ((fd = open(fname, (O_RDWR), (S_IRUSR | S_IWUSR))) >= 0)
        {
            /* The file is already there. so return error */
            Close(fd);
            dprintf(DPRINTF_DEFAULT, "%s: The file is already there%s\n", __func__, fname);
            retval = PDATA_DUPLICATE_RECORD;
        }

        /* The file does not already exist. So create it and return success */
        else if ((fd = open(fname, (O_RDWR | O_CREAT | O_EXCL), (S_IRUSR | S_IWUSR))) >= 0)
        {
            Close(fd);
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "%s: Unable to create record on the disk %s\n", __func__, fname);
            retval = PDATA_FILESYSTEM_ERROR;
        }
    }
    else
    {
        /* File name is null */
        dprintf(DPRINTF_DEFAULT, "%s: File name parameter is NULL\n", __func__);
        retval = PDATA_INVALID_RECORD;
    }

    return retval;
}


/*----------------------------------------------------------------------------
** Function:    ReadClientRecord
**
** Description:
**              Read the file with record name.  if the file does not exist
**              error will be thrwon.  If the file does exist, then open
**              the file and read nbytes starting from start byte. If EOF
**              is before the start error will be thrown.
**
** Inputs:      fName   - Name of file(client record name) to use.
**              start   - Start byte offset from the begining of record/file.
**              nbytes  - Number of bytes to read from start offset.
**              buffer  - Place holder for storing the read bytes.
**              truncate  - truncate flag.
**
** Returns:     PI_GOOD
**              PDATA_FILESYSTEM_ERROR
**              PDATA_INVALID_RECORD
**              PDATA_EOF_REACHED
**
**--------------------------------------------------------------------------*/
static INT32 ReadClientRecord(char *fname, UINT32 start, UINT32 *nbytes, UINT8 *buffer,
                              INT32 truncate_lth)
{
    INT32       fd = -1;
    int         nread;
    int         retval = PI_GOOD;

    if (fname)
    {
        /*
         * Open the file, if the file does not exist this will error
         * and we will take the new file path.
         */
        if ((fd = open(fname, (O_RDWR), (S_IRUSR | S_IWUSR))) >= 0)
        {
            /* Set the fd to the start offset */
            if (lseek(fd, start, SEEK_SET) != (INT32)start)
            {
                Close(fd);
                retval = PDATA_FILESYSTEM_ERROR;
            }

            /* Read the nbytes into the buffer */
            if (((nread = read(fd, buffer, *nbytes)) <= 0) && (retval == PI_GOOD))
            {
                Close(fd);
                if (errno == EFAULT)
                {
                    /* buffer is outside accessible address */
                    retval = PDATA_CANNOT_ALLOCATE_BUFFER;
                }
                retval = PDATA_FILESYSTEM_ERROR;
            }
            if ((*nbytes != (UINT32)nread) && (retval == PI_GOOD))
            {
                *nbytes = nread;
                retval = PDATA_EOF_REACHED;
            }

            /* Truncate flag is enabled */
            if ((truncate_lth) && (retval == PI_GOOD))
            {
                dprintf(DPRINTF_DEFAULT, "TRUNCING ON READ\n");
                ftruncate(fd, start + *nbytes);
            }
            Close(fd);
        }
        else
        {
            /* The file is not there. so return error */
            retval = PDATA_FILESYSTEM_ERROR;
        }
    }
    else
    {
        /* File name is null */
        retval = PDATA_INVALID_RECORD;
    }

    return retval;
}


/*----------------------------------------------------------------------------
** Function:    WriteClientRecord
**
** Description:
**              Write the buffer to the file with record name.
**              If the file does not exist or can not write the nbytes
**              error will be returned. The buffer will be written to
**              file from the start offset to nbytes.
**
** Inputs:      fName   - Name of file(client record name) to use.
**              start   - Start byte offset from the begining of record/file.
**              nbytes  - Number of bytes to read from start offset.
**              buffer  - Place holder for storing the read bytes.
**              truncate  - truncate flag.
**
** Returns:     PI_GOOD
**              PDATA_FILESYSTEM_ERROR
**              PDATA_INVALID_RECORD
**
**--------------------------------------------------------------------------*/
INT32 WriteClientRecord(char *fname, UINT32 start, UINT32 *nbytes,
                        void *buffer, INT32 truncate_lth)
{
    INT32       fd = -1;
    INT32       written = 0;
    INT32       retval = PI_GOOD;

    if (fname)
    {
        /*
         * Open the file, if the file does not exist this will error
         * and we will take the new file path.
         */
        if ((fd = open(fname, (O_RDWR), (S_IRUSR | S_IWUSR))) >= 0)
        {
            /* Set the fd to the start offset */
            if (lseek(fd, start, SEEK_SET) >= 0)
            {
                /* Write the buffer to the specified record */
                if ((written = write(fd, buffer, *nbytes)) > 0)
                {
                    if (written != (INT32)*nbytes)
                    {
                        retval = PDATA_FILESYSTEM_ERROR;
                    }

                    /* Copy all the info to disk. */
                    if (0 != fsync(fd))
                    {
                        dprintf(DPRINTF_DEFAULT, "Fsync is not successful for file %s error is %s\n",
                                fname, strerror(errno));
                    }
                    if ((truncate_lth) && (retval == PI_GOOD))
                    {
                        ftruncate(fd, start + *nbytes);
                    }
                }
                else
                {
                    dprintf(DPRINTF_DEFAULT, "Write failed for file %s -%s\n", fname,
                            strerror(errno));
                    retval = PDATA_FILESYSTEM_ERROR;
                }
            }
            else
            {
                dprintf(DPRINTF_DEFAULT, "lseek failed %s\n", strerror(errno));
                retval = PDATA_FILESYSTEM_ERROR;
            }
            Close(fd);
        }
        else
        {
            /* File opening error */
            dprintf(DPRINTF_DEFAULT, "File opening error\n");
            retval = PDATA_FILESYSTEM_ERROR;
        }
    }
    else
    {
        /* File name is null */
        retval = PDATA_INVALID_RECORD;
    }

    return retval;
}


/*----------------------------------------------------------------------------
** Function:    TruncateClientRecord
**
** Description:
**              truncate the client record for the specified bytes from.
**              start byte
**              error will be returned.
**              file from the start offset to nbytes.
**
** Inputs:      fName   - Name of file(client record name) to use.
**              start   - Start byte offset from the begining of record/file.
**              nbytes  - Number of bytes to read from start offset.
**
** Returns:     PI_GOOD
**              PDATA_FILESYSTEM_ERROR
**              PDATA_INVALID_RECORD
**
**--------------------------------------------------------------------------*/
static INT32 TruncateClientRecord(char *fname, UINT32 start, UINT32 *nbytes)
{
    INT32       fd = -1;
    INT32       retval = PI_GOOD;

    if (fname)
    {
        /*
         * Open the file, if the file does not exist this will error
         * and we will take the new file path.
         */
        if ((fd = open(fname, (O_RDWR), (S_IRUSR | S_IWUSR))) >= 0)
        {
            /* Truncate flag specified */
            ftruncate(fd, start + *nbytes);
            Close(fd);
        }
        else
        {
            /* File opening error */
            dprintf(DPRINTF_DEFAULT, "File opening error\n");
            retval = PDATA_FILESYSTEM_ERROR;
        }
    }
    else
    {
        /* File name is null */
        retval = PDATA_INVALID_RECORD;
    }
    return retval;
}


/*----------------------------------------------------------------------------
** Function:    GetClientRecordSize
**
** Description:
**              Get the size of file.
**
** Inputs:      fName   - Name of file(client record name) to use.
**              fSize   - file size is returned in this (-1 on error)
**
** Returns:     PI_GOOD
**              PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 GetClientRecordSize(const char *fname, UINT32 *fsize)
{
    int         fd;
    INT32       oldval = 0;
    INT32       retval = PI_GOOD;
    INT32       endval = 0;

    ccb_assert(fsize != NULL, fsize);

    /*
     * Open the file, if the file does not exist this will error
     * and we will take the new file path.
     */
    if ((fd = open(fname, (O_RDWR), (S_IRUSR | S_IWUSR))) < 0)
    {
        dprintf(DPRINTF_DEFAULT, "%s: file unable to opened fd = %d %s\n", __func__, fd, fname);
        *fsize = -1;
        retval = PI_ERROR;
    }
    else
    {
        oldval = lseek(fd, 0, SEEK_CUR);
        endval = lseek(fd, 0, SEEK_END);
        if ((oldval == -1) || (endval == -1))
        {
            dprintf(DPRINTF_DEFAULT, "%s: lseek error\n", __func__);
            *fsize = -1;
            retval = PI_ERROR;
        }
        else if (lseek(fd, oldval, SEEK_SET) == -1)
        {
            dprintf(DPRINTF_DEFAULT, "%s: lseek setting file error\n", __func__);
            *fsize = -1;
            retval = PI_ERROR;
        }
        else
        {
            *fsize = endval;
        }
        Close(fd);
    }
    return retval;
}


/*----------------------------------------------------------------------------
** Function:    DeleteClientRecord
**
** Description:
**              Deletes the file with record name.
**              If the file does not exist, error will be returned.
**
** Inputs:      fName   - Name of file(client record name) to use.
**
** Returns:     PI_GOOD
**              PDATA_FILESYSTEM_ERROR
**              PDATA_INVALID_RECORD
**
**--------------------------------------------------------------------------*/
static INT32 DeleteClientRecord(char *fname)
{
    INT32       retval = PI_GOOD;

    if (fname)
    {
        /* Remove the file (record) from the file system */
        if (remove(fname) < 0)
        {
            dprintf(DPRINTF_DEFAULT, " %s: error in deleting file\n", __func__);
            retval = PDATA_FILESYSTEM_ERROR;
        }
    }
    else
    {
        retval = PDATA_INVALID_RECORD;
    }
    return retval;
}


/*----------------------------------------------------------------------------
** Name:     DeleteAllPersistentFiles
**
** Desc:    Remove all files and management file in the opt/xiotech/xiodata
**          directory.
**
** Inputs:  None
**
** Returns: None
**
** WARNING: verified for errors
**--------------------------------------------------------------------------*/
void DeleteAllPersistentFiles(void)
{
    DIR        *dirp;
    char        pathname[MAX_PATH_NAME_LEN];
    struct dirent *pdirent;

    /* Open the directory of persistent data */
    dirp = opendir(PDATA_DIR);

    /* Get each file in the directory and remove */
    while ((pdirent = readdir(dirp)))
    {
        /* . and .. files need not process */
        if (pdirent->d_name[0] == '.')
        {
            continue;
        }

        /* append the default path to the record name for reading from file */
        appendDefaultPath(pathname, pdirent->d_name);

        /* Delete the record */
        DeleteClientRecord(pathname);
    }
}


/**
******************************************************************************
**
**  @brief      Create the Mgt file and client data directory if those are not
**              existing
**
**              creates the management structure file.
**              If the file exists,the function just returns success
**              if not then the file is created.
**
**  @param      fname   - Name of file with path to create.
**
**  @return     PI_GOOD
**              PI_ERROR
**
**  @attention  modified on 10/30/06
**
******************************************************************************
**/
INT32 Client_InitMgtFile(const char *fname)
{
    int         fd;
    UINT32      timeStamp = 0;
    INT32       retval = PI_GOOD;

    if (fname)
    {
        /*
         * Open the file, if the file does not exist this will error
         * and we will take the new file path.
         */
        if ((fd = open(fname, (O_RDWR), (S_IRUSR | S_IWUSR))) > 0)
        {
            dprintf(DPRINTF_DEFAULT, "%s: file opened fd = %d\n", __func__, fd);
            Close(fd);
        }
        else if ((fd = open(fname, (O_RDWR | O_CREAT | O_EXCL), (S_IRUSR | S_IWUSR))) > 0)
        {
            /* If the file does not already exist, create it.  */
            dprintf(DPRINTF_DEFAULT, "%s: file created fd = %d\n", __func__, fd);
            timeStamp = 0;
            write(fd, &timeStamp, sizeof(UINT32));
            Close(fd);
        }
        else
        {
            /* File creation error */
            dprintf(DPRINTF_DEFAULT, "%s: Error in creating MgtStructure.mmf\n", __func__);
            retval = PI_ERROR;
        }
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "%s: Null pointer passed\n", __func__);
        retval = PI_ERROR;
    }

    return retval;
}


/**
******************************************************************************
**
**  @brief      Set/Rest timeout for lock on a record
**
**  @param      client - Client management node
**              timeoutSec - Timout in seconds
**
**  @return     none
**
******************************************************************************
**/
static void setRecordTimeout(node *client, UINT16 timeoutSec)
{
    client->client_timer.timeout = timeoutSec;
    client->client_timer.locktime = RTC_GetLongTimeStamp();
    return;
}


/**
******************************************************************************
**
**  @brief      Reset timeout for lock on a record
**
**  @param      pDataReqP - PI request packet
**
**  @return     none
**
******************************************************************************
**/
static void resetRecordTimeout(PI_CLIENT_DATA_CONTROL_REQ *pDataReqP)
{
    node       *pRecord;
    INT32       retval = PI_GOOD;

    /* Get the record node in MGT list */
    (void)LockMutex(&gMgtListMutex, MUTEX_WAIT);
    retval = FindMgtStructure((char *)pDataReqP->recordName, &pRecord);

    if (retval == PI_GOOD)
    {
        /* if record is locked then reset the timer */
        if (pRecord->record.recordLocked)
        {
            setRecordTimeout(pRecord, pRecord->client_timer.timeout);
        }
    }
    UnlockMutex(&gMgtListMutex);
}


/*----------------------------------------------------------------------------
** Name:    SendIpcPersistentData
**
** Desc:    Create and send the IPC_PERSISTENTDATA packet to
**          another controller.
**
** Inputs:  serialNum - Serial number of the controller.
**          ptrPacket - IPC packet to be sent to the slave.
**
** Returns: PI_GOOD if the event was successfully sent and executed on the other
**          controller. PDATA error code is returned in case of failure.
**
**          PDATA errors returned are
**
**          PDATA_TOO_MUCH_DATA
**          PDATA_OUT_OF_RANGE
**          PDATA_INVALID_OPTION
**          PDATA_RECORD_NOT_FOUND
**          PDATA_DUPLICATE_RECORD
**          PDATA_FILESYSTEM_ERROR
**          PDATA_CANNOT_ALLOCATE_BUFFER
**          PDATA_EOF_REACHED
**          PDATA_INVALID_RECORD
**          PDATA_LOCKED_ERROR
**          PDATA_SLAVE_DEAD
**
**--------------------------------------------------------------------------*/
static UINT32 SendIpcPersistentData(UINT32 serialNum, IPC_PACKET *ptrPacket)
{
    INT32       rc = PI_GOOD;
    IPC_PACKET  rx = { NULL, NULL };
    PATH_TYPE   pathType;
    XIO_PACKET  XIORsp = { NULL, NULL };
    UINT8       retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

    ccb_assert(serialNum > 0, serialNum);
    ccb_assert(ptrPacket != NULL, ptrPacket);

    dprintf(DPRINTF_DEFAULT, "%s: Processing PersistentData (dest: 0x%x, et: 0x%x, sn: 0x%x, ds: 0x%x).\n",
            __func__, serialNum, ptrPacket->data->persistent.eventType,
            ptrPacket->data->persistent.serialNum, ptrPacket->data->persistent.dataSize);

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacketBySN with rxPacket of %p\n", __FILE__, __LINE__, __func__, &rx);
#endif  /* HISTORY_KEEP */

    do
    {
        Free(rx.data);

        /* Sending packet to the other controller using any IPC path possible */
        pathType = IpcSendPacketBySN(serialNum, SENDPACKET_ANY_PATH,
                                     ptrPacket, &rx, NULL, NULL, NULL, PDC_IPC_SEND_TMO);
    } while (pathType == SENDPACKET_NO_PATH && (retries--) > 0);

    if (!IpcSuccessfulXfer(pathType))
    {
        dprintf(DPRINTF_DEFAULT, "%s - ERROR: Failed to send event to controller (0x%x).\n",
                __func__, serialNum);

        rc = PDATA_SLAVE_DEAD;
    }
    else
    {
        /* Get the xio packet from the response ipc packet */
        XIORsp.pHeader = (PI_PACKET_HEADER *)rx.data->persistent.packet;
        XIORsp.pPacket = rx.data->persistent.packet + sizeof(PI_PACKET_HEADER);

        /* The status of the xiopacket response is stored in return value */
        rc = XIORsp.pHeader->errorCode;
    }

    FreePacketStaticPacketPointer(&rx, __FILE__, __LINE__);

    return (rc);
}


/*----------------------------------------------------------------------------
** Name:    SendToSlavesPersistentData
**
** Desc:    Create and send the IPC_PERSISTENTDATA packet to
**          another controller.
**
** Inputs:  event - event type
**          pReqPacket - command to send to the other controller.
**
** Returns: PI_GOOD if the event was successfully sent or there are no
**          slaves
**          All PDATA errors will be carried from the sendIpcPersistentData
**          function.
**
** WARNING: The error code returned from the last slave only returned.
**          If we have more than one slave code should be modified.
**
**          modified on 11/1/06
**--------------------------------------------------------------------------*/
static INT32 SendToSlavesPersistentData(UINT32 event, XIO_PACKET *pXIOReq)
{
    UINT32      slaveSN = 0;
    INT32       rc = PI_GOOD;
    INT16       slaveIndex = 0;
    UINT32      ipcReqSize;
    IPC_PACKET *pIPCReq;

    ccb_assert(pXIOReq != NULL, pXIOReq);

    /*
     * Create the IPC request packet.  CreatePacket() creates and fills
     * in the header portion of the IPC packet and allocates memory for
     * the data portion.  The tunneled packet (header + request) goes in
     * the data portion of the IPC packet.
     */
    ipcReqSize = sizeof(IPC_CLIENT_PERSISTENT_DATA) + sizeof(PI_PACKET_HEADER) +
        pXIOReq->pHeader->length;

    pIPCReq = CreatePacket(PACKET_IPC_CLIENT_PERSISTENT_DATA_CMD, ipcReqSize, __FILE__, __LINE__);

    pIPCReq->data->persistent.eventType = event;
    pIPCReq->data->persistent.dataSize = pXIOReq->pHeader->length;

    /*
     * Copy the XIO request header and request packet from the XIO_PACKET
     * request struct into the IPC request packet.
     */
    memcpy(pIPCReq->data->persistent.packet, pXIOReq->pHeader, sizeof(PI_PACKET_HEADER));

    memcpy(pIPCReq->data->persistent.packet + sizeof(PI_PACKET_HEADER),
           pXIOReq->pPacket, pXIOReq->pHeader->length);

    /* Send persistent data control command to all slaves */
    while ((slaveSN = GetNextRemoteControllerSN(&slaveIndex)) && (slaveSN != 0))
    {
        pIPCReq->data->persistent.serialNum = slaveSN;

        rc = SendIpcPersistentData(slaveSN, pIPCReq);
    }

    FreePacket(&pIPCReq, __FILE__, __LINE__);

    return (rc);
}


/*----------------------------------------------------------------------------
** Name:    ClientPersistentDataPostProcess
**
** Desc:    Post Process of client persistent data command
**
**
** Inputs:  event - IPC_PACKET type (refer to ipc_packets.h file)
**          pReqPacket - command to requested.
**
** Returns: GOOD if the event was successfully sent and executed on the other
**          controller, ERROR otherwise.
**
**--------------------------------------------------------------------------*/
static INT32 ClientPersistentDataPostProcess(UINT32 event, XIO_PACKET *pReqPacket)
{
    INT32       rc = PI_GOOD;
    PI_CLIENT_DATA_CONTROL_REQ *pDataReqP = NULL;
    char        eventName[50];

    if (Qm_GetMasterControllerSN() != GetMyControllerSN())
    {
        /* Post process is not required for the slaves */
        return (PI_GOOD);
    }

    pDataReqP = (PI_CLIENT_DATA_CONTROL_REQ *)pReqPacket->pPacket;

    /* Remove the locks if there are any, before sending to slave controllers */
    pDataReqP->flags &= CLIENT_FLAG_TRUNC;

    switch (event)
    {
        case IPC_PERSISTENT_CREATE:
            /*
             * send persistent data create event and also
             * send the create command to all slaves
             */
            rc = SendToSlavesPersistentData(IPC_PERSISTENT_CREATE, pReqPacket);
            if ((rc == PI_GOOD) || (rc == PDATA_SLAVE_DEAD))
            {
                SendAsyncEvent(LOG_PDATA_CREATE, 0, NULL);
            }
            sprintf(eventName, "IPC_PERSISTENT_CREATE");
            break;

        case IPC_PERSISTENT_READ:
            /* Send persistent data read event with trunc option */
            rc = SendToSlavesPersistentData(IPC_PERSISTENT_READ, pReqPacket);
            if ((rc == PI_GOOD) || (rc == PDATA_SLAVE_DEAD))
            {
                SendAsyncEvent(LOG_PDATA_WRITE, 0, NULL);
            }
            sprintf(eventName, "IPC_PERSISTENT_READ");
            break;

        case IPC_PERSISTENT_REMOVE:
            /*
             * send persistent data remove event and also
             * send the remove command to all slaves
             */
            rc = SendToSlavesPersistentData(IPC_PERSISTENT_REMOVE, pReqPacket);
            if ((rc == PI_GOOD) || (rc == PDATA_SLAVE_DEAD))
            {
                SendAsyncEvent(LOG_PDATA_REMOVE, 0, NULL);
            }
            sprintf(eventName, "IPC_PERSISTENT_REMOVE");
            break;

        case IPC_PERSISTENT_WRITE:
            /*
             * send persistent data write event and also
             * send the write command to all slaves
             */
            rc = SendToSlavesPersistentData(IPC_PERSISTENT_WRITE, pReqPacket);
            if ((rc == PI_GOOD) || (rc == PDATA_SLAVE_DEAD))
            {
                SendAsyncEvent(LOG_PDATA_WRITE, 0, NULL);
            }
            sprintf(eventName, "IPC_PERSISTENT_WRITE");
            break;

        case IPC_PERSISTENT_NOP:
            /* Send NOP with truncate option to all slaves */
            rc = SendToSlavesPersistentData(IPC_PERSISTENT_NOP, pReqPacket);
            if ((rc == PI_GOOD) || (rc == PDATA_SLAVE_DEAD))
            {
                SendAsyncEvent(LOG_PDATA_WRITE, 0, NULL);
            }
            sprintf(eventName, "IPC_PERSISTENT_NOP");
            break;

        default:
            rc = ERROR;
            break;
    }

    /*
     * If any of the return value is pdata operation
     * failure on slave then print a log message
     */
    if ((rc == PDATA_TOO_MUCH_DATA) ||
        (rc == PDATA_OUT_OF_RANGE) ||
        (rc == PDATA_INVALID_OPTION) ||
        (rc == PDATA_RECORD_NOT_FOUND) ||
        (rc == PDATA_DUPLICATE_RECORD) ||
        (rc == PDATA_FILESYSTEM_ERROR) ||
        (rc == PDATA_CANNOT_ALLOCATE_BUFFER) ||
        (rc == PDATA_EOF_REACHED) ||
        (rc == PDATA_INVALID_RECORD) || (rc == PDATA_LOCKED_ERROR))
    {
        LogMessage(LOG_TYPE_DEBUG, "Replication of %s operation for record %s is NOT done due to error 0x%x on Slave\n",
                   eventName, pDataReqP->recordName, rc);
    }
    return (rc);
}


/*----------------------------------------------------------------------------
** Name:    TransferClientDataToSlave
**
** Desc:    Transfer client persistent data to requested controller
**
** Inputs:  serNum - serial number of the controller requested
**
** Returns: PI_GOOD
**
**--------------------------------------------------------------------------*/
INT32 TransferClientDataToSlave(UINT32 serNum)
{
    node       *nodedata;
    char        recordName[CLIENT_RECORD_NAME_MAX_LEN];

    /* record name. 0x00 terminated string  can only
     * contain digits, alphabet and underscore */
    UINT32      recordLength;   /* record size in bytes */
    char        pathname[MAX_PATH_NAME_LEN];

    /*
     * Lock the mutex on the management list to prevent others from using
     * the iterator while it is in use here.
     */
    (void)LockMutex(&gMgtListMutex, MUTEX_WAIT);

    /* Traverse the entire list and transfer the records */
    SetIterator(mgtList);
    while ((nodedata = Iterate(mgtList)) != NULL)
    {
        /* Get the record name and length */
        strcpy(recordName, (char *)nodedata->record.recordName);

        /* Append the default path to the record name for reading from file */
        appendDefaultPath(pathname, recordName);

        if (PI_ERROR != GetClientRecordSize(pathname, &recordLength))
        {
            /* Copy the record to slave */
            CopyMasterClientRecordToSlave(serNum, recordName, recordLength);
        }
    }

    /* Done using the iterator on the management list so unlock the mutex */
    UnlockMutex(&gMgtListMutex);

    return PI_GOOD;
}


/*----------------------------------------------------------------------------
** Name:     CreateResyncRecordPacket
**
** Desc:    Create resync ipc packet and send to requested slave
**
** Inputs:  serNum - serial number of the slave requested
**          recordName - name of the record set for copy to slave
**          offset - offset from starting of the record to be copied
**          pktSize - size of the record data to be transfered
**
** Returns: PI_GOOD on success, otherwise the error returned by
**          SendIpcPersistentData or read is forwarded.
**
**--------------------------------------------------------------------------*/
static INT32 CreateResyncRecordPacket(UINT32 serNum, char *recordName, UINT32 offset,
                                      UINT32 pktSize)
{
    UINT32      ipcReqSize;
    IPC_PACKET *pIPCReq;
    char        pathname[MAX_PATH_NAME_LEN];
    UINT8      *rbuffer = NULL;
    INT32       error = 0;
    INT32       retval = PI_GOOD;

    /*
     * Create the IPC request packet.  CreatePacket() creates and fills
     * in the header portion of the IPC packet and allocates memory for
     * the data portion.  The clientResync packet (header + request) goes in
     * the data portion of the IPC packet.
     */
    ipcReqSize = sizeof(IPC_RESYNC_CLIENT_RECORD) + sizeof(PI_PACKET_HEADER) + pktSize;

    /* Create ipc resync client record packet */
    pIPCReq = CreatePacket(PACKET_IPC_RESYNC_CLIENT_RECORD, ipcReqSize, __FILE__, __LINE__);

    /* copy the record name, offset and datasize to packet */
    strcpy(pIPCReq->data->resyncClientRecord.recordName, recordName);

    pIPCReq->data->resyncClientRecord.startOffset = offset;

    pIPCReq->data->resyncClientRecord.dataSize = pktSize;

    /* Allocate the memory for the response. */
    rbuffer = MallocWC(pktSize);

    /* Append the default path to the record name for reading from file */
    appendDefaultPath(pathname, recordName);

    /* Read Client record from the file */
    error = ReadClientRecord(pathname, offset, &pktSize, rbuffer, 0);

    if (error == PI_GOOD)
    {
        /*
         * Copy the XIO request header and request packet from the XIO_PACKET
         * request struct into the IPC request packet.
         */
        memcpy(pIPCReq->data->resyncClientRecord.data, rbuffer, pktSize);

        /* Send data of record to the slave */
        retval = SendIpcPersistentData(serNum, pIPCReq);
    }
    else
    {
        retval = error;
    }
    Free(rbuffer);
    FreePacket(&pIPCReq, __FILE__, __LINE__);

    return retval;
}


/*----------------------------------------------------------------------------
** Name:     CopyMasterClientRecordToSlave
**
** Desc:    divide the record in 64K chunks and send to slave
**
** Inputs:  serNum - serial number of the slave requested
**          recordName - name of the record set for copy to slave
**          recordLength - size of the record data
**
** Returns: GOOD if the event was successfully sent and executed on the other
**          controller, ERROR otherwise.
**
**--------------------------------------------------------------------------*/
static INT32 CopyMasterClientRecordToSlave(UINT32 serNum, char *recordName,
                                           UINT32 recordLength)
{
    UINT32      nChunks = 0;
    UINT32      startOffset = 0;
    UINT32      remSize = 0;
    UINT32      i;
    INT32       rc = PI_GOOD;

    /*
     * If the record Length is more than 64K then split
     * the data into <= 64K size data chunks.
     */
    if (recordLength > SIZE_64K)
    {
        nChunks = recordLength / SIZE_64K;

    }

    for (i = 0; ((i < nChunks) && (rc == PI_GOOD)); i++)
    {
        /* Update the startOffset */
        startOffset = i * SIZE_64K;
        rc = CreateResyncRecordPacket(serNum, recordName, startOffset, SIZE_64K);
    }
    if (PI_GOOD == rc)
    {
        /* Calculate startOffset */
        startOffset = nChunks * SIZE_64K;

        remSize = (recordLength - (nChunks * SIZE_64K));

        /* Call the resync record for the last remaining data */
        if (remSize > 0)
        {
            rc = CreateResyncRecordPacket(serNum, recordName, startOffset, remSize);
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "%s: remSize = 0 occured\n", __func__);
        }
    }
    return rc;
}


/*----------------------------------------------------------------------------
** Name:    MgtFileTimeStamp
**
** Desc:    Get the latest modified time stamp of the file MgtStructure.mmf
**
** Inputs:  None
**
** Returns: timestamp value in seconds.
**          0 on error.
**
**--------------------------------------------------------------------------*/
static INT32 MgtFileTimeStamp(void)
{
    int         fd;
    UINT32      timeStamp = 0;

    if ((fd = open(PDATA_MGT_FNAME, (O_RDWR), (S_IRUSR | S_IWUSR))) >= 0)
    {
        read(fd, &timeStamp, sizeof(UINT32));
        Close(fd);
    }
    return (timeStamp);
}


/**
******************************************************************************
**
**  @brief      IpcSendLatestPersistentData
**
**              Sends the IPC_CLIENT_PERSISTENT packet to the controller specified
**              by serialNum.
**
**  @param      serialNum - serial number of the controller.
**  @param      desiredPath - path to be sent.
**
**  @return     None
******************************************************************************
**/
static void IpcSendLatestPersistentData(UINT32 serialNum, UNUSED PATH_TYPE desiredPath)
{
    IPC_PACKET  rx = { NULL, NULL };
    IPC_PACKET *pIPCReq;
    UINT32      ipcReqSize;
    PATH_TYPE   pathType;
    UINT8       retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

    /*
     * Create the IPC request packet.  CreatePacket() creates and fills
     * in the header portion of the IPC packet and allocates memory for
     * the data portion.
     */
    ipcReqSize = sizeof(IPC_LATEST_PERSISTENT_DATA) + sizeof(PI_PACKET_HEADER);

    pIPCReq = CreatePacket(PACKET_IPC_LATEST_PERSISTENT_DATA, ipcReqSize, __FILE__, __LINE__);

    pIPCReq->data->latestClientData.timeStamp = latestClientDataTimeStamp;
    pIPCReq->data->latestClientData.controllerSN = latestClientDataControllerSN;

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacketBySN with rxPacket of %p\n", __FILE__, __LINE__, __func__, &rx);
#endif  /* HISTORY_KEEP */

    do
    {
        Free(rx.data);

        /* Send latest persistent data command to other controller */
        pathType = IpcSendPacketBySN(serialNum, SENDPACKET_ANY_PATH,
                                     pIPCReq, &rx, NULL, NULL, NULL, PDC_IPC_SEND_TMO);
    } while (pathType == SENDPACKET_NO_PATH && (retries--) > 0);

    FreePacket(&pIPCReq, __FILE__, __LINE__);
    FreePacketStaticPacketPointer(&rx, __FILE__, __LINE__);
}


/*----------------------------------------------------------------------------
** Name:    SyncClientData
**
** Desc:    Updates the timestamp if it is less than the earlier one
**          and sends a Latest persistent data command to other controller
**
** Inputs:  None
**
** Returns: None
**
**--------------------------------------------------------------------------*/
void SyncClientData(void)
{
    UINT32      mgtTime;
    UINT32      count;
    UINT32      configIndex;
    UINT32      controllerSN;

    latestClientDataControllerSN = GetMyControllerSN();

    /* Get the management file time stamp, which tells the latest updated record */
    mgtTime = MgtFileTimeStamp();

    /* update the latestClient data variables */
    if (latestClientDataTimeStamp < mgtTime)
    {
        latestClientDataTimeStamp = mgtTime;
    }

    /* Broadcast the latest persistent data to all controllers */
    for (count = 0; count < Qm_GetNumControllersAllowed(); count++)
    {
        configIndex = Qm_GetActiveCntlMap(count);

        if (configIndex != ACM_NODE_UNDEFINED)
        {
            controllerSN = cntlConfigMap.cntlConfigInfo[configIndex].controllerSN;

            /*
             * If the serial number is zero or it is myself, skip this entry.
             */
            if (controllerSN == 0 || controllerSN == GetMyControllerSN())
            {
                continue;
            }
            dprintf(DPRINTF_DEFAULT, "%s: the controller SN %d\n", __func__, controllerSN);
            IpcSendLatestPersistentData(controllerSN, SENDPACKET_ANY_PATH);
        }
    }
}


/*----------------------------------------------------------------------------
** Name:    GetLatestClientData
**
** Desc:    Updates the timestamp if it is less than the earlier one
**          and sends a Latest persistent data command to other controller
**
** Inputs:  pClientCmd -- IPC packet
**
** Returns: PI_GOOD
**
**--------------------------------------------------------------------------*/
INT32 GetLatestClientData(IPC_LATEST_PERSISTENT_DATA *pClientCmd)
{
    UINT32      mgtTime;

    /* Get the management file time stamp, which tells the latest updated record */
    mgtTime = MgtFileTimeStamp();
    dprintf(DPRINTF_DEFAULT, "%s: mgtTime = %d\n", __func__, mgtTime);
    dprintf(DPRINTF_DEFAULT, "%s: latestClientTimeStamp= %d\n", __func__, latestClientDataTimeStamp);

    /* Update the latestClient data variables */
    if (latestClientDataTimeStamp < mgtTime)
    {
        latestClientDataTimeStamp = mgtTime;
    }

    if (latestClientDataTimeStamp == pClientCmd->timeStamp)
    {
        /* This controller has the same data of other controller */
        return (PI_GOOD);
    }

    /* Check if the time stamp is latest than we have */
    if (latestClientDataTimeStamp < pClientCmd->timeStamp)
    {
        latestClientDataTimeStamp = pClientCmd->timeStamp;
        latestClientDataControllerSN = pClientCmd->controllerSN;

        /* Delete all existing persistent data files */
        DeleteAllClientRecords();

        /* Initiate the resync data command to the latest controller */
        dprintf(DPRINTF_DEFAULT, "%s: Deleted all client records and send update to %d controller\n",
                __func__, latestClientDataControllerSN);
        IpcSendResyncPersistentData(latestClientDataControllerSN);
    }
    else
    {
        /* Send a latest persistent data command, since we have latest data */
        latestClientDataControllerSN = GetMyControllerSN();
        dprintf(DPRINTF_DEFAULT, "%s: This %d controller has latest\n",
                __func__, latestClientDataControllerSN);
        IpcSendLatestPersistentData(pClientCmd->controllerSN, SENDPACKET_ANY_PATH);
    }

    return (PI_GOOD);
}


/*----------------------------------------------------------------------------
** Name:    DeleteAllClientRecords
**
** Desc:    Deletes all persistent records on the controller
**
** Inputs:  None
**
** Returns: GOOD if the event was successfully sent and executed on the other
**          controller, ERROR otherwise.
**
**--------------------------------------------------------------------------*/
static INT32 DeleteAllClientRecords(void)
{
    node       *nodedata;
    char        pathname[MAX_PATH_NAME_LEN];
    INT32       error;

    (void)LockMutex(&gMgtListMutex, MUTEX_WAIT);

    /* Remove all the records */
    while ((nodedata = Pop(mgtList)) != NULL)
    {
        /* Append the default path and delete the record */
        appendDefaultPath(pathname, (char *)nodedata->record.recordName);
        error = DeleteClientRecord(pathname);
        if (error == PI_GOOD)
        {
            Free(nodedata);
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "%s: unable to delete record = %s\n",
                    __func__, nodedata->record.recordName);
        }
    }

    /* Update the mgt file */
    error = StoreMgtStructureList(NORMAL_PHASE);

    UnlockMutex(&gMgtListMutex);

    return (error);
}


/*----------------------------------------------------------------------------
** Name:    InitClientPersistent
**
** Desc:    Initialization for Client Persistent Data
**
** Inputs:  None
**
** Returns: None
**
**--------------------------------------------------------------------------*/
void InitClientPersistent(void)
{
    /* Initialize EwokPersistant memory */
    dprintf(DPRINTF_DEFAULT, "Initializing EwokPersistent memory...\n");
    if (Client_InitMgtFile(PDATA_MGT_FNAME))
    {
        dprintf(DPRINTF_DEFAULT, "ERROR Initializing Ewokdata\n");
    }

    CreateMgtStructureList();
}


/*----------------------------------------------------------------------------
** Name:    ValidateClientParameters
**
** Desc:    validates all the parameters like lock request and record name
**          reports the error condition in return value.
**
** Inputs:  pReqPacket - request packet pointer
**          pRecord - if a node is found, its address is returned here
**
** Returns: PI_GOOD if validations are ok
**          returns PDATA errors or PI_ERROR
**
** WARNING: Do not call this function when record name is NULL. For eg. List
**          records.
**
** ATTENTION:   gMgtListMutex must be locked before calling.
**
**--------------------------------------------------------------------------*/
static INT32 ValidateClientParameters(XIO_PACKET *pReqPacket, node **pRecord)
{
    PI_CLIENT_DATA_CONTROL_REQ *pDataReqP = NULL;
    INT32       retval = PI_GOOD;
    UINT16      lockoption;

    pDataReqP = (PI_CLIENT_DATA_CONTROL_REQ *)pReqPacket->pPacket;

    /* Get the lock option requested */
    lockoption = (pDataReqP->flags & CLIENT_FLAG_LOCK);

    /* Validate the record name */
    retval = ValidateRecordName((char *)pDataReqP->recordName);
    if (PI_GOOD == retval)
    {
        /* Get the record node in MGT list -- see ATTENTION above. */
        retval = FindMgtStructure((char *)pDataReqP->recordName, pRecord);

        if (PI_GOOD == retval)
        {
            /*
             * If lock is requested for, try to apply lock. Even otherwise
             * verify if the record is not locked by someone else.
             */
            if (lockoption)
            {
                retval = ClientApplyLock(pReqPacket, *pRecord);
            }
            else
            {
                retval = IsRecordLocked(pReqPacket, *pRecord);
            }
        }
    }
    return (retval);
}


/*----------------------------------------------------------------------------
** Function:    IsRecordLocked
**
** Description: Verify if a record is already locked and if an operation on
**              the record is permitted.
**
** Inputs:      pReqPacket -- pointer to request packet
**              pRecord - pointer to record in mgt table.
**
** Returns:     PDATA_LOCK_REJECTED - rejecting the command
**              PI_GOOD
**
**--------------------------------------------------------------------------*/
static INT32 IsRecordLocked(XIO_PACKET *pReqPacket, node *pRecord)
{
    INT32       reqlockfd;
    INT32       presentlockfd;
    INT32       retval = PI_GOOD;
    PI_CLIENT_DATA_CONTROL_REQ *pDataReqP = NULL;

    pDataReqP = (PI_CLIENT_DATA_CONTROL_REQ *)pReqPacket->pPacket;
    reqlockfd = pReqPacket->pHeader->socket;
    presentlockfd = pRecord->client_timer.lockfd;
    retval = ValidateLock(reqlockfd, pRecord);

    return (retval);
}


/*----------------------------------------------------------------------------
** Function:    ClientApplyLock
**
** Description: Put the lock for the record if there was no lock. If lock is
**              existing and different from the previous, then error is returned.
**
** Inputs:      pReqPacket -- pointer to request packet
**              pRecord - pointer to record in mgt table.
**
** Returns:     PI_GOOD if success,
**              PDATA_LOCK_REJECTED otherwise
**
** ATTENTION:   gMgtListMutex must be locked before calling.
**
**--------------------------------------------------------------------------*/
static INT32 ClientApplyLock(XIO_PACKET *pReqPacket, node *pRecord)
{
    INT32       reqlockfd;
    INT32       retval = PI_GOOD;
    UINT16      timeOutSec = 0;
    PI_CLIENT_DATA_CONTROL_REQ *pDataReqP = NULL;

    pDataReqP = (PI_CLIENT_DATA_CONTROL_REQ *)pReqPacket->pPacket;

    timeOutSec = pDataReqP->timeOutSec;

    reqlockfd = pReqPacket->pHeader->socket;

    retval = ValidateLock(reqlockfd, pRecord);

    if (retval == PI_GOOD)
    {
        /* The lockflag is zero. so store the lockfd into the record */
        pRecord->record.recordLocked = LOCK_SET;

        /* Set the lockfd to the record info */
        pRecord->client_timer.lockfd = reqlockfd;

        /* Set the record timeout after apply the lock */
        if (!timeOutSec)
        {
            timeOutSec = DEFAULT_LOCK_TIMEOUT;
        }
        setRecordTimeout(pRecord, timeOutSec);

        /* Update the mgt table to file */
        retval = StoreMgtStructureList(NORMAL_PHASE);
    }
    return retval;
}


/**
******************************************************************************
**
**  @brief      Create Duplicate copy of a record
**
**  The copy will be used during rollback of any operation.
**
**  @param  srcfname - source path name
**  @param  dstfname - destination path name
**
**  @return FCOPY_SUCCESS if success, FCOPY_FAILURE if error
**
******************************************************************************
**/
static INT32 fcopy(char *srcfname, char *dstfname)
{
    int     fd1, fd2;

    fd1 = open(srcfname, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd1 < 0)
    {
        return FCOPY_FAILURE;       /* Source file open error */
    }

    fd2 = open(dstfname, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd2 >= 0)
    {
        Close(fd1);
        Close(fd2);
        return FCOPY_FAILURE;       /* Destination file already exists */
    }

    fd2 = open(dstfname, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd2 < 0)
    {
        Close(fd1);
        return FCOPY_FAILURE;       /* Cannot create destination file */
    }

    /* Do the file copy */
    for (;;)
    {
        ssize_t nbytes;
        UINT8   rbuf[BLK_SIZ];

        nbytes = read(fd1, rbuf, sizeof(rbuf));
        if (nbytes <= 0)
        {
            break;
        }
        write(fd2, rbuf, nbytes);
        if ((unsigned)nbytes < sizeof(rbuf))
        {
            break;
        }
    }
    fdatasync(fd2);                 /* Be sure data gets to storage */
    Close(fd2);
    Close(fd1);

    return FCOPY_SUCCESS;
}


/*----------------------------------------------------------------------------
** Name:    BackupRecord
**
** Desc:    Create Duplicate copy of a record. This copy will be used during
**          rollback of any operation.
**
** Inputs:  recordHeader - pointer to record management structure
**          backupnode - pointer to backup node
**
** Returns  PI_GOOD on success
**          PDATA_CANNOT_BACKUP on failure
**
**--------------------------------------------------------------------------*/
static INT32 BackupRecord(node *recordHeader, node *backupnode)
{
    INT32       error;
    INT8        ec;
    char        srcpathname[MAX_PATH_NAME_LEN];
    char        dstpathname[MAX_PATH_NAME_LEN];

    /* Copy the mgt structure */
    memcpy(backupnode, recordHeader, sizeof(*backupnode));

    /* Copy the record */
    appendDefaultPath(srcpathname, (char *)backupnode->record.recordName);
    strcpy(dstpathname, srcpathname);
    strcat(dstpathname, ".bak");
    ec = fcopy(srcpathname, dstpathname);
    if (ec == FCOPY_FAILURE)
    {
        error = PDATA_CANNOT_BACKUP;
    }
    else
    {
        error = PI_GOOD;
    }
    return (error);
}


/*----------------------------------------------------------------------------
** Name:    RollbackRecord
**
** Desc:    Rollback the operation on the given record, overwrite current copy
**          with backed up copy.
**
** Inputs:  backupnode - record management structure
**
** Returns: PI_GOOD if success, PI_ERROR otherwise
**
**--------------------------------------------------------------------------*/
static INT32 RollbackRecord(node backupnode)
{
    node       *recnode;
    char        srcpathname[MAX_PATH_NAME_LEN];
    char        dstpathname[MAX_PATH_NAME_LEN];
    INT32       status;

    (void)LockMutex(&gMgtListMutex, MUTEX_WAIT);
    FindMgtStructure((char *)backupnode.record.recordName, &recnode);

    if (recnode == NULL)
    {
        /* Record removed, create */
        recnode = MallocWC(sizeof(*recnode));
        memcpy(recnode, &backupnode, sizeof(*recnode) - 4);

        AddElement(mgtList, recnode);

        /* Update the mgt file */
        status = StoreMgtStructureList(NORMAL_PHASE);

        UnlockMutex(&gMgtListMutex);

        if (status != PI_GOOD)
        {
            return status;
        }

        /* Write back the record */
        appendDefaultPath(srcpathname, (char *)backupnode.record.recordName);
        strcpy(dstpathname, srcpathname);
        strcat(dstpathname, ".bak");
        rename(dstpathname, srcpathname);
    }
    else
    {
        /* Write back the mgt structure and Update the mgt file */
        memcpy(recnode, &backupnode, sizeof(*recnode) - 4);
        status = StoreMgtStructureList(NORMAL_PHASE);

        UnlockMutex(&gMgtListMutex);

        if (status != PI_GOOD)
        {
            return status;
        }

        /* Write back the record */
        appendDefaultPath(srcpathname, (char *)backupnode.record.recordName);
        strcpy(dstpathname, srcpathname);
        strcat(dstpathname, ".bak");
        unlink(srcpathname);
        rename(dstpathname, srcpathname);
    }
    return PI_GOOD;
}


/*----------------------------------------------------------------------------
** Name:    CleanupRecord
**
** Desc:    Cleanup the backup copy.
**
** Inputs:  backupnode - record management structure
**
** Returns: PI_GOOD if success, PI_ERROR otherwise
**
**--------------------------------------------------------------------------*/
static INT32 CleanupRecord(node backupnode)
{
    char        pathname[MAX_PATH_NAME_LEN];

    appendDefaultPath(pathname, (char *)backupnode.record.recordName);
    strcat(pathname, ".bak");
    unlink(pathname);

    return (PI_GOOD);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
