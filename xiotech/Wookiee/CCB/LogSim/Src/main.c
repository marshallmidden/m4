/* $Id: main.c 137382 2010-04-19 16:02:18Z mdr $ */
/**
******************************************************************************
**
**  @file   main.c
**
**  @brief  Main LogSim Program
**
**  Copyright (c) 2001-2003,2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include <stdio.h>
#include <stdlib.h>
#include "LogSimFuncs.h"    
#include "logging.h"
#include "logview.h"
#include "LOG_Defs.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define MAX_BUFFERSIZE  1024

/*****************************************************************************
** Private variables
*****************************************************************************/
static char buffer[MAX_BUFFERSIZE];

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
extern void LogTest(void);

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void help(void);
static void MainMenu(void);
static void TestLogMenu(void);
static void LoadLogMenu(void);
static void LogInfo(void);

static void QuickSearch(void);
static void QuickSearchExtended(void);

static char readChar(void);
static bool readString(char *buf);
static bool readUINT32(UINT32 *number);
static void clearIn(void);

/*****************************************************************************
** Code Start
*****************************************************************************/

/**
******************************************************************************
**
**  @brief  Main program
**
**  @param  argc - Argument count
**  @param  argv - Array of pointers to argument strings
**
**  @return 0
**
******************************************************************************
**/
int main(int argc, char *argv[])
{
    int     ix;
    char    *cFile = NULL;
    char    *dFile = NULL;
    char    *oFile = NULL;
    bool    valid  = true;
    bool    clogs  = false;
    bool    dlogs  = false;
    bool    reverse = false;

    if (argc <= 1)
    {
        help();
        return 0;
    }

    if (strcmp(argv[1], "help") == 0)
    {
        printf("Command: %s\n", argv[1]);
        help();
        return 0;
    }

    if (strcmp(argv[1], "menu") != 0 && strcmp(argv[1], "process") != 0)
    {
        printf("Invalid Command: %s\n", argv[1]);
        return 0;
    }        

    printf("Command: %s\n", argv[1]);

    for (ix = 2; ix < argc; ++ix)
    {
        if (strlen(argv[ix]) > 2)
        {
            if (strncmp(argv[ix], "-c", 2) == 0)
            {
                cFile = argv[ix] + 2;
                printf("Customer File: %s\n", cFile);
            }
            else if (strncmp(argv[ix], "-d", 2) == 0)
            {
                dFile = argv[ix] + 2;
                printf("Debug File: %s\n", dFile);
            }
            else if (strncmp(argv[ix], "-f", 2) == 0 &&
                      strcmp(argv[1], "process") == 0)
            {
                oFile = argv[ix] + 2;
                printf("Output File: %s\n", oFile);
            }
            else
            {
                printf("Invalid argument: %s\n", argv[ix]);
                valid = false;
            }
        }
        else if (strlen(argv[ix]) == 2)
        {
            if (strncmp(argv[ix], "-e", 2) == 0 &&
                 strcmp(argv[1], "process") == 0)
            {
                LogSimSetLogType(LONG_MESSAGE);
            }
            else if (strncmp(argv[ix], "-r", 2) == 0 &&
                      strcmp(argv[1], "process") == 0)
            {
                reverse = true;
            }
            else
            {
                printf("Invalid argument: %s\n", argv[ix]);
                valid = false;
            }
        }
        else
        {
            printf("Invalid argument: %s\n", argv[ix]);
            valid = false;
        }
    }

    if (valid && cFile && dFile)
    {
        if (LogSimLoadCDFile(cFile, dFile) == GOOD)
        {
            printf("Customer File: %s Load Successful\n", cFile);
            printf("Debug File: %s Load Successful\n", dFile);
            clogs = true;
            dlogs = true;
        }
        else
        {
            printf("Customer File: %s Load Not Successful and/or\n", cFile);
            printf("Debug File: %s Load Not Successful\n", dFile);
        }
    }
    else if (valid && cFile)
    {
        if (LogSimLoadFile(cFile) == GOOD)
        {
            printf("Customer File: %s Load Successful\n", cFile);
            clogs = true;
        }
        else
        {
            printf("Customer File: %s Load Not Successful\n", cFile);
        }
    }
    else if (valid && dFile)
    {
        if (LogSimLoadDebugFile(dFile) == GOOD)
        {
            printf("Debug File: %s Load Successful\n", dFile);
            dlogs = true;
        }
        else
        {
            printf("Debug File: %s Load Not Successful\n", dFile);
        }
    }

    if (!valid)
    {
        printf("LogSim exiting! Invalid Options!\n");
        return 1;
    }

    if (strcmp(argv[1], "process") == 0)
    {
        printf("Processing Logs to: %s\n", oFile ? oFile : "STDOUT");

        if (clogs && dlogs)
        {
            LogSimPrintAllLogs(oFile, NULL, reverse);
        }
        else if (clogs)
        {
            LogSimPrintCustomerLogs(oFile, NULL, reverse);
        }
        else if (dlogs)
        {
            LogSimPrintDebugLogs(oFile, NULL, reverse);
        }
        else
        {
            printf("Nothing to do (No logs successfully loaded!)\n");
        }
    }
    else
    {
        printf("Loading Main Menu\n");
        MainMenu();
    }

    return 0;
}


