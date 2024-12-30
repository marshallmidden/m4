/* $Header$ */
/**
******************************************************************************
**
**  @file       logtrim.c
**
**  @version    $Revision: 70343 $
**
**  @brief      Log trimming utility
**
**  @author     Michael McMaster
**
**  @date       02/09/2005
**
**  This utility is responsible for writing all data that comes into stdin
**  to a user-specified file, and assuring that the file does not grow
**  beyond a predetermined size.
**
**  Copyright (c) 2005-2006 Xiotech Corporation. All rights reserved.
**     
******************************************************************************
**/

#include <stdint.h>
#include <stdbool.h>
#include <bzlib.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>

#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>


/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
typedef char                    RETURN_CODE;

#define GOOD    0
#define ERROR   1

#define LOGTRIM_VERSION_STRING  "1.1"

#define DEFAULT_FILE_PATH       "/var/log/xiotech/"
#define DEFAULT_FILE_NAME       "trim.xiolog"

#define MAX_FILE_PATH_LENGTH    80
#define MAX_FILE_NAME_LENGTH    80

#define FILE_LIMIT_MINIMUM      0x00010000  /* 64 KB  */
#define FILE_LIMIT_DEFAULT      0x00A00000  /* 10 MB  */
#define FILE_LIMIT_MAXIMUM      0x06000000  /* 100 MB */
#define FILE_LIMIT_TOO_BIG      (2 * FILE_LIMIT_MAXIMUM)

#define ROTATION_COUNT_MINIMUM  1
#define ROTATION_COUNT_DEFAULT  5
#define ROTATION_COUNT_MAXIMUM  99

#define FILE_EXTENSION_NONE     ".txt"
#define FILE_EXTENSION_GZIP     ".gz"
#define FILE_EXTENSION_BZIP2    ".bz2"

typedef enum
{
    COMPRESSION_TYPE_NONE  = 0,
    COMPRESSION_TYPE_GZIP  = 1,
    COMPRESSION_TYPE_BZIP2 = 2,
} COMPRESSION_TYPE;

#define COMPRESSION_TYPE_DEFAULT (COMPRESSION_TYPE_BZIP2)


/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/


/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/
typedef struct
{
    FILE            *pFile;
    char            aFilePath[MAX_FILE_PATH_LENGTH];
    char            aFileName[MAX_FILE_NAME_LENGTH];
    int64_t         currentFileSize;
    int64_t         maximumFileSize;
    int64_t         rotationLimit;
    uint32_t        compressionType;
    pthread_t       rotateThread;
    pthread_mutex_t rotateMutex;
    pthread_cond_t  rotateCondition;
    pthread_mutex_t archiveMutex;
    pthread_cond_t  archiveCondition;
    uint32_t        logWriteInProgress;
} LOG;


/*
******************************************************************************
** Private variables
******************************************************************************
*/
char gInputBuffer[0x00080000] = { 0 };
char gZipBuffer[0x00080000]   = { 0 };


/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/


/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
RETURN_CODE LogInit( LOG *pLog );
RETURN_CODE LogArchive( LOG *pLog );
RETURN_CODE LogOpen( LOG *pLog );
RETURN_CODE LogWrite( LOG *pLog, char *pBuffer, uint32_t length );
RETURN_CODE ProcessOptions( LOG *pLog, int32_t argc, char *argv[] );
int32_t LogPrint( LOG *pLog, char *pFormat, ... );
void *LogRotateTask( void *pLog );
uint32_t LogGZIPCompress( LOG *pLog );
uint32_t LogBZIP2Compress( LOG *pLog );


