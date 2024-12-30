/* $Id: Snapshot.c 157452 2011-08-03 13:00:14Z m4 $ */
/*============================================================================
** FILE NAME:       Snapshot.c
** MODULE TITLE:    Simple File System Functions
**
** DESCRIPTION:     Utility functions for reading and writing configuration
**                  snapshots.
**
** Copyright (c) 2002 - 2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/

#include "CacheManager.h"
#include "CacheMisc.h"
#include "CmdLayers.h"
#include "crc32.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "EL.h"
#include "errorCodes.h"
#include "FIO.h"
#include "FW_Header.h"
#include "ipc_sendpacket.h"
#include "LargeArrays.h"
#include "logdef.h"

#include "MR_Defs.h"
#include "PacketInterface.h"
#include "PI_Utils.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "RMCmdHdl.h"
#include "rm_val.h"
#include "rtc.h"
#include "Snapshot.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"

/*****************************************************************************
** Private defines
*****************************************************************************/

typedef struct
{
    char        eyecatcher[8];  /* These first 4 items must not get     */
    UINT32      masterConfigFID;        /* modified or erased once initialized. */
    UINT32      controllerMapFID;
    UINT32      beNVRAMFID;
    /* Be careful not to modify the above fields */

    UINT32      snapshotNumber; /* The structure is cleared from here   */
    /* on down when a new entry is written. */

    TIMESTAMP   currentTime;    /* The time snapshot was taken  */

    FW_DATA     ccbRT;          /* Firmware versions when taken */
    FW_DATA     ccbBoot;
    FW_DATA     feRT;
    FW_DATA     feBoot;
    FW_DATA     feDiag;
    FW_DATA     beRT;
    FW_DATA     beBoot;
    FW_DATA     beDiag;

    UINT32      snapshotType;   /* Powerup, ConfigChg, PowerDown, Manual */
    UINT32      status;         /* open, inuse, keep, deleted etc */
    UINT32      flags;          /* which fids saved, etc */

    UINT32      reserved[10];   /* For later use */

    char        description[SNAPSHOT_DESCRIPTION_LEN];  /* User description.     */
                                                        /* \0 terminated string. */
    UINT32      CRC;
} SNAPSHOT_ENTRY;

typedef struct
{
    UINT8       eyecatcher[8];
    UINT32      initialized;
    UINT32      numberOfSnapshots;
    UINT32      magicNumber;
    UINT32      schema;
    UINT32      nextSnapshotNumber;
    UINT8       useCRCFlag;     /* 0 up thru 4.0, 1 after that */
    UINT8       reserved[19];   /* For later use */
    SNAPSHOT_ENTRY entry[FS_NUM_SNAPSHOT_FIDS]; /* Allocate max number */
} SNAPSHOT_DIR;

/*****************************************************************************
** Private variables
*****************************************************************************/
static SNAPSHOT_DIR snapshotDir LOCATE_IN_SHMEM;

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
MUTEX       configJournalMutex;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static INT32 FidToFidCopy(UINT32 destinationFID, UINT32 sourceFID);
static INT32 RestoreBENVRAM(UINT32 sourceFID);
static SNAPSHOT_ENTRY *FindNextEmptySnapshot(void);
static INT32 RefreshSnapshotDirectory(void);
static INT32 RecoverSnapshotDirectory(void);
static INT32 WriteSnapshotDirectory(void);
static INT32 ResetAllControllers(void);
static void SortSnapshotEntries(UINT32 *sorted);
static INT32 SeqNum2Index(UINT32 seq);

/*****************************************************************************
** Code Start - Public functions
*****************************************************************************/

void TakeSnapshotTask(TASK_PARMS *parms)
{
    UINT32      type = parms->p1;
    char       *description = (char *)parms->p2;

    TakeSnapshot(type, description);
}