/**
******************************************************************************
**
**  @brief  Help text
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void help(void)
{
    printf("\n");
    printf("Usage: ");
    printf("LogSim.exe COMMAND [OPTIONS]...\n");
    printf("\n");
    printf("   COMMAND\n");
    printf("   menu        - Open the Menu.\n");
    printf("   process     - Process the logs to (-f\"file\") or STDOUT.\n");
    printf("   help        - Display this help menu.\n");
    printf("\n");
    printf("   OPTIONS (NOTE: No space between option and option value!)\n");
    printf("   -c\"file\"    - Binary Customer Log Dump.\n");
    printf("   -d\"file\"    - Binary Debug Log Dump.\n");
    printf("   -f\"file\"    - File to Output to (STDOUT if none).\n");
    printf("                 Note: Only used for COMMAND = \"process\".\n");
    printf("   -e          - Use Extended Message.\n");
    printf("   -r          - Reverse Order (New->Old) default(Old->New).\n");
    printf("\n");
}


/**
******************************************************************************
**
**  @brief  Main menu
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void MainMenu(void)
{
    char command;
    bool quit       = false;

    while (!quit)
    {
        /* Print the menu */

        printf("\n");
        printf("Welcome to the Log Simulator!\n");
        printf("\n");
        printf("Main Menu\n");
        printf("Select by Number.\n");
        printf("1. Test Logs\n");
        printf("2. Load Logs\n");
        printf("3. Log Information\n");
        printf("4. Quick Search\n");
        printf("5. Quick Search + Extended Data\n");
        printf("9. Quit\n");
        printf("\n");
        printf("Please Enter Selection\n");

        command = getchar();     /* Get the command */

        switch (command)        /* Execute the command */
        {
        case '9':
            quit = true;
            break;

        case '1':
            TestLogMenu();
            break;

        case '2':
            LoadLogMenu();
            break;

        case '3':
            LogInfo();
            break;

        case '4':
            QuickSearch();
            break;

        case '5':
            QuickSearchExtended();
            break;

        default:
            printf("Invalid command: %c\n", command);
            break;
        }
    }
}