/*
******************************************************************************
** Code Start
******************************************************************************
*/
/**
******************************************************************************
**
**  @brief      Main start of program
**
**              Main start of program
**
**  @param      argc    - number of args to prgram
**  @param      argv    - array of string arguments
**
**  @return     Should not return
**
**  @attention  none
**
******************************************************************************
**/
int main( int argc, char *argv[] )
{
    RETURN_CODE     returnCode      = GOOD;
    uint32_t        bytesRead       = 0;
    char            aBuffer[8192]   = { 0 };
    struct timeval  selectTimeout   = { 5, 0 };
    LOG             myLog;          /* Uninitialized */
    fd_set          fdsRead;        /* Uninitialized */
    struct timeval  time;           /* Uninitialized */

    /*
    ** Initialize variables
    */
    returnCode = LogInit( &myLog );
    memset( &fdsRead, 0, sizeof(fdsRead) );
    memset( &time, 0, sizeof(time) );

    /*
    ** Process the command line arguments
    */
    if( returnCode == GOOD )
    {
        if( ProcessOptions(&myLog, argc, argv) != GOOD )
        {
            LogPrint( &myLog, "Failed to process command line options\n" );
            returnCode = ERROR;
        }
    }

    /*
    ** Create a new, larger, fully buffered stdin buffer.  We will also
    **  set the OS output buffering mode to line buffer to improve the
    ** 'tail' behavior.
    */
    if( returnCode == GOOD )
    {
        setvbuf( stdin, gInputBuffer, _IOFBF, sizeof(gInputBuffer) );
        setvbuf( stdout, NULL, _IOLBF, 0 );
        setvbuf( stderr, NULL, _IOLBF, 0 );
    }

    /*
    ** Open the output file
    */
    if( returnCode == GOOD )
    {
        if( LogOpen(&myLog) != GOOD )
        {
            LogPrint( &myLog, "Unable to open output file\n" );
            returnCode = ERROR;
        }
    }

    /*
    ** Set stdin to non-blocking
    */
    if( returnCode == GOOD )
    {
        if( fcntl(fileno(stdin), F_SETFL, fcntl(fileno(stdin), F_GETFL, 0) | O_NONBLOCK) )
        {
            LogPrint( &myLog, "Unable to change stdin to non-blocking: %s\n", strerror(errno) );
            returnCode = ERROR;
        }
    }

    /*
    ** Announce the LogTrim startup in the output file
    */
    if( returnCode == GOOD )
    {
        LogPrint( &myLog, "********************************************************************************\n" );
        LogPrint( &myLog, "**  Logtrim version %s running\n", LOGTRIM_VERSION_STRING );

        gettimeofday( &time, NULL );
        strftime( aBuffer, sizeof(aBuffer), "%F_%T", localtime(&time.tv_sec) );
        LogPrint( &myLog, "**    Time (local): %s\n", aBuffer );

        strftime( aBuffer, sizeof(aBuffer), "%F_%T", gmtime(&time.tv_sec) );
        LogPrint( &myLog, "**    Time (GMT):   %s\n", aBuffer );
        LogPrint( &myLog, "**    File size:    %lld bytes\n", myLog.currentFileSize );
        LogPrint( &myLog, "**    File limit:   %lld bytes\n", myLog.maximumFileSize );
        LogPrint( &myLog, "********************************************************************************\n" );
    }

    /*
    ** Wait for something to come in on stdin
    */
    while( returnCode == GOOD )
    {
        selectTimeout.tv_sec  = 60;
        selectTimeout.tv_usec = 0;

        FD_ZERO( &fdsRead );
        FD_SET( fileno(stdin), &fdsRead );

        if( select(fileno(stdin) + 1, &fdsRead, NULL, NULL, &selectTimeout) != -1 )
        {
            /*
            ** Verify that the select is not a timeout
            */
            if( FD_ISSET(fileno(stdin), &fdsRead) != 0 )
            {
                /*
                ** Handle any data that's waiting
                */
                do
                {
                    /*
                    ** Collect a chunk of data from stdin
                    */
                    bytesRead = fread( aBuffer, 1, sizeof(aBuffer), stdin );

                    /*
                    ** If we've collected some stdin data, write it to
                    ** the output file.
                    */
                    if( bytesRead > 0 )
                    {
                        if( LogWrite(&myLog, aBuffer, bytesRead) != GOOD )
                        {
                            LogPrint( &myLog, "Problem with LogWrite\n" );
                            returnCode = ERROR;
                        }
                    }
                    else if( feof(stdin) != 0 )
                    {
                        LogPrint( &myLog, "stdin reached EOF - pipe lost\n" );
                        returnCode = ERROR;
                    }
                } while( (bytesRead > 0) && (returnCode == GOOD) );
            }
            else
            {
#ifdef EXIT_ON_SELECT_TIMEOUT
                LogPrint( &myLog, "Select timeout - exiting\n" );
                returnCode = ERROR;
#endif /* EXIT_ON_SELECT_TIMEOUT */
            }
        }
        else
        {
            LogPrint( &myLog, "Select failed: %s\n", strerror(errno) );
            returnCode = ERROR;
        }
    }
    
    /*
    ** If the output file is open, make sure is closed
    */
    if( myLog.pFile != NULL )
    {
        if( fclose(myLog.pFile) == 0 )
        {
            myLog.pFile = NULL;
            LogPrint( &myLog, "File closed\n" );
        }
        else
        {
            LogPrint( &myLog, "Unable to close output file: %s\n", strerror(errno) );
        }
    }

    return( returnCode );
}