/*----------------------------------------------------------------------------
** Function:    TakeSnapshot()
**
** Description: Takes a system configuration snapshot
**
** Inputs:      type - type of snapshot
**              description - description string to save with the snapshot.
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
INT32 TakeSnapshot(UINT32 type, const char *description)
{
    INT32       rc = GOOD;
    INT32       rc1 = GOOD;
    INT32       rc2 = GOOD;
    INT32       rc3 = GOOD;
    SNAPSHOT_ENTRY *ssEntry = NULL;
    LOG_SNAPSHOT_TAKEN_PKT *logMsg = NULL;

    /*
     * Don't take a snapshot if not the master,
     * simply return GOOD status.
     */
    if (!(TestforMaster(GetMyControllerSN())))
    {
        return GOOD;
    }

    /*
     * lock the journaling mutex
     */
    (void)LockMutex(&configJournalMutex, MUTEX_WAIT);

    do
    {
        if ((rc = RefreshSnapshotDirectory()) != GOOD)
        {
            break;
        }

        /*
         * Get an open slot in the directory
         */
        ssEntry = FindNextEmptySnapshot();
        if (ssEntry == NULL)
        {
            rc = SNAPSHOT_ERROR_NO_EMPTY_SLOTS;
            break;
        }

        /*
         * Fill in what we know
         */
        memset(&ssEntry->snapshotNumber, 0,
               sizeof(*ssEntry) - offsetof(SNAPSHOT_ENTRY, snapshotNumber));
        ssEntry->status = SNAPSHOT_STATUS_INUSE;
        ssEntry->snapshotNumber = snapshotDir.nextSnapshotNumber++;
        RTC_GetTimeStamp(&ssEntry->currentTime);
        ssEntry->snapshotType = type;
        if (description)
        {
            strncpy(ssEntry->description, description, sizeof(ssEntry->description) - 1);
        }
        else
        {
            strcpy(ssEntry->description, "No description specified");
        }

        /*
         * Get the firmware version information.
         */
        GetFirmwareData(&ssEntry->ccbRT);

        /*
         * TODO - do an intermediate write of the directory with an IN_PROGRESS
         * flag set so that if we tank on the next steps, we know the entry is
         * hozed.
         */

        /*
         * Copy the 3 working fids into the snapshot fids
         */
        rc1 = FidToFidCopy(ssEntry->masterConfigFID, FS_FID_QM_MASTER_CONFIG);
        if (rc1 == GOOD)
        {
            ssEntry->flags |= SNAPSHOT_FLAG_MASTER_CONFIG;
        }

        rc2 = FidToFidCopy(ssEntry->controllerMapFID, FS_FID_QM_CONTROLLER_MAP);
        if (rc2 == GOOD)
        {
            ssEntry->flags |= SNAPSHOT_FLAG_CTRL_MAP;
        }

        rc3 = FidToFidCopy(ssEntry->beNVRAMFID, FS_FID_BE_NVRAM);
        if (rc3 == GOOD)
        {
            ssEntry->flags |= SNAPSHOT_FLAG_BE_NVRAM;
        }

        if (rc1 != GOOD || rc2 != GOOD || rc3 != GOOD)
        {
            ssEntry->status = SNAPSHOT_STATUS_ERROR;
            rc = SNAPSHOT_ERROR_RESTORING_FIDS;
            break;
        }

        /*
         * Calculate a CRC over this entry
         */
        ssEntry->CRC = CRC32(ssEntry, sizeof(*ssEntry) - 4);
    } while (FALSE);

    /*
     * Save out the updated snapshot directory.  Even on error,
     * save out the new entry as it may contain some stuff that
     * might be useful later.
     */
    if (ssEntry)
    {
        rc = WriteSnapshotDirectory();
    }

    /*
     * Unlock the journaling mutex
     */
    UnlockMutex(&configJournalMutex);

    /*
     * Only log if the file system was able to be written
     */
    if (FileSystemInitialized() && ssEntry)
    {
        /*
         * Log a message
         */
        logMsg = MallocWC(sizeof(LOG_SNAPSHOT_TAKEN_PKT) + SNAPSHOT_DESCRIPTION_LEN);


        dprintf(DPRINTF_DEFAULT, "TakeSnapshot: snapshot %s written to entry: %d, rc: %d\n",
                rc ? "(with errors)" : "successfully", (ssEntry - &snapshotDir.entry[0]), rc);

        logMsg->status = rc;
        logMsg->number = ssEntry->snapshotNumber;
        logMsg->index = (ssEntry - &snapshotDir.entry[0]);
        logMsg->time = *&ssEntry->currentTime;
        logMsg->avlFlags = ssEntry->flags;
        logMsg->rstFlags = 0;
        strcpy((char *)logMsg->description, ssEntry->description);

        /*
         * Only save away the actual length of the log meesage (which is variable
         * because of the variable description string length).
         */
        SendAsyncEvent(LOG_SNAPSHOT_TAKEN, sizeof(LOG_SNAPSHOT_TAKEN_PKT) +
                       strlen((char *)logMsg->description), logMsg);

        Free(logMsg);
    }

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    LoadSnapshot()
**
** Description: Loads a system configuration snapshot
**
** Inputs:      seq - sequence number of the snapshot to load
**              flags - which fids to reload
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
INT32 LoadSnapshot(UINT32 seq, UINT32 flags)
{
    INT32       rc;
    INT32       rc1 = GOOD;
    INT32       rc2 = GOOD;
    INT32       rc3 = GOOD;
    INT32       loadable = 0;
    SNAPSHOT_ENTRY *ssEntry = NULL;
    LOG_SNAPSHOT_RESTORED_PKT *logMsg = NULL;
    INT32       Index = SeqNum2Index(seq);

    /*
     * lock the journaling mutex
     */
    (void)LockMutex(&configJournalMutex, MUTEX_WAIT);

    do
    {
        /*
         * Init/refresh the local snapshot directory
         */
        if ((rc = RefreshSnapshotDirectory()) != GOOD)
        {
            break;
        }

        /*
         * Verify that 'Index' was found
         */
        if (Index < 0)
        {
            rc = SNAPSHOT_LOAD_INDEX_OUT_OF_RANGE;
            dprintf(DPRINTF_DEFAULT, "LoadSnapshot: Entry out of range, exiting\n");
            break;
        }

        /*
         * Get pointer to entry
         */
        ssEntry = &snapshotDir.entry[Index];

        /*
         * Determine if this entry is "loadable."  Remember, entrys that
         * are marked as ERROR can be loaded, its only that not all requested
         * fids were saved, so obviously cannot be reloaded.
         * An entry marked as CRC can't be loaded -- it is known to be
         * corrupt.
         */
        switch (ssEntry->status)
        {
            case SNAPSHOT_STATUS_INUSE:
            case SNAPSHOT_STATUS_ERROR:
            case SNAPSHOT_STATUS_KEEP:
                /* These types can be loaded */
                break;

                /*
                 * All other types can't be loaded
                 */
            default:
                rc = SNAPSHOT_LOAD_NOT_LOADABLE;
                dprintf(DPRINTF_DEFAULT, "LoadSnapshot: Snapshot not loadable, exiting\n");
                break;
        }

        if (rc != GOOD)
        {
            break;
        }

        loadable = 1;

        /*
         * Determine if all of the requested fids can be loaded, before
         * loading any.
         */
        rc = GOOD;
        do
        {
            if ((flags & SNAPSHOT_FLAG_MASTER_CONFIG) &&
                (ssEntry->flags & SNAPSHOT_FLAG_MASTER_CONFIG) == 0)
            {
                rc = SNAPSHOT_LOAD_NOT_ALL_FIDS_AVAILABLE;
                break;
            }

            if ((flags & SNAPSHOT_FLAG_CTRL_MAP) &&
                (ssEntry->flags & SNAPSHOT_FLAG_CTRL_MAP) == 0)
            {
                rc = SNAPSHOT_LOAD_NOT_ALL_FIDS_AVAILABLE;
                break;
            }

            if ((flags & SNAPSHOT_FLAG_BE_NVRAM) &&
                (ssEntry->flags & SNAPSHOT_FLAG_BE_NVRAM) == 0)
            {
                rc = SNAPSHOT_LOAD_NOT_ALL_FIDS_AVAILABLE;
                break;
            }
        } while (FALSE);

        if (rc != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "LoadSnapshot: Not all requested FIDS are available to load, exiting\n");
            break;
        }

        /*
         * Copy/Load the requested fids.
         */
        if (flags & SNAPSHOT_FLAG_MASTER_CONFIG)
        {
            rc1 = FidToFidCopy(FS_FID_QM_MASTER_CONFIG, ssEntry->masterConfigFID);
        }

        if (flags & SNAPSHOT_FLAG_CTRL_MAP)
        {
            rc2 = FidToFidCopy(FS_FID_QM_CONTROLLER_MAP, ssEntry->controllerMapFID);
        }

        if (flags & SNAPSHOT_FLAG_BE_NVRAM)
        {
            if ((rc3 = FidToFidCopy(FS_FID_BE_NVRAM, ssEntry->beNVRAMFID)) == GOOD)
            {
                rc3 = RestoreBENVRAM(ssEntry->beNVRAMFID);
            }
        }

        if (rc1 != GOOD || rc2 != GOOD || rc3 != GOOD)
        {
            rc = SNAPSHOT_LOAD_ERROR_LOADING_A_FID;
            dprintf(DPRINTF_DEFAULT, "LoadSnapshot: Error reloading a FID, exiting\n");
            break;
        }
    } while (FALSE);

    /*
     * Unlock the journaling mutex.
     */
    UnlockMutex(&configJournalMutex);

    /*
     * Log a pass/fail message
     */
    logMsg = MallocWC(sizeof(LOG_SNAPSHOT_RESTORED_PKT) + SNAPSHOT_DESCRIPTION_LEN);

    logMsg->status = rc;
    if (loadable)
    {
        logMsg->number = ssEntry->snapshotNumber;
        logMsg->index = (ssEntry - &snapshotDir.entry[0]);
        logMsg->time = *&ssEntry->currentTime;
        logMsg->avlFlags = ssEntry->flags;
        logMsg->rstFlags = flags;
        strcpy((char *)logMsg->description, ssEntry->description);
    }

    /*
     * Only save away the actual length of the log meesage (which is variable
     * because of the variable description string length).
     */
    SendAsyncEvent(rc == GOOD ? LOG_SNAPSHOT_RESTORED : LOG_SNAPSHOT_RESTORE_FAILED,
                   sizeof(LOG_SNAPSHOT_RESTORED_PKT) +
                   strlen((char *)logMsg->description), logMsg);

    Free(logMsg);

    /*
     * The final step is to reset all of the controllers.  This will restart
     * the system with the selected configuration.
     */
    if (rc == GOOD)
    {
        rc = ResetAllControllers();
    }

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    ReadSnapshotDirectory()
**
** Description: Read up the snapshot directory and return to the caller
**
** Inputs:      buffer - buffer to read directory in to
**              length - length of the buffer
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
INT32 ReadSnapshotDirectory(char *buffer, UINT32 length)
{
    INT32       rc;

    do
    {
        if (buffer == NULL)
        {
            rc = ERROR;
            break;
        }

        /*
         * Init/refresh the local snapshot directory
         */
        rc = RefreshSnapshotDirectory();

        if (rc == GOOD)
        {
            memcpy(buffer, &snapshotDir,
                   length > sizeof(SNAPSHOT_DIR) ? sizeof(SNAPSHOT_DIR) : length);
        }
        else
        {
            rc = ERROR;
            break;
        }

    } while (FALSE);

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    ChangeSnapshot()
**
** Description: Change a system configuration snapshot in the directory
**
** Inputs:      seq - sequence number of the snapshot to load
**              status - what to change the status to (DEL etc)
**              description - description string to save with the snapshot.
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
INT32 ChangeSnapshot(UINT32 seq, UINT32 status, char *description)
{
    INT32       rc;
    SNAPSHOT_ENTRY *ssEntry = NULL;
    INT32       Index = SeqNum2Index(seq);

    do
    {
        /*
         * Init/refresh the local snapshot directory
         */
        if ((rc = RefreshSnapshotDirectory()) != GOOD)
        {
            break;
        }

        /*
         * Verify that 'Index' was found
         */
        if (Index < 0)
        {
            rc = ERROR;
            break;
        }

        /*
         * Get the entry
         */
        ssEntry = &snapshotDir.entry[Index];

        /*
         * Do the status change
         */
        switch (status)
        {
            case SNAPSHOT_STATUS_OPEN:
            case SNAPSHOT_STATUS_DELETED:
            case SNAPSHOT_STATUS_INUSE:
            case SNAPSHOT_STATUS_ERROR:
            case SNAPSHOT_STATUS_KEEP:
                ssEntry->status = status;
                break;

            case SNAPSHOT_STATUS_NOOP:
                break;

            default:
                rc = ERROR;
                break;
        }

        if (rc == ERROR)
        {
            break;
        }

        /*
         * Copy in new description if one provided
         */
        if (description)
        {
            memset(ssEntry->description, 0, sizeof(ssEntry->description));
            strncpy(ssEntry->description, description, sizeof(ssEntry->description) - 1);
        }
    } while (FALSE);

    /*
     * Write out the changed directory
     */
    if (rc == GOOD)
    {
        rc = WriteSnapshotDirectory();
    }

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    InitSnapshotFID()
**
** Description: Initialize the snapshot FID on first drive label.
**
** Inputs:      void
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
void InitSnapshotFID(void)
{
    memset(&snapshotDir, 0, sizeof(SNAPSHOT_DIR));

    /*
     * If FileSystemInitialized() is TRUE, go initialize the snapshot
     * directory on it.
     */
    if (FileSystemInitialized())
    {
        WriteFile(FS_FID_CKP_DIRECTORY, &snapshotDir, sizeof(SNAPSHOT_DIR));
    }

    return;
}


/*----------------------------------------------------------------------------
** Function:    WriteSnapshotDirectory()
**
** Description: Flush the snapshot directory to disk
**
** Inputs:      void
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 WriteSnapshotDirectory(void)
{
    INT32       rc = ERROR;

    if (FileSystemInitialized())
    {
        rc = WriteFile(FS_FID_CKP_DIRECTORY, &snapshotDir, sizeof(SNAPSHOT_DIR));
    }

    return rc ? SNAPSHOT_ERROR_WRITING_DIR : GOOD;
}


/*----------------------------------------------------------------------------
** Function:    FindNextEmptySnapshot()
**
** Description: Search the snapshot directory and return the next open
**              or useable snapshot entry.
**
** Inputs:      void
**
** Returns:     SNAPSHOT_ENTRY pointer to entry or NULL if error or none
**              found.
**
**--------------------------------------------------------------------------*/
static SNAPSHOT_ENTRY *FindNextEmptySnapshot(void)
{
    UINT32      i;
    SNAPSHOT_ENTRY *ssEntry = NULL;
    TIMESTAMP  *oldestP = NULL;
    TIMESTAMP  *newerP;
    INT32       found;

    do
    {
        /*
         * First look for an OPEN entry
         */
        for (i = 0; i < snapshotDir.numberOfSnapshots; i++)
        {
            if (snapshotDir.entry[i].status == SNAPSHOT_STATUS_OPEN)
            {
                ssEntry = &snapshotDir.entry[i];
                break;
            }
        }

        if (ssEntry)
        {
            break;
        }

        /*
         * If no OPEN entries, look for the OLDEST DELETED entry
         */
        found = -1;
        for (i = 0; i < snapshotDir.numberOfSnapshots; i++)
        {
            if (snapshotDir.entry[i].status == SNAPSHOT_STATUS_DELETED)
            {
                if (found == -1)
                {
                    oldestP = &snapshotDir.entry[i].currentTime;
                    found = i;
                }
                else
                {
                    newerP = RTC_NewerTimeStamp(oldestP, &snapshotDir.entry[i].currentTime);
                    if (newerP == oldestP)
                    {
                        oldestP = &snapshotDir.entry[i].currentTime;
                        found = i;
                    }
                }
            }
        }

        if (found != -1)
        {
            ssEntry = &snapshotDir.entry[found];
            break;
        }

        /*
         * If no DELETED entries, look for the OLDEST INUSE or ERROR entry
         */
        found = -1;
        for (i = 0; i < snapshotDir.numberOfSnapshots; i++)
        {
            if (snapshotDir.entry[i].status == SNAPSHOT_STATUS_INUSE ||
                snapshotDir.entry[i].status == SNAPSHOT_STATUS_ERROR)
            {
                if (found == -1)
                {
                    oldestP = &snapshotDir.entry[i].currentTime;
                    found = i;
                }
                else
                {
                    newerP = RTC_NewerTimeStamp(oldestP, &snapshotDir.entry[i].currentTime);
                    if (newerP == oldestP)
                    {
                        oldestP = &snapshotDir.entry[i].currentTime;
                        found = i;
                    }
                }
            }
        }

        if (found != -1)
        {
            ssEntry = &snapshotDir.entry[found];
            break;
        }

    } while (FALSE);

    return ssEntry;
}


/*----------------------------------------------------------------------------
** Function:    RefreshSnapshotDirectory()
**
** Description: Read up the snapshot directory from disk and initialize
**              if necessary.
**
** Inputs:      refresh - flag to indicate that a forced refresh to be done.
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 RefreshSnapshotDirectory(void)
{
    INT32       rc = GOOD;
    INT32       i;
    char        temp[20];

    dprintf(DPRINTF_DEFAULT, "RefreshSnapshotDirectory: Entry\n");

    do
    {
        /*
         * Read up the snapshot directory.
         */
        if (FileSystemInitialized())
        {
            rc = ReadFile(FS_FID_CKP_DIRECTORY, &snapshotDir, sizeof(snapshotDir));
            if (rc == FS_ERROR_READ_CRC_CHECK_HEADER ||
                rc == FS_ERROR_READ_CRC_CHECK_DATA)
            {
                /*
                 * Try to recover as much as we can
                 */
                dprintf(DPRINTF_DEFAULT, "RefreshSnapshotDirectory: rc %X, calling RecoverSnapshotDirectory.\n",
                        rc);
                rc = RecoverSnapshotDirectory();
                if (rc != GOOD)
                {
                    break;
                }
            }
        }
        else
        {
            rc = SNAPSHOT_FILESYSTEM_NOT_READY;
            break;
        }

        /*
         * Check the Magic Number, and Init Flag.  Finally check the schema.
         * Reinitialize if any bad.
         */
        if (snapshotDir.initialized != TRUE ||
            snapshotDir.magicNumber != SNAPSHOT_MAGIC_NUMBER ||
            snapshotDir.schema != SNAPSHOT_SCHEMA_1 ||
            (snapshotDir.useCRCFlag != 0 && snapshotDir.useCRCFlag != 1))
        {
            LogMessage(LOG_TYPE_DEBUG, "RefreshSnapshotDirectory: Reinitializing to defaults");
            memset(&snapshotDir, 0, sizeof(snapshotDir));
            memcpy(snapshotDir.eyecatcher, "SNAP DIR", sizeof(snapshotDir.eyecatcher));
            snapshotDir.nextSnapshotNumber = 100;
            snapshotDir.useCRCFlag = 1;
            for (i = 0; i < FS_NUM_SNAPSHOT_FIDS; i++)
            {
                sprintf(temp, "SNAP %02u ", i);
                memcpy(snapshotDir.entry[i].eyecatcher, temp,
                       sizeof(snapshotDir.entry[i].eyecatcher));
                snapshotDir.entry[i].status = SNAPSHOT_STATUS_OPEN;
                snapshotDir.entry[i].masterConfigFID = FS_FID_CKP_MASTER_CONFIG + i;
                snapshotDir.entry[i].controllerMapFID = FS_FID_CKP_CONTROLLER_MAP + i;
                snapshotDir.entry[i].beNVRAMFID = FS_FID_CKP_BE_NVRAM + i;

                /*
                 * Calculate a CRC over this entry
                 */
                snapshotDir.entry[i].CRC = CRC32(&snapshotDir.entry[i], sizeof(SNAPSHOT_ENTRY) - 4);
            }

            snapshotDir.initialized = TRUE;
            snapshotDir.magicNumber = SNAPSHOT_MAGIC_NUMBER;
            snapshotDir.schema = SNAPSHOT_SCHEMA_1;
            snapshotDir.numberOfSnapshots = FS_NUM_SNAPSHOT_FIDS;

            /*
             * Write out the init'd directory
             */
            rc = WriteSnapshotDirectory();
        }
        else
        {
            switch (snapshotDir.useCRCFlag)
            {
                case 0:
                    /*
                     * Convert to using CRC's
                     */
                    LogMessage(LOG_TYPE_DEBUG, "RefreshSnapshotDirectory: Adding CRC's to entries");
                    snapshotDir.useCRCFlag = 1;
                    for (i = 0; i < FS_NUM_SNAPSHOT_FIDS; i++)
                    {
                        /*
                         * Calculate a CRC over this entry
                         */
                        snapshotDir.entry[i].CRC = CRC32(&snapshotDir.entry[i], sizeof(SNAPSHOT_ENTRY) - 4);
                    }

                    /*
                     * Write out the init'd directory
                     */
                    rc = WriteSnapshotDirectory();

                    break;

                case 1:
                    /*
                     * Good to go as is
                     */
                    break;

                default:
                    /*
                     * Can't get here
                     */
                    break;
            }
        }

    } while (FALSE);

    if (rc != GOOD)
    {
        rc = SNAPSHOT_REFRESH_DIR_FAILED;
    }

    dprintf(DPRINTF_DEFAULT, "RefreshSnapshotDirectory: Exit, rc %X\n", rc);
    return rc;
}