/**
******************************************************************************
**
**  @brief  Test Log Menu
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void TestLogMenu(void)
{
    char    command;
    bool    quit        = false;

    while (!quit)
    {
        /* Print the menu */

        printf("\n");
        printf("Test Log Menu\n");
        printf("Select by Number.\n");
        printf("1. Test Short Logs\n");
        printf("2. Test Extended Logs\n");
        printf("9. Return to Main Menu\n");
        printf("\n");
        printf("Please Enter Selection\n");

        command = getchar();    /* Get the command */

        switch (command)        /* Execute the command */
        {
        case '9':
            quit = true;
            break;

        case '2':
#if 0
            LogSimSetLogType(LONG_MESSAGE);
#endif
        case '1':
#if 0
            printf("Enter the number of times to run the test: ");
            scanf("%d", &loopcount);

            while (i++ < loopcount)
            {
                printf("\n\n\n");
                printf("*****************************************************************\n");
                printf("    Loop %6d of %6d\n", i, loopcount);
                printf("*****************************************************************\n");
                printf("\n");
    
                LogTest();
            }
            
            i = 0;
            loopcount = 0;
            LogSimSetLogType(SHORT_MESSAGE);
            buffer[0] = '\0';
#endif
            printf("Logtest No longer supported!");
            break;

        default:
            printf("Invalid command: %c\n", command);
            break;
        }
    }
}


/**
******************************************************************************
**
**  @brief  Load Log Menu
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void LoadLogMenu(void)
{
    char    command;
    bool    quit    = false;
    int     retval  = 0;
    char    *pBuf;

    while (!quit)
    {
        pBuf = buffer;

        /* Print the menu */

        printf("\n");
        printf("Load Log Menu\n");
        printf("Select by Number.\n");
        printf("1. Load Customer Logs\n");
        printf("2. Load Debug Logs\n");
        printf("3. Print All Logs\n");
        printf("4. Print Debug Logs\n");
        printf("5. Print Customer Logs\n");
        printf("9. Return to Main Menu\n");
        printf("\n");
        printf("Please Enter Selection\n");

        command = getchar();    /* Get the command */

        switch (command)        /* Execute the command */
        {
        case '9':
            quit = true;
            break;

        case '1':
            printf("Enter the location of the customer logs to load: ");
            retval = scanf("%s", buffer);
            
            if (retval != 0 && retval != EOF)
            {
                retval = LogSimLoadFile(buffer);

                if (retval == GOOD)
                {
                    printf("Customer Log File %s loaded succesfully\n", buffer);
                }
                else
                {
                    printf("Could not open file: %s\n", buffer);
                }
            }
            else
            {
                printf("Invalid input!\n");
            }

            buffer[0] = '\0';
            break;

        case '2':
            printf("Enter the location of the debug logs to load: ");
            retval = scanf("%s", buffer);
            
            if (retval != 0 && retval != EOF)
            {
                retval = LogSimLoadDebugFile(buffer);

                if (retval == GOOD)
                {
                    printf("Debug Log File %s loaded succesfully\n", buffer);
                }
                else
                {
                    printf("Could not open file: %s\n", buffer);
                }
            }
            else
            {
                printf("Invalid input!\n");
            }

            buffer[0] = '\0';
            break;

        case '3':
        case '4':
        case '5':
            printf("Enter a file to write the logs (\"none\" for STDOUT): ");
            retval = scanf("%s", pBuf);

            if (retval == 0 || retval == EOF)
            {
                pBuf = NULL;
            }
            else if (strncmp(pBuf, "none", 4) == 0)
            {
                pBuf = NULL;
            }

            if (command == '3')
            {
                LogSimPrintAllLogs(pBuf, NULL, false);
            }
            else if (command == '4')
            {
                LogSimPrintDebugLogs(pBuf, NULL, false);
            }
            else
            {
                LogSimPrintCustomerLogs(pBuf, NULL, false);
            }
            buffer[0] = '\0';
            printf("\nFinished Writing logs\n");
            break;

        default:
            printf("Invalid command: %c\n", command);
            break;
        }
    }
}