/**
******************************************************************************
**
**  @brief      ProcessOptions
**
**              Process the command line options into the program
**
**  @param      pLog    - pointer to a log structure
**  @param      argc    - number of args to prgram
**  @param      argv    - array of string arguments
**
**  @return     none    - exits on error
**
**  @attention  none
**
******************************************************************************
**/
RETURN_CODE ProcessOptions( LOG *pLog, int32_t argc, char *argv[] )
{
    RETURN_CODE returnCode      = GOOD;
    uint32_t    stringLength    = 0;
    uint8_t     moreArguments   = true;
    uint8_t     printHelpFlag   = false;

    const struct option longOptions[] =
    {
        {"compression", 1, 0, 'c'},
        {"file",        1, 0, 'f'},
        {"help",        0, 0, 'h'},
        {"path",        1, 0, 'p'},
        {"rotate",      1, 0, 'r'},
        {"size",        1, 0, 's'},
        {0,             0, 0, 0}
    };

    if( pLog != NULL )
    {
        while( moreArguments == true )
        {
            switch( getopt_long(argc, argv, "c:f:hH?p:r:s:", longOptions, NULL) )
            {
                case -1:
                    moreArguments = false;
                    break;

                case 'c':
                    if( sscanf(optarg, "%u", &pLog->compressionType) == 1 )
                    {
                        if( (pLog->compressionType != COMPRESSION_TYPE_NONE) &&
                            (pLog->compressionType != COMPRESSION_TYPE_GZIP) &&
                            (pLog->compressionType != COMPRESSION_TYPE_BZIP2) )
                        {
                            LogPrint( pLog, "Compression type %d unknown\n", pLog->compressionType );
                            returnCode = ERROR;
                        }
                    }
                    else
                    {
                        LogPrint( pLog, "Unable to determine file compression type\n" );
                        returnCode = ERROR;
                    }
                    break;

                case 'f':
                    stringLength = strlen( optarg );

                    /*
                    ** Copy the entire file name
                    */
                    if( stringLength < sizeof(pLog->aFileName) )
                    {
                        if( snprintf(pLog->aFileName, sizeof(pLog->aFileName), "%s", optarg) > 0 )
                        {
                            /*
                            ** Check for a valid file name
                            */
                            if( strlen(pLog->aFileName) < 1 )
                            {
                                LogPrint( pLog, "File name too short\n" );
                                returnCode = ERROR;
                            }
                            else if( strchr(pLog->aFileName, '/') != NULL )
                            {
                                LogPrint( pLog, "No path extensions allowed in the filename parameter\n" );
                                returnCode = ERROR;
                            }
                        }
                        else
                        {
                            LogPrint( pLog, "Internal path name copy failed\n" );
                            returnCode = ERROR;
                        }
                    }
                    else
                    {
                        LogPrint( pLog, "File name too long\n" );
                        returnCode = ERROR;
                    }
                    break;

                case 'h':
                case 'H':
                case '?':
                    printHelpFlag = true;
                    break;

                case 'p':
                    stringLength = strlen( optarg );

                    /*
                    ** Copy the entire path name
                    */
                    if( stringLength < sizeof(pLog->aFilePath) )
                    {
                        if( snprintf(pLog->aFilePath, sizeof(pLog->aFilePath), "%s", optarg) > 0 )
                        {
                            /*
                            ** Make sure there's space to append the slash
                            */
                            if( stringLength >= 1 )
                            {
                                if( pLog->aFilePath[stringLength - 1] != '/' )
                                {
                                    if( stringLength < sizeof(pLog->aFilePath) - 2 )
                                    {
                                        pLog->aFilePath[stringLength] = '/';
                                        pLog->aFilePath[stringLength + 1] = '\0';
                                    }
                                    else
                                    {
                                        LogPrint( pLog, "Path name too long\n" );
                                        returnCode = ERROR;
                                    }
                                }
                            }
                            else
                            {
                                LogPrint( pLog, "Path name too short\n" );
                                returnCode = ERROR;
                            }
                        }
                        else
                        {
                            LogPrint( pLog, "Internal file name copy failed\n" );
                            returnCode = ERROR;
                        }
                    }
                    else
                    {
                        LogPrint( pLog, "Path name too long\n" );
                        returnCode = ERROR;
                    }
                    break;

                case 'r':
                    if( sscanf(optarg, "%lld", &pLog->rotationLimit) == 1 )
                    {
                        if( (pLog->rotationLimit < ROTATION_COUNT_MINIMUM) ||
                            (pLog->rotationLimit > ROTATION_COUNT_MAXIMUM) )
                        {
                            LogPrint( pLog, "Rotation count out of accectable range\n" );
                            returnCode = ERROR;
                        }
                    }
                    else
                    {
                        LogPrint( pLog, "Unable to determine rotation count\n" );
                        returnCode = ERROR;
                    }
                    break;

                case 's':
                    if( sscanf(optarg, "%lld", &pLog->maximumFileSize) == 1 )
                    {
                        if( (pLog->maximumFileSize < FILE_LIMIT_MINIMUM) ||
                            (pLog->maximumFileSize > FILE_LIMIT_MAXIMUM) )
                        {
                            LogPrint( pLog, "Output file size limit out of accectable range\n" );
                            returnCode = ERROR;
                        }
                    }
                    else
                    {
                        LogPrint( pLog, "Unable to determine size\n" );
                        returnCode = ERROR;
                    }
                    break;

                default:
                    /* Unknown argument */
                    returnCode = ERROR;
                    break;
            }
        }

        /*
        ** Display the command line usage
        */
        if( (printHelpFlag == true) ||
            (returnCode != GOOD) )
        {
            LogPrint( pLog, "\n" );
            LogPrint( pLog, "LogTrim version %s\n", LOGTRIM_VERSION_STRING );
            LogPrint( pLog, "Usage: logtrim [OPTION...]\n" );
            LogPrint( pLog, "  -c, --compression <#>     Compression type (%u=None, %u=GZip, %u=BZip2)\n",
                COMPRESSION_TYPE_NONE, COMPRESSION_TYPE_GZIP, COMPRESSION_TYPE_BZIP2 );
            LogPrint( pLog, "                              Default: %u\n", COMPRESSION_TYPE_DEFAULT );
            LogPrint( pLog, "  -f, --filename <name>     Specify the output file name\n" );
            LogPrint( pLog, "  -p, --path <path>         Specify the output path\n" );
            LogPrint( pLog, "  -r, --rotate <decimal>    Specify the number of files in rotation\n" );
            LogPrint( pLog, "  -s, --size <decimal>      Specify the maximum output file size\n" );
            LogPrint( pLog, "                              Min: %u, Max: %u\n",
                FILE_LIMIT_MINIMUM, FILE_LIMIT_MAXIMUM );
            LogPrint( pLog, "Help options:\n" );
            LogPrint( pLog, "  -?, -h, --help            Show this help message\n" );

            /*
            ** If we print the help text, we need to bail out of the program
            */
            exit( returnCode );
        }
    }

    return( returnCode );
}