/*----------------------------------------------------------------------------
** Function:    RecoverSnapshotDirectory()
**
** Description: Read up a corrupted snapshot directory and recover what you can.
**
** Inputs:      none
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 RecoverSnapshotDirectory(void)
{
    INT32       rc = GOOD;
    INT32       i;
    UINT32      crc;

    dprintf(DPRINTF_DEFAULT, "RecoverSnapshotDirectory: Entry\n");

    do
    {
        /*
         * Read up the snapshot directory, but skip the header.
         */
        rc = ReadFileAtOffset(FS_FID_CKP_DIRECTORY, 1, &snapshotDir, sizeof(snapshotDir));

        /*
         * If this fails, we're pretty screwed.
         */
        if (rc != GOOD)
        {
            LogMessage(LOG_TYPE_DEBUG, "RecoverSnapshotDirectory: read failed. rc %X", rc);
            break;
        }

        /*
         * First check the basic stuff
         */
        if (snapshotDir.initialized != TRUE ||
            snapshotDir.magicNumber != SNAPSHOT_MAGIC_NUMBER ||
            snapshotDir.schema != SNAPSHOT_SCHEMA_1 ||
            (snapshotDir.useCRCFlag != 0 && snapshotDir.useCRCFlag != 1))
        {
            /*
             * If any of this stuff is hozed, just bail out and let the
             * entire directory be reinitialized.
             */
            LogMessage(LOG_TYPE_DEBUG, "RecoverSnapshotDirectory: basics missing");
            rc = FAIL;
            break;
        }

        /*
         * Go through each entry and check the CRC.
         */
        switch (snapshotDir.useCRCFlag)
        {
            case 0:
                /*
                 * If its an old format (no CRC's), it will be fixed up in
                 * RefreshSnapshotDirectory(). Just exit.
                 */
                break;

            case 1:
                /*
                 * Check only those entries that are actually loadable
                 */
                LogMessage(LOG_TYPE_DEBUG, "RecoverSnapshotDirectory: Checking CRC's on entries");
                for (i = 0; i < FS_NUM_SNAPSHOT_FIDS; i++)
                {
                    switch (snapshotDir.entry[i].status)
                    {
                        case SNAPSHOT_STATUS_INUSE:
                        case SNAPSHOT_STATUS_ERROR:
                        case SNAPSHOT_STATUS_KEEP:

                            /*
                             * Calculate a CRC over this entry
                             */
                            crc = CRC32(&snapshotDir.entry[i], sizeof(SNAPSHOT_ENTRY) - 4);
                            if (snapshotDir.entry[i].CRC != crc)
                            {
                                /*
                                 * We found a bad one!!!
                                 */
                                snapshotDir.entry[i].status = SNAPSHOT_STATUS_CRC;
                                LogMessage(LOG_TYPE_DEBUG, "RecoverSnapshotDirectory: Marked index %d as bad crc",
                                           i);
                            }
                            break;

                        default:
                            break;
                    }
                }

                /*
                 * Write out the init'd directory
                 */
                rc = WriteSnapshotDirectory();

                break;

            default:
                /*
                 * Can't get here ...
                 */
                break;

        }

    } while (FALSE);

    if (rc != GOOD)
    {
        rc = SNAPSHOT_RECOVER_DIR_FAILED;
    }

    dprintf(DPRINTF_DEFAULT, "RecoverSnapshotDirectory: Exit, rc %d\n", rc);
    return rc;
}