/**
******************************************************************************
**
**  @brief  Get log information
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void LogInfo(void)
{
    UINT16  mode        = MODE_MASTER_SEQUENCE_LOGS;
    UINT32  count       = 10;
    UINT32  sequence    = 0;
    char    *file       = NULL;
    char    command;
    char    *pNeedle    = NULL;
    char    needle[512] = {0};

    printf("What do you want to display? ([C]ustomer, [D]ebug, [A]ll): ");
    command = readChar();

    if (command == 'C' || command == 'c')
    {
        mode &= ~MODE_MASTER_SEQUENCE_LOGS;
    }
    else if (command == 'D' || command == 'd')
    {
        mode &= ~MODE_MASTER_SEQUENCE_LOGS;
        mode |= MODE_DEBUG_LOGS;
    }

    printf("Display extended data? ([Y]|[N]): ");
    command = readChar();

    if (command == 'Y' || command == 'y')
    {
        LogSimSetLogType(LONG_MESSAGE);
    }

    printf("Write ouput to a file? ([Y]|[N]): ");
    command = readChar();

    if (command == 'Y' || command == 'y')
    {
        printf("Enter the file to write output: ");
        if (readString(buffer))
        {
            file = buffer;
        }
    }

    printf("Specify number of logs (default 10)? ([Y]|[N]): ");
    command = readChar();

    if (command == 'Y' || command == 'y')
    {
        printf("Enter number of logs to display: ");
        readUINT32(&count);
    }

    printf("Specify a sequence number? ([Y]|[N]): ");
    command = readChar();

    if (command == 'Y' || command == 'y')
    {
        printf("Enter starting sequence number: ");
        readUINT32(&sequence);
        sequence = (count + sequence) - 1;
        mode |= MODE_USE_SEQUENCE;
    }

    printf("Search for a string? ([Y]|[N]): ");
    command = readChar();

    if (command == 'Y' || command == 'y')
    {
        printf("Enter search string: ");
        if (readString(needle))
        {
            pNeedle = needle;
        }
    }

    LogSimPrintLogs(mode, sequence, count, file, pNeedle, false);

    printf("\n\n");

    buffer[0] = '\0';

    LogSimSetLogType(SHORT_MESSAGE);
}


/**
******************************************************************************
**
**  @brief  Quick Search the logs
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void QuickSearch(void)
{
    UINT16  mode        = MODE_MASTER_SEQUENCE_LOGS;
    UINT32  count       = 0xFFFF;
    UINT32  sequence    = 0;
    char    *pNeedle    = NULL;
    char    needle[512] = {0};

    printf ("Enter search string: ");
    if (readString(needle))
    {
        pNeedle = needle;
    }

    LogSimPrintLogs(mode, sequence, count, NULL, pNeedle, false);
    
    printf("\n\n");

    LogSimSetLogType(SHORT_MESSAGE);
}


/**
******************************************************************************
**
**  @brief  Quick Search the logs plus extended data
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void QuickSearchExtended(void)
{
    LogSimSetLogType(LONG_MESSAGE);
    QuickSearch();
    LogSimSetLogType(SHORT_MESSAGE);
}

static char readChar(void)
{
    char rc = getchar();

    clearIn();

    return rc;
}

static bool readString(char *buf)
{
    int  count = 0;
    char read;

    while ((read = getchar()) != '\n')
    {
        buf[count] = read;
        ++count;
    }

    buf[count] = '\0';

    if (count == 0)
    {
        return false;
    }

    return true;
}

static bool readUINT32(UINT32 *number)
{
    char num[11];
    int count       = 0;
    char read;
    bool rc         = true;

    *number = 0;

    while ((read = getchar()) != '\n')
    {
        if (read >= '0' && read <= '9')
        {
            num[count] = read;
            ++count;
        }
        else
        {
            rc = false;
            clearIn();
            break;
        }

        if (count >= 10)
        {
            clearIn();
            break;
        }
    }

    num[count] = '\0';

    if (rc)
    {
        *number = atoi(num);
    }

    if (count == 0)
    {
        rc = false;
    }

    return rc;
}


static void clearIn(void)
{
    while (getchar() != '\n')
        {/* No-op */}
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