/**
******************************************************************************
**
**  @brief      LogArchive
**
**              This function compresses the current log file and reopens
**              a new log file with the same name.
**
**  @param      pLog - pointer to the current log
**
**  @return     GOOD or ERROR
**
**  @attention  Don't make any calls to LogPrint within this function
**
******************************************************************************
**/
RETURN_CODE LogArchive( LOG *pLog )
{
    RETURN_CODE returnCode = GOOD;
    uint32_t message = 0;

    if( (pLog != NULL) &&
        (pLog->pFile != NULL) )
    {
        LogPrint( pLog, "Log file size: %llu\n", pLog->currentFileSize );

        if( pLog->rotateThread != 0 )
        {
            /*
            ** Signal the rotate thread to continue.
            ** NOTE: This will wait for a previous invocation of the rotate
            ** thread to end before the lock will succeed.
            */
            LogPrint( pLog, "LogArchive: lock rotate mutex\n" );
            while( pthread_mutex_trylock(&pLog->rotateMutex) != 0 )
            {
                if( message == 0 )
                {
                    LogPrint( pLog, "LogArchive: waiting for rotate mutex to be unlocked\n" );
                    message = 1;
                }
            }

            LogPrint( pLog, "LogArchive: sending rotate signal\n" );
            pthread_cond_signal( &pLog->rotateCondition );

            /*
            ** Wait for rotate thread to signal archive to continue.
            ** NOTE: We must lock the archive mutex before unlocking
            ** the rotate mutex so that we won't ever miss the archive
            ** signal when the rotate thread sends it.
            */
            LogPrint( pLog, "LogArchive: lock archive mutex\n" );
            pthread_mutex_lock( &pLog->archiveMutex );

            LogPrint( pLog, "LogArchive: unlock rotate mutex\n" );
            pthread_mutex_unlock( &pLog->rotateMutex );

            LogPrint( pLog, "LogArchive: waiting for archive signal\n" );
            pthread_cond_wait( &pLog->archiveCondition, &pLog->archiveMutex );

            LogPrint( pLog, "LogArchive: unlock archive mutex\n" );
            pthread_mutex_unlock( &pLog->archiveMutex );
        }
        else
        {
            LogPrint( pLog, "LogArchive: rotate thread not yet created\n" );
        }
    }
    else
    {
        returnCode = ERROR;
    }

    return( returnCode );
}