/*----------------------------------------------------------------------------
** Function:    FidToFidCopy()
**
** Description: Copy one filesystem fid to another.
**
** Inputs:      destinationFID, sourceFID
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 FidToFidCopy(UINT32 destinationFID, UINT32 sourceFID)
{
    INT32       rc = PI_GOOD;
    MRFILECOPY_REQ *inPkt = NULL;
    MRFILECOPY_RSP *outPkt = NULL;
    INT32       mrpRc = PI_GOOD;

    inPkt = MallocWC(sizeof(*inPkt));
    outPkt = MallocSharedWC(sizeof(*outPkt));

    do
    {
        /*
         * Load up input packet and call the MRP.
         */
        memset(inPkt, 0, sizeof(*inPkt));
        inPkt->srcFID = sourceFID;
        inPkt->destFID = destinationFID;
        inPkt->length = GetFileSizeInBlocksIncHeader(sourceFID);

        if (inPkt->length == 0)
        {
            dprintf(DPRINTF_DEFAULT, "FidToFidCopy: Couldn't determine file size.\n");
            rc = PI_ERROR;
            break;
        }

        /*
         * Send the request to Thunderbolt.  This function handles timeout
         * conditions and task switches while waiting.
         */
#if 0
        dprintf(DPRINTF_DEFAULT, "FidToFidCopy: File Copy from FID %u to %u started\n",
                sourceFID, destinationFID);
#endif  /* 0 */

        mrpRc = PI_ExecMRP(inPkt, sizeof(*inPkt), MRFILECOPY,
                           outPkt, sizeof(*outPkt), MRP_FSYSOP_TIMEOUT);

        /*
         * At this point, we should have received the return packet.
         */
        switch (mrpRc)
        {
            case PI_GOOD:
                break;

            case PI_TIMEOUT:
                rc = FS_ERROR_FID2FID_PI_TIMEOUT;
                break;

            case PI_ERROR:
                /*
                 * If 2 or more disks written, call it a good write
                 */
                if (outPkt->good >= 2 && outPkt->error > 0 &&
                    outPkt->header.status == DEIOERR)
                {
                    SendFileioErr(LOG_FILEIO_DEBUG, FILEIO_FID2FID,
                                  rc, outPkt->header.status, destinationFID,
                                  outPkt->good, outPkt->error);
                    break;
                }

                /* Fall through */
            default:
                dprintf(DPRINTF_DEFAULT, "FidToFidCopy: File Copy MRP failed (rc = %d)\n",
                        rc);
                rc = FS_ERROR_FID2FID_PI_ERROR;
                break;
        }

        if (rc)
        {
            SendFileioErr(LOG_FILEIO_ERR, FILEIO_FID2FID,
                          rc, outPkt->header.status, destinationFID,
                          outPkt->good, outPkt->error);
        }
    } while (FALSE);

    /*
     * Cleanup
     */
    Free(inPkt);
    if (mrpRc != PI_TIMEOUT)
    {
        Free(outPkt);
    }

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    GetSnapStatus()
**
** Description: Converts a 'status' field to its ASCII equivalent.
**
** Inputs:      status
**
** Returns:     a pointer to a static string.
**
**--------------------------------------------------------------------------*/
static char textBuf[50];
static const char *GetSnapStatus(UINT32 status)
{
    const char *theStatus = NULL;

    switch (status)
    {
        case SNAPSHOT_STATUS_OPEN:
            theStatus = "OPEN";
            break;
        case SNAPSHOT_STATUS_DELETED:
            theStatus = "DELETED";
            break;
        case SNAPSHOT_STATUS_INUSE:
            theStatus = "IN USE";
            break;
        case SNAPSHOT_STATUS_ERROR:
            theStatus = "PARTIAL SAVE";
            break;
        case SNAPSHOT_STATUS_KEEP:
            theStatus = "KEEP";
            break;
        case SNAPSHOT_STATUS_CRC:
            theStatus = "CRC ERROR";
            break;
        default:
            sprintf(textBuf, "UNKNOWN (%u)", status);
            theStatus = textBuf;
            break;
    }

    return theStatus;
}


/*----------------------------------------------------------------------------
** Function:    GetSnapType()
**
** Description: Converts a 'type' field to its ASCII equivalent.
**
** Inputs:      type
**
** Returns:     a pointer to a static string.
**
**--------------------------------------------------------------------------*/
static const char *GetSnapType(UINT32 type)
{
    const char *theType = NULL;

    switch (type)
    {
        case SNAPSHOT_TYPE_MANUAL:
            theType = "MANUAL";
            break;
        case SNAPSHOT_TYPE_POWERUP:
            theType = "POWERUP";
            break;
        case SNAPSHOT_TYPE_SHUTDOWN:
            theType = "SHUTDOWN";
            break;
        case SNAPSHOT_TYPE_CONFIGCHG:
            theType = "CONFIG CHG";
            break;
        case SNAPSHOT_TYPE_HOTSPARE:
            theType = "HOTSPARE";
            break;
        default:
            sprintf(textBuf, "UNKNOWN (%u)", type);
            theType = textBuf;
            break;
    }

    return theType;
}


/*----------------------------------------------------------------------------
** Function:    GetFIDs()
**
** Description: Converts a 'flags' field to its ASCII equivalent, listing out
**              saved FIDs.
**
** Inputs:      flags
**
** Returns:     a pointer to a static string.
**
**--------------------------------------------------------------------------*/
static char *GetFIDs(UINT32 flags)
{
    char       *theFids = textBuf;

    strcpy(textBuf, "NONE, ");

    if (flags & SNAPSHOT_FLAG_MASTER_CONFIG)
    {
        theFids += sprintf(theFids, "Master Cfg, ");
    }

    if (flags & SNAPSHOT_FLAG_CTRL_MAP)
    {
        theFids += sprintf(theFids, "Ctrl Map, ");
    }

    if (flags & SNAPSHOT_FLAG_BE_NVRAM)
    {
        theFids += sprintf(theFids, "BE NVRAM, ");
    }

    /*
     * Truncate the ", "
     */
    textBuf[strlen(textBuf) - 2] = 0;

    return textBuf;
}


/*----------------------------------------------------------------------------
** Function:    FormatFWData()
**
** Description: Converts a 'FW_DATA' field to its ASCII equivalent.
**
** Inputs:      pointer to a FW_DATA field
**
** Returns:     a pointer to a static string.
**
**--------------------------------------------------------------------------*/
static char *FormatFWData(FW_DATA *fwP)
{
    char       *tbufP = textBuf;

    memcpy(tbufP, &fwP->revision, 4);
    tbufP += 4;
    *tbufP++ = ' ';

    memcpy(tbufP, &fwP->revCount, 4);
    tbufP += 4;
    *tbufP++ = ' ';

    memcpy(tbufP, &fwP->buildID, 4);
    tbufP += 4;
    *tbufP++ = ' ';

    sprintf(tbufP, "%02X/%02X/%04X %02X:%02X:%02X",
            fwP->timeStamp.month, fwP->timeStamp.date,
            fwP->timeStamp.year, fwP->timeStamp.hours,
            fwP->timeStamp.minutes, fwP->timeStamp.seconds);

    return textBuf;
}