/**
******************************************************************************
**
**  @brief      LogRotateTask
**
**              Rotate the file the same way that the logrotate does
**
**  @param      pLog - pointer to a log structure
**
**  @return     0 on success
**
**  @attention  This thread's rotateMutex is locked at startup.  It is only
**              unlocked in the condition wait.  The archive thread will signal
**              this thread when it's time for this to run.  The rotate thread
**              signals the archive thread after a new output file has
**              been established.
**
******************************************************************************
**/
void *LogRotateTask( void *pTempLog )
{
    RETURN_CODE returnCode          = ERROR;
    int32_t     rotationCounter     = 0;
    LOG         *pLog               = (LOG *)pTempLog;
    char aSourceFile[sizeof(pLog->aFilePath) + sizeof(pLog->aFilePath) + 10] = { 0 };
    char aDestinationFile[sizeof(pLog->aFilePath) + sizeof(pLog->aFilePath) + 10] = { 0 };
    char aCpCmd[((sizeof(pLog->aFilePath) + sizeof(pLog->aFileName)) * 2) + 10] = { 0 };

    while( pLog != NULL )
    {
        /*
        ** Wait for archive thread to signal rotate to continue
        */
        pthread_cond_wait( &(pLog->rotateCondition), &(pLog->rotateMutex) );
        LogPrint( pLog, "LogRotateTask: received rotate signal\n" );

        if( pLog->pFile != NULL )
        {
            /*
            ** Move the current output file (uncompressed) 
            */
            snprintf( aSourceFile, sizeof(aSourceFile), "%s%s", 
                pLog->aFilePath, pLog->aFileName );

            snprintf( aDestinationFile, sizeof(aDestinationFile), "%s%s.0", 
                pLog->aFilePath, pLog->aFileName );
            
            snprintf( aCpCmd, sizeof(aCpCmd), "cp -f %s %s", 
                aSourceFile, aDestinationFile );

            unlink( aDestinationFile );

            if ( system(aCpCmd) == -1 )
            {
                LogPrint( pLog, "Copy failed from %s to %s: %s\n", 
                          aSourceFile, aDestinationFile, strerror(errno) );
            } 

            /*
            ** Open a new output file (truncate to zero length)
            */
            snprintf( aSourceFile, sizeof(aSourceFile), "%s%s", 
                pLog->aFilePath, pLog->aFileName );
            
            if ( ftruncate(fileno(pLog->pFile), 0) != 0 )
            {
                LogPrint( pLog, "Truncate failed: %s\n", strerror(errno) );
                
                fclose( pLog->pFile );
                pLog->pFile = NULL;
                unlink( aSourceFile );
                pLog->pFile = fopen( aSourceFile, "w+" );
            }

            if( pLog->pFile != NULL )
            {
                /*
                ** Reset the file size to zero - we just truncated it above
                */
                pLog->currentFileSize = 0;
                returnCode = GOOD;
            }
            else
            {
                LogPrint( pLog, "Unable to reopen output file: %s\n", strerror(errno) );
            }

            /*
            ** Signal the archive thread to continute to run
            */
            LogPrint( pLog, "LogRotateTask: lock archive mutex\n" );
            pthread_mutex_lock( &(pLog->archiveMutex) );

            LogPrint( pLog, "LogRotateTask: sending archive signal\n" );
            pthread_cond_signal( &(pLog->archiveCondition) );

            LogPrint( pLog, "LogRotateTask: unlock archive mutex\n" );
            pthread_mutex_unlock( &(pLog->archiveMutex) );

            if( returnCode == GOOD )
            {
                char *pFileExtension = "";

                snprintf( aSourceFile, sizeof(aSourceFile), "%s%s.0",
                    pLog->aFilePath, pLog->aFileName );

                /*
                ** Compress the old output file into a temporary zip file.
                ** NOTE: LogRotate will move this when it's called.
                */
                switch( pLog->compressionType )
                {
                    case COMPRESSION_TYPE_NONE:
                        LogPrint( pLog, "LogRotateTask: No compression\n" );
                        pFileExtension = FILE_EXTENSION_NONE;

                        /* Place the uncompressed extention on the file */
                        snprintf( aDestinationFile, sizeof(aDestinationFile), "%s%s.0%s",
                            pLog->aFilePath, pLog->aFileName, FILE_EXTENSION_NONE );

                        if( rename(aSourceFile, aDestinationFile) == 0 )
                        {
                            LogPrint( pLog, "%s renamed to %s\n", aSourceFile, aDestinationFile );
                        }
                        else
                        {
                            LogPrint( pLog, "Error renaming %s to %s\n",
                                aSourceFile, aDestinationFile );

                            returnCode = ERROR;
                        }
                        break;

                    case COMPRESSION_TYPE_GZIP:
                        LogPrint( pLog, "LogRotateTask: Compress using GZip\n" );
                        pFileExtension = FILE_EXTENSION_GZIP;
                        returnCode = LogGZIPCompress( pLog );
                        break;

                    case COMPRESSION_TYPE_BZIP2:
                        LogPrint( pLog, "LogRotateTask: Compress using BZip2\n" );
                        pFileExtension = FILE_EXTENSION_BZIP2;
                        returnCode = LogBZIP2Compress( pLog );
                        break;

                    default:
                        LogPrint( pLog, "LogRotateTask: ERROR-Unknown compression (%d)\n",
                            pLog->compressionType );
                        returnCode = ERROR;
                        break;
                }

                /*
                ** Remove the uncompressed version of the output file
                */
                unlink( aSourceFile );

                if( returnCode == GOOD )
                {
                    /*
                    ** Rotate the log files
                    */
                    LogPrint( pLog, "LogRotateTask: Rotate the files\n" );

                    for( rotationCounter = ROTATION_COUNT_MAXIMUM;
                         rotationCounter > 0;
                         rotationCounter-- )
                    {
                        snprintf( aSourceFile, sizeof(aSourceFile), "%s%s.%u%s", 
                            pLog->aFilePath, pLog->aFileName, (rotationCounter - 1), pFileExtension );

                        snprintf( aDestinationFile, sizeof(aDestinationFile), "%s%s.%u%s", 
                            pLog->aFilePath, pLog->aFileName, rotationCounter, pFileExtension );

                        if( rotationCounter > pLog->rotationLimit )
                        {
                            /* Remove any unwanted archive files */
                            if( unlink(aDestinationFile) == 0 )
                            {
                                LogPrint( pLog, "%s removed\n", aDestinationFile );
                            }
                        }
                        else
                        {
                            /* Age the archive files by one */
                            if( rename(aSourceFile, aDestinationFile) == 0 )
                            {
                                LogPrint( pLog, "%s renamed to %s\n", aSourceFile, aDestinationFile );
                            }
                        }
                    }
                }
                else
                {
                    LogPrint( pLog, "LogRotateTask: Error compressing the output file\n" );
                }
            }
        }
        else
        {
            LogPrint( pLog, "LogRotateTask: file pointer is NULL - nothing to do\n" );

            /*
            ** Signal the archive thread to continute to run
            */
            LogPrint( pLog, "LogRotateTask: lock archive mutex\n" );
            pthread_mutex_unlock( &(pLog->archiveMutex) );

            LogPrint( pLog, "LogRotateTask: sending archive signal\n" );
            pthread_cond_signal( &(pLog->archiveCondition) );

            LogPrint( pLog, "LogRotateTask: unlock archive mutex\n" );
            pthread_mutex_unlock( &(pLog->archiveMutex) );
        }

        LogPrint( pLog, "LogRotateTask: waiting for rotate signal\n" );
    }

    LogPrint( pLog, "LogRotateTask: exiting\n" );

    return( NULL );
}