/*----------------------------------------------------------------------------
** Function:    DisplaySnapshotDirectoryVerbose
**
** Description: Display the snapshot directory contents. Called by the
**              debug structure mechanism.
**
**              Called by the CCBE "STRUCT 4" command.
**
** Inputs:      void
**
** Returns:     len  - length of display string
**
** WARNING:     Uses Global Big Buffer
**
**--------------------------------------------------------------------------*/
UINT32 DisplaySnapshotDirectoryVerbose(void)
{
    UINT32      i,
                rc = GOOD;
    SNAPSHOT_ENTRY *ssEntry;
    UINT32      sorted[FS_NUM_SNAPSHOT_FIDS];

    /*
     * gBigBuffer is available here since the "bigBufferMutex" is locked
     * by the calling function (in pi_debug.c). If someone else calls this,
     * make sure you know what you are doing!
     */
    char       *bufP = gBigBuffer;

    /*
     * Write out header info
     */
    bufP += sprintf(bufP, "\nSnapshot Directory Contents\n");

    /*
     * sort the entries
     */
    SortSnapshotEntries(sorted);

    /*
     * Init/refresh the local snapshot directory
     */
    if ((rc = RefreshSnapshotDirectory()) != GOOD)
    {
        bufP += sprintf(bufP, "\n  ERROR initializing/reading the snapshot directory ...\n");
    }
    else
    {

        /*
         * Flush out everything else
         */
        bufP += sprintf(bufP, "\nDirectory:\n");
        bufP += sprintf(bufP, "  initialized:        %u\n", snapshotDir.initialized);
        bufP += sprintf(bufP, "  numberOfSnapshots:  %u\n", snapshotDir.numberOfSnapshots);
        bufP += sprintf(bufP, "  magicNumber:        0x%X\n", snapshotDir.magicNumber);
        bufP += sprintf(bufP, "  schema:             0x%X\n", snapshotDir.schema);
        bufP += sprintf(bufP, "  nextSnapshotNumber: %u\n", snapshotDir.nextSnapshotNumber);

        for (i = 0; i < snapshotDir.numberOfSnapshots; i++)
        {
            ssEntry = &snapshotDir.entry[sorted[i]];
            bufP += sprintf(bufP, "\nEntry %02u:\n", i);
            bufP += sprintf(bufP, "  snapshotNumber: %u\n", ssEntry->snapshotNumber);
            bufP += sprintf(bufP, "  timestamp:      %02X/%02X/%04X %02X:%02X:%02X (GMT)\n",
                        ssEntry->currentTime.month, ssEntry->currentTime.date,
                        ssEntry->currentTime.year, ssEntry->currentTime.hours,
                        ssEntry->currentTime.minutes, ssEntry->currentTime.seconds);
            bufP += sprintf(bufP, "  snapshotType:   %s\n", GetSnapType(ssEntry->snapshotType));
            bufP += sprintf(bufP, "  status:         %s\n", GetSnapStatus(ssEntry->status));
            bufP += sprintf(bufP, "  ccbRT:          %s\n", FormatFWData(&ssEntry->ccbRT));
            bufP += sprintf(bufP, "  ccbBoot:        %s\n", FormatFWData(&ssEntry->ccbBoot));
            bufP += sprintf(bufP, "  beRT:           %s\n", FormatFWData(&ssEntry->beRT));
            bufP += sprintf(bufP, "  beBoot:         %s\n", FormatFWData(&ssEntry->beBoot));
            bufP += sprintf(bufP, "  beDiag:         %s\n", FormatFWData(&ssEntry->beDiag));
            bufP += sprintf(bufP, "  feRT:           %s\n", FormatFWData(&ssEntry->feRT));
            bufP += sprintf(bufP, "  feBoot:         %s\n", FormatFWData(&ssEntry->feBoot));
            bufP += sprintf(bufP, "  feDiag:         %s\n", FormatFWData(&ssEntry->feDiag));
            bufP += sprintf(bufP, "  flags:          %s\n", GetFIDs(ssEntry->flags));
            bufP += sprintf(bufP, "  Master Cfg FID: %u\n", ssEntry->masterConfigFID);
            bufP += sprintf(bufP, "  Ctrl Map FID:   %u\n", ssEntry->controllerMapFID);
            bufP += sprintf(bufP, "  BE NVRAM FID:   %u\n", ssEntry->beNVRAMFID);
            bufP += sprintf(bufP, "  Description:    %s\n", ssEntry->description);
            bufP += sprintf(bufP, "  CRC:            0x%08X\n", ssEntry->CRC);
        }
    }

    return (bufP - gBigBuffer);
}


/*----------------------------------------------------------------------------
** Function:    DisplaySnapshotDirectory
**
** Description: Display the snapshot directory contents. Different from
**              DisplaySnapshotDirectoryVerbose() in that the data is written
**              to a malloc'd buffer instead of gBigBuffer.
**
**              Called from the Serial Port Menu.
**
** Inputs:      Pointer to a pointer to the buffer where data is stored.
**
** Returns:     len  - length of display string
**
** WARNING:     The Caller is responsible for freeing the buffer.
**
**--------------------------------------------------------------------------*/
UINT32 DisplaySnapshotDirectory(char **buffer)
{
    UINT32      i,
                rc = GOOD;
    SNAPSHOT_ENTRY *ssEntry;
    char       *bufP = NULL;
    UINT32      sorted[FS_NUM_SNAPSHOT_FIDS];

    /*
     * If no buffer, exit out immediately
     */
    if (buffer == NULL)
    {
        return 0;
    }

    /*
     * Get a buffer to write the formatted data to.  This buffer is passed
     * back to the caller and is freed there.
     */
    bufP = *buffer = MallocWC(0x10000);

    /*
     * Init/refresh the local snapshot directory
     */
    if ((rc = RefreshSnapshotDirectory()) != GOOD)
    {
        bufP += sprintf(bufP, "\n  ERROR initializing/reading the journaling directory ...\n");
    }
    else
    {
        bufP += sprintf(bufP, "    Entry#    Date/Time(GMT)     Description\n\n");

        /*
         * sort the entries
         */
        SortSnapshotEntries(sorted);

        /*
         * Now run through and print out the entries
         */
        for (i = 0; i < snapshotDir.numberOfSnapshots; i++)
        {
            ssEntry = &snapshotDir.entry[sorted[i]];
            if (ssEntry->snapshotNumber)
            {
                bufP += sprintf(bufP, "%10u  %02X/%02X/%04X %02X:%02X:%02X  %s\n",
                                ssEntry->snapshotNumber,
                                ssEntry->currentTime.month, ssEntry->currentTime.date,
                                ssEntry->currentTime.year, ssEntry->currentTime.hours,
                                ssEntry->currentTime.minutes,
                                ssEntry->currentTime.seconds, ssEntry->description);
            }
        }
        bufP += sprintf(bufP, "\n");
    }

    return bufP - *buffer;
}

/*----------------------------------------------------------------------------
** Function:    SortSnapshotEntries
**
** Description: Sort the Snapshot entries (bubble sort)
**
** Inputs:      Pointer to an array where the sorted list will be put.
**
** Returns:     void; but sorts the indices in 'sorted'
**
**--------------------------------------------------------------------------*/
static void SortSnapshotEntries(UINT32 *sorted)
{
    UINT32      i;
    UINT32      iterations;
    UINT32      tmp;

    /*
     * Initialize the list
     */
    iterations = snapshotDir.numberOfSnapshots;
    for (i = 0; i < iterations; i++)
    {
        sorted[i] = i;
    }

    /*
     * Bubble sort -- low to high.
     */
    if (iterations == 0)
    {
        return;
    }
    while (--iterations)
    {
        for (i = 0; i < iterations; i++)
        {
            if (snapshotDir.entry[sorted[i]].snapshotNumber >
                snapshotDir.entry[sorted[i + 1]].snapshotNumber)
            {
                /*
                 * Swap entries
                 */
                tmp = sorted[i];
                sorted[i] = sorted[i + 1];
                sorted[i + 1] = tmp;
            }
        }
    }

    return;
}


/*----------------------------------------------------------------------------
** Function:    SeqNum2Index
**
** Description: Find the snapshot index number given its sequence number
**
** Inputs:      sequence number
**
** Returns:     index number or -1 if seq num not found
**
**--------------------------------------------------------------------------*/
static INT32 SeqNum2Index(UINT32 seq)
{
    UINT32      i;
    INT32       Index = -1;

    for (i = 0; i < snapshotDir.numberOfSnapshots; i++)
    {
        if (snapshotDir.entry[i].snapshotNumber == seq)
        {
            Index = i;
        }
    }

    return Index;
}


/*----------------------------------------------------------------------------
** Function:    DisplaySnapshotDirectoryEntry()
**
** Description: Display the details of a snapshot directory entry.
**
**              Called from the Serial Port Menu.
**
** Inputs:      Pointer to a pointer to the buffer where data is stored.
**
** Returns:     len  - length of display string
**
** WARNING:     The Caller is responsible for freeing the buffer.
**
**--------------------------------------------------------------------------*/
UINT32 DisplaySnapshotDirectoryEntry(char **buffer, UINT32 seq)
{
    UINT32      rc = GOOD;
    SNAPSHOT_ENTRY *ssEntry;
    char       *bufP = NULL;
    INT32       Index = SeqNum2Index(seq);

    /*
     * If no buffer, exit out immediately
     */
    if (buffer == NULL)
    {
        return 0;
    }

    /*
     * Get a buffer to write the formatted data to.  This buffer is passed
     * back to the caller and is freed there.
     */
    bufP = *buffer = MallocWC(4096);

    /*
     * Init/refresh the local snapshot directory
     */
    if ((rc = RefreshSnapshotDirectory()) != GOOD)
    {
        bufP += sprintf(bufP, "\n  ERROR initializing/reading the journaling directory ...\n");
    }
    else
    {
        /*
         * Verify that 'Index' is within range
         */
        if (Index < 0)
        {
            bufP += sprintf(bufP, "\nEntry Number not found.\n");
        }
        else
        {
            ssEntry = &snapshotDir.entry[Index];
            bufP += sprintf(bufP, "Basic Information:\n");
            bufP += sprintf(bufP, "  Entry Number:    %u\n", ssEntry->snapshotNumber);
            bufP += sprintf(bufP, "  Slot Number      %02u\n", Index);
            bufP += sprintf(bufP, "  Timestamp:       %02X/%02X/%04X %02X:%02X:%02X GMT\n",
                        ssEntry->currentTime.month, ssEntry->currentTime.date,
                        ssEntry->currentTime.year, ssEntry->currentTime.hours,
                        ssEntry->currentTime.minutes, ssEntry->currentTime.seconds);
            bufP += sprintf(bufP, "  Description:     %s\n", ssEntry->description);

            bufP += sprintf(bufP, "\nFirmware at time entry was taken:\n");
            bufP += sprintf(bufP, "  FW:              %s\n", FormatFWData(&ssEntry->ccbRT));

            bufP += sprintf(bufP, "\nMiscellaneous:\n");
            bufP += sprintf(bufP, "  Type:            %s\n", GetSnapType(ssEntry->snapshotType));
            bufP += sprintf(bufP, "  Status:          %s\n", GetSnapStatus(ssEntry->status));
            bufP += sprintf(bufP, "  Available FIDs:  %s\n", GetFIDs(ssEntry->flags));
            bufP += sprintf(bufP, "  Master Cfg FID:  %u\n", ssEntry->masterConfigFID);
            bufP += sprintf(bufP, "  Ctrl Map FID:    %u\n", ssEntry->controllerMapFID);
            bufP += sprintf(bufP, "  BE NVRAM FID:    %u\n", ssEntry->beNVRAMFID);
            bufP += sprintf(bufP, "\n");
        }
    }

    return bufP - *buffer;
}


/*----------------------------------------------------------------------------
** Function:    SizeofSnapshotDirectory()
**
** Description: Return the size of the snapshot directory to the caller.
**
** Inputs:      void
**
** Returns:     uint size
**
**--------------------------------------------------------------------------*/
UINT32 SizeofSnapshotDirectory(void)
{
    return sizeof(snapshotDir);
}