/**
******************************************************************************
**
**  @brief      LogWrite
**
**              Write to the output file
**
**  @param      pFile   - file pointer for the output file
**  @param      pBuffer - pointer to the buffer to be written (source)
**  @param      length  - number of bytes to write from buffer
**
**  @return     GOOD or ERROR
**
**  @attention  none
**
******************************************************************************
**/
RETURN_CODE LogWrite( LOG *pLog, char *pBuffer, uint32_t length )
{
    RETURN_CODE returnCode = GOOD;

    if( (pLog != NULL) &&
        (pLog->pFile != NULL) )
    {
        if( pLog->logWriteInProgress == false )
        {
            /*
            ** Prevent any LogWrite calls from causing an infinite loop
            */
            pLog->logWriteInProgress = true;

            /*
            ** Check if this chunk will increase the log file size
            ** beyond the specified size limit.
            */
            if( (length + pLog->currentFileSize) > pLog->maximumFileSize )
            {
                if( LogArchive(pLog) != GOOD )
                {
                    LogPrint( pLog, "Unable to archive current output file\n" );
                    returnCode = ERROR;
                }
            }

            if( returnCode == GOOD )
            {
                if( fwrite(pBuffer, 1, length, pLog->pFile) == length )
                {
                    /*
                    ** Track how many bytes we've added to the file
                    */
                    pLog->currentFileSize += length;
                    returnCode = GOOD;
                }
                else
                {
                    LogPrint( pLog, "ERROR: Unable to write to output file\n" );
                }
            }

            pLog->logWriteInProgress = false;
        }
    }
    else
    {
        returnCode = ERROR;
    }

    return( returnCode );
}


/**
******************************************************************************
**
**  @brief      PrintLog
**
**              This is a standard way for messages to be printed from
**              within the logtrim application.
**
**  @param      pLog - pointer to the log file (or NULL)
**  @param      Standard 'printf' arguments
**
**  @return     Standard 'printf' returns
**
**  @attention  none
**
******************************************************************************
**/
int32_t LogPrint( LOG *pLog, char *pFormat, ... )
{
    int32_t returnValue     = 0;
    va_list ap;             /* Uninitialized */
    char    aBuffer[1000];  /* Uninitialized */

    /* Prepare the variable argument list */
    va_start( ap, pFormat );

    /*
    ** Send formatted string to the screen
    */
    returnValue = vsnprintf( aBuffer, sizeof(aBuffer), pFormat, ap );

    if( returnValue > 0 )
    {
        fwrite( aBuffer, 1, returnValue, stdout );

        /*
        ** Write to the output file, if open
        */
        if( (pLog != NULL) &&
            (pLog->pFile != NULL) )
        {
            LogWrite( pLog, aBuffer, returnValue );
            fflush( pLog->pFile );
        }
    }

    /* Clean up the variable argument list */
    va_end( ap );

    return( returnValue );
}


/**
******************************************************************************
**
**  @brief      LogOpen
**
**              Open the specified log file
**
**  @param      pFile      - file pointer for the output file
**  @param      appendFlag - true will open in append mode
**
**  @return     GOOD or ERROR
**
**  @attention  none
**
******************************************************************************
**/
RETURN_CODE LogOpen( LOG *pLog )
{
    RETURN_CODE returnCode = GOOD;
    char aFullFileName[sizeof(pLog->aFilePath) + sizeof(pLog->aFilePath)] = { 0 };
    struct stat fileStats;  /* Uninitialized */

    /*
    ** Initialize the local variables
    */
    memset( &fileStats, 0, sizeof(fileStats) );

    if( pLog != NULL )
    {
        if( pLog->pFile == NULL )
        {
            snprintf( aFullFileName, sizeof(aFullFileName), "%s%s", 
                pLog->aFilePath, pLog->aFileName );

            /*
            ** Determine the current size of the output file
            */
            if( stat(aFullFileName, &fileStats) == 0 )
            {
                pLog->currentFileSize = fileStats.st_size;
            }
            else
            {
                LogPrint( pLog, "Unable to stat output file: %s\n", strerror(errno) );
            }

            /*
            ** Open the output file
            */
            if( pLog->currentFileSize < FILE_LIMIT_TOO_BIG )
            {
                /* Open - append */
                pLog->pFile = fopen( aFullFileName, "a+" );
            }
            else
            {
                /* Open - trucate to zero length */
                pLog->pFile = fopen( aFullFileName, "w+" );
            }

            /*
            ** Verify that the output file was opened
            */
            if( pLog->pFile != NULL )
            {
                LogPrint( pLog, "Output file [%s] opened\n", aFullFileName );
            }
            else
            {
                LogPrint( pLog, "Unable to open output file: %s\n", strerror(errno) );
                returnCode = ERROR;
            }

            if( returnCode == GOOD )
            {
                /*
                ** Set the OS output buffering mode to line buffer mode.  Without
                ** this setting, a 'tail' of the file is not up-to-date with
                ** what has been written.
                */
                if( setvbuf(pLog->pFile, NULL, _IOLBF, 0) != 0 )
                {
                    LogPrint( pLog, "Unable to configure output file line buffering\n" );
                    returnCode = ERROR;
                }
            }
        }
        else
        {
            LogPrint( pLog, "Output file already open\n" );
        }
    }
    else
    {
        returnCode = ERROR;
    }

    return( returnCode );
}


/**
******************************************************************************
**
**  @brief      LogInit
**
**              Initialize the specified log structure
**
**  @param      pLog - pointer for the log structure
**
**  @return     GOOD or ERROR
**
**  @attention  none
**
******************************************************************************
**/
RETURN_CODE LogInit( LOG *pLog )
{
    RETURN_CODE     returnCode          = ERROR;
    pthread_attr_t  pthreadAttributes;  /* Uninitialized */

    if( pLog != NULL )
    {
        /*
        ** Initialize the log structure with default values
        */
        memset( pLog, 0, sizeof(*pLog) );
        strncpy( pLog->aFilePath, DEFAULT_FILE_PATH, sizeof(pLog->aFilePath) );
        strncpy( pLog->aFileName, DEFAULT_FILE_NAME, sizeof(pLog->aFileName) );
        pLog->currentFileSize = 0;
        pLog->maximumFileSize = FILE_LIMIT_DEFAULT;
        pLog->rotationLimit = ROTATION_COUNT_DEFAULT;
        pLog->compressionType = COMPRESSION_TYPE_DEFAULT;
        pthread_mutex_init( &pLog->rotateMutex, NULL );
        pthread_cond_init( &pLog->rotateCondition, NULL );
        pthread_mutex_init( &pLog->archiveMutex, NULL );
        pthread_cond_init( &pLog->archiveCondition, NULL );
        pLog->logWriteInProgress = false;

        /*
        ** The rotate mutex will only be unlocked with via the conditional wait
        */
        pthread_mutex_lock( &(pLog->rotateMutex) );

        /*
        ** Create and initialize pthread attributes
        */
        pthread_attr_init( &pthreadAttributes );
        pthread_attr_setdetachstate( &pthreadAttributes, PTHREAD_CREATE_DETACHED );

        if( pthread_create(&pLog->rotateThread, &pthreadAttributes, LogRotateTask, pLog) == 0 )
        {
            returnCode = GOOD;
        }

        /*
        ** Release pthread attributes
        */
        pthread_attr_destroy( &pthreadAttributes );
    }

    return( returnCode );
}