/*----------------------------------------------------------------------------
** Function:    RestoreBENVRAM()
**
** Description: Restore the BE NVRAM from a filesystem FID.
**
** Inputs:      sourceFID
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 RestoreBENVRAM(UINT32 sourceFID)
{
    PI_PACKET_HEADER reqHeader;
    PI_PACKET_HEADER rspHeader;
    XIO_PACKET  reqPacket = { &reqHeader, NULL };
    XIO_PACKET  rspPacket = { &rspHeader, NULL };
    UINT32      rc = PI_GOOD;

    memset(&reqHeader, 0, sizeof(reqHeader));
    memset(&rspHeader, 0, sizeof(rspHeader));

    reqHeader.packetVersion = 1;
    rspHeader.packetVersion = 1;

    /*
     * Allocate memory for the request data.  The response data will be
     * allocated in the called function.
     */
    reqPacket.pPacket = MallocSharedWC(sizeof(PI_RESTORE_PROC_NVRAM_REQ));

    /*
     * Fill in the Header
     */
    reqPacket.pHeader->commandCode = PI_PROC_RESTORE_NVRAM_CMD;
    reqPacket.pHeader->length = sizeof(PI_RESTORE_PROC_NVRAM_REQ);

    /*
     * Fill in the request packet
     */
    ((PI_RESTORE_PROC_NVRAM_REQ *)(reqPacket.pPacket))->opt = MRNOFID | MRNORESTORE;
    ((PI_RESTORE_PROC_NVRAM_REQ *)(reqPacket.pPacket))->addr = (void *)sourceFID;

    /*
     * Issue the command through the packet command handler
     */
    dprintf(DPRINTF_DEFAULT, "RestoreBENVRAM: Restore from FID %u started\n", sourceFID);

    if (TestforMaster(GetMyControllerSN()))
    {
        rc = PortServerCommandHandler(&reqPacket, &rspPacket);
    }
    else
    {
        UINT8   retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

        do
        {
            if (rc != PI_TIMEOUT)
            {
                Free(rspPacket.pPacket);
            }
            else
            {
                rspPacket.pPacket = NULL;
            }
            rc = TunnelRequest(Qm_GetMasterControllerSN(), &reqPacket, &rspPacket);
        } while (rc != GOOD && (retries--) > 0);
    }

    switch (rc)
    {
        case PI_GOOD:
            dprintf(DPRINTF_DEFAULT, "RestoreBENVRAM: BE NVRAM restore successful.\n");
            break;

        case PI_TIMEOUT:
            dprintf(DPRINTF_DEFAULT, "RestoreBENVRAM: BE NVRAM restore timed out!\n");
            break;

        default:
            dprintf(DPRINTF_DEFAULT, "RestoreBENVRAM: BE NVRAM restore failed (%d)\n", rc);
            break;
    }

    /*
     * Free the allocated memory
     */
    Free(reqPacket.pPacket);
    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    if (rc != PI_GOOD)
    {
        rc = SNAPSHOT_ERROR_RESTORING_BE_NVRAM;
    }

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    ResetAllControllers()
**
** Description: Reset all controllers in the VCG to load the new config.
**
** Inputs:      none
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ResetAllControllers(void)
{
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };
    INT32       rc = PI_GOOD;
    UINT32      controllerSN;
    UINT32      configIndex;
    UINT32      i;

    dprintf(DPRINTF_DEFAULT, "ResetAllControllers: ENTER\n");

    /*
     * Allocate memory for the request (header and data) and the
     * response header.  Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_RESET_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Fill in the request header
     */
    reqPacket.pHeader->commandCode = PI_RESET_CMD;
    reqPacket.pHeader->length = sizeof(PI_RESET_REQ);

    /*
     * Fill in the request parms.
     */
    ((PI_RESET_REQ *)(reqPacket.pPacket))->which = PROCESS_ALL;
    ((PI_RESET_REQ *)(reqPacket.pPacket))->delayed = 1;

    /*
     * Loop through the controllers in the active controller map and send
     * the tunnel event.
     */
    for (i = 0; i < Qm_GetNumControllersAllowed(); i++)
    {
        configIndex = Qm_GetActiveCntlMap(i);

        /*
         * If the index == ACM_NODE_UNDEFINED then there is no controller at
         * this position and since the active controller map is packed there
         * are no more controller to look at.
         */
        if (configIndex == ACM_NODE_UNDEFINED)
        {
            break;
        }

        controllerSN = cntlConfigMap.cntlConfigInfo[configIndex].controllerSN;

        /*
         * If the port list request is for this controller skip it. We must
         * reset ourselves last! Otherwise tunnel the request to that
         * controller.
         */
        if (controllerSN == GetSerialNumber(CONTROLLER_SN))
        {
            /*
             * Issue the command through the top-level command handler.
             * Validate the ports and generate a port bit map to be used later.
             */
            continue;
        }
        else
        {
            UINT8   retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

            dprintf(DPRINTF_DEFAULT, "ResetAllControllers: resetting controller: %u\n",
                    controllerSN);

            do
            {
                if (rc != PI_TIMEOUT)
                {
                    Free(rspPacket.pPacket);
                }
                else
                {
                    rspPacket.pPacket = NULL;
                }
                rc = TunnelRequest(controllerSN, &reqPacket, &rspPacket);
            } while (rc != GOOD && (retries--) > 0);
        }

        if (rspPacket.pPacket && rc != PI_TIMEOUT)
        {
            Free(rspPacket.pPacket);
        }

        if (rc != PI_GOOD)
        {
            /*
             * If failure, don't continue
             */
            dprintf(DPRINTF_DEFAULT, "ResetAllControllers: Failed while resetting controller: %u\n",
                    controllerSN);
            break;
        }
    }

    if (rc == PI_GOOD)
    {
        /*
         * Reset ourselves by making the request to the port server directly.
         */
        dprintf(DPRINTF_DEFAULT, "ResetAllControllers: resetting *this* controller: %u\n",
                GetMyControllerSN());

        rc = PortServerCommandHandler(&reqPacket, &rspPacket);

        if (rspPacket.pPacket && rc != PI_TIMEOUT)
        {
            Free(rspPacket.pPacket);
        }
    }

    /*
     * Free the allocated memory
     */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_GOOD)
    {
        rc = SNAPSHOT_ERROR_RESETTING_CONTROLLERS;
    }

    dprintf(DPRINTF_DEFAULT, "ResetAllControllers: EXIT, rc = %d\n", rc);

    return rc;
}

/*--------------------------------------------------------------------------*/
typedef struct
{
    PCB *pcbPtr;
    UINT32   timeToFire;
    UINT32   moreToDo;
    UINT32   inTimedWait;
} DLYD_SNAPSHOT_CB;
/*
 * NOTE: this is in the bss section, and thus is zero upon startup.
 */
static DLYD_SNAPSHOT_CB delayedSnapshotCB;

/*----------------------------------------------------------------------------
** Function:    DelayedSnapshotTask()
**
** Description: The task that actually implements DelayedSnapshot();
**
** Inputs:      none
**
** Returns:     none
**
**--------------------------------------------------------------------------*/

static void DelayedSnapshotTask(UNUSED TASK_PARMS *parms)
{
    UINT32      timeNow;
    UINT32      sleepTime;

    dprintf(DPRINTF_DEFAULT, "DelayedSnapshotTask: entry\n");

    for (;;)
    {
        /* Flag that we have started to take a snapshot. */
        delayedSnapshotCB.moreToDo = FALSE;

        /* Take a snapshot immediately the first time. */
        TakeSnapshot(SNAPSHOT_TYPE_CONFIGCHG, "AUTO - Configuration Change");

        timeNow = RTC_GetSystemSeconds();

        if (timeNow < delayedSnapshotCB.timeToFire)
        {
            sleepTime = delayedSnapshotCB.timeToFire - timeNow;
            dprintf(DPRINTF_DEFAULT, "DelayedSnapshotTask: sleeping %u seconds\n", sleepTime);
            delayedSnapshotCB.inTimedWait = TRUE;
            TaskSleepMS(sleepTime * 1000);
            delayedSnapshotCB.inTimedWait = FALSE;
            dprintf(DPRINTF_DEFAULT, "DelayedSnapshotTask: woke up\n");
        }

        /*
         * TakeSnapshot() can context switch.  Another DelayedSnapshot() may
         * have been called, and there may be more to do so don't end the task.
         */
        if (delayedSnapshotCB.moreToDo == TRUE)
        {
            dprintf(DPRINTF_DEFAULT, "DelayedSnapshotTask: more to do\n");
            continue;
        }
        break;                  /* Done with snapshot taking. */
    }

    /*
    ** End the task.  No reason to leave it around for infrequent config
    ** change events. NOTE: pcbPtr cleared.
    */
    memset(&delayedSnapshotCB, 0, sizeof(DLYD_SNAPSHOT_CB));
    dprintf(DPRINTF_DEFAULT, "DelayedSnapshotTask: exit\n");
    return;
}


/*----------------------------------------------------------------------------
** Function:    DelayedSnapshot()
**
** Description: Takes a snapshot of the system configuration after a specified
**              wait.  This allows for a group of config operations to come
**              down together, and only one snapshot be taken after a period
**              of inactivity.
**
** Inputs:      Delay in seconds.  This is a minimum delay time.  The actual
**              delay may be longer.  However, if "0" is passed in, that
**              forces a snapshot immediately, and cancels any delays.
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
void DelayedSnapshot(UINT32 delay)
{
    UINT32      calcTime = RTC_GetSystemSeconds() + delay;

    /* Note: it is zero the first time. */
    if (calcTime > delayedSnapshotCB.timeToFire)
    {
        delayedSnapshotCB.timeToFire = calcTime;
    }

    /*
     * Start the Delayed Snapshot Task if not running
     */
    if (delayedSnapshotCB.pcbPtr == NULL)
    {
        delayedSnapshotCB.pcbPtr = TaskCreate(DelayedSnapshotTask, NULL);
    }
    else
    {
        /* Set the "moreToDo" flag in case the snapshot task is going to exit. */
        delayedSnapshotCB.moreToDo = TRUE;
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