/**
******************************************************************************
**
**  @brief      LogGZIPCompress
**
**              Compress the archvie file using GZip
**
**  @param      pLog - pointer for the log structure
**
**  @return     GOOD or ERROR
**
**  @attention  none
**
******************************************************************************
**/
uint32_t LogGZIPCompress( LOG *pLog )
{
    /*
    ** Compress the old output file into a temporary zip file.
    ** NOTE: LogRotate will move this when it's called.
    */
    gzFile pSource          = NULL;
    gzFile pDestination     = NULL;
    uint32_t returnCode       = GOOD;
    int32_t  sourceCount      = 0;
    int32_t  destinationCount = 0;
    char aSourceFile[sizeof(pLog->aFilePath) + sizeof(pLog->aFilePath) + 10] = { 0 };
    char aDestinationFile[sizeof(pLog->aFilePath) + sizeof(pLog->aFilePath) + 10] = { 0 };

    LogPrint( pLog, "GZip: Compressing the output file - start\n" );

    snprintf( aSourceFile, sizeof(aSourceFile), "%s%s.0",
        pLog->aFilePath, pLog->aFileName );

    snprintf( aDestinationFile, sizeof(aDestinationFile), "%s%s.0%s",
        pLog->aFilePath, pLog->aFileName, FILE_EXTENSION_GZIP );

    /*
    ** Open the files used during the compression
    */
    pSource = gzopen( aSourceFile, "r" );
    pDestination = gzopen( aDestinationFile, "w+" );

    if( (pSource != NULL) &&
        (pDestination != NULL) )
    {
        do
        {
            sourceCount = gzread( pSource, gZipBuffer, sizeof(gZipBuffer) );

            if( sourceCount > 0 )
            {
                destinationCount = gzwrite( pDestination, gZipBuffer, sourceCount );

                if( sourceCount != destinationCount )
                {
                    LogPrint( pLog, "GZip: Problem writing to compressed file\n" );
                    LogPrint( pLog, "GZip:   Read: %d, Write: %d\n", sourceCount, destinationCount );

                    returnCode = ERROR;
                }
            }
        } while( (sourceCount > 0) && (returnCode == GOOD) );
    }
    else
    {
        LogPrint( pLog, "GZip: File open failed\n" );
        returnCode = ERROR;
    }

    /*
    ** Close the files used during the compression
    */
    gzclose( pSource );
    gzclose( pDestination );

    return( returnCode );
}


/**
******************************************************************************
**
**  @brief      LogBZIP2Compress
**
**              Compress the archvie file using BZip2
**
**  @param      pLog - pointer for the log structure
**
**  @return     GOOD or ERROR
**
**  @attention  none
**
******************************************************************************
**/
uint32_t LogBZIP2Compress( LOG *pLog )
{
    /*
    ** Compress the old output file into a temporary zip file.
    ** NOTE: LogRotate will move this when it's called.
    */
    FILE   *pSource         = NULL;
    FILE   *pDestination    = NULL;
    BZFILE *pBZIP           = NULL;
    uint32_t returnCode       = GOOD;
    size_t  sourceCount      = 0;
    size_t  destinationCount = 0;
    int    bzError          = BZ_OK;
    char   aSourceFile[sizeof(pLog->aFilePath) + sizeof(pLog->aFilePath) + 10] = { 0 };
    char   aDestinationFile[sizeof(pLog->aFilePath) + sizeof(pLog->aFilePath) + 10] = { 0 };

    LogPrint( pLog, "BZip2: Compressing the output file - start\n" );

    snprintf( aSourceFile, sizeof(aSourceFile), "%s%s.0",
        pLog->aFilePath, pLog->aFileName );

    snprintf( aDestinationFile, sizeof(aDestinationFile), "%s%s.0%s",
        pLog->aFilePath, pLog->aFileName, FILE_EXTENSION_BZIP2 );

    /*
    ** Open the files used during the compression
    */
    pSource = fopen( aSourceFile, "r" );
    pDestination = fopen( aDestinationFile, "w+" );

    if( (pSource != NULL) &&
        (pDestination != NULL) )
    {
        pBZIP = BZ2_bzWriteOpen( &bzError, pDestination, 9, 0, 0 );

        if( (pBZIP != NULL) &&
            (bzError == BZ_OK) )
        {
            do
            {
                sourceCount = fread( gZipBuffer, 1, sizeof(gZipBuffer), pSource );

                if( sourceCount > 0 )
                {
                    BZ2_bzWrite( &bzError, pBZIP, gZipBuffer, sourceCount );

                    if( bzError != BZ_OK )
                    {
                        LogPrint( pLog, "BZip2: I/O Error (%d)\n", bzError );
                        returnCode = ERROR;
                    }
                }
            } while( (sourceCount > 0) && (returnCode == GOOD) );
        }
        else
        {
            LogPrint( pLog, "BZip2: Archive file open failed (%d)\n", bzError );
            returnCode = ERROR;
        }

        /*
        ** Release all BZip resources associated with the file
        */
        BZ2_bzWriteClose( &bzError, pBZIP, 0, &sourceCount, &destinationCount );

        if( bzError == BZ_OK )
        {
            LogPrint( pLog, "BZip2: Archive file closed\n", sourceCount, destinationCount );
        }
        else
        {
            LogPrint( pLog, "BZip2: Problem closing archive file (%d)\n", bzError );
            returnCode = ERROR;
        }
        LogPrint( pLog, "BZip2:   Read:%u, Write:%u\n", sourceCount, destinationCount );
    }
    else
    {
        LogPrint( pLog, "BZip2: File open failed\n" );
        returnCode = ERROR;
    }

    /*
    ** Close the files used during the compression
    */
    fclose( pSource );
    fclose( pDestination );

    return( returnCode );
}
