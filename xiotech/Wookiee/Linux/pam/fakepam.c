/* $Header$ */
/**
******************************************************************************
**
**  @file       fakepam.c
**
**  Copyright (c) 2004-2008 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#include <stdio.h>
#include <stdlib.h>

#include "li_pci.h"
#include <sys/pci.h>
#ifndef PCI_VENDOR_ID_QLOGIC
  #define PCI_VENDOR_ID_QLOGIC          0x1077
#endif  /* PCI_VENDOR_ID_QLOGIC */

#include "XIO_Const.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/

/*
 * This define allows a routine to be declared as not returning.
 */
#define NORETURN __attribute__((noreturn))

#define PCIMAXDEV   21        /**< Number of devices to use for scanbus     */

/*
******************************************************************************
** Routines not externed in header files.
******************************************************************************
*/
extern void     SETUP_linux(void);
extern INT32    resetQlogic(INT32);

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static void     PAM_ResetQlogics(void);

/**
******************************************************************************
**
**  @brief      Main start of program
**
**  @param      argc    - number of args to prgram
**  @param      argv    - array of string arguments
**
**  @return     Does not return.
**
******************************************************************************
**/
NORETURN INT32 main(INT32 argc, char *argv[])
{
    PAM_ResetQlogics();
    sleep(2);
    exit(0);
}

/**
******************************************************************************
**
**  @brief      Reset the FE/BE Qlogic cards
**
**  @param      none
**
**  @return     zero if successful, non-zero if error occurred.
**
******************************************************************************
**/
static void PAM_ResetQlogics(void)
{
    unsigned long   bitmap = 0;
    long            index = 0;
    pcidevtbl       devtbl[PCIMAXDEV];

    /*
     * Search for Qlogic cards
     */
    bitmap = LI_ScanBus(devtbl, PCIMAXDEV);

    /*
     * If bitmap is 0, we did not find any devices.
     * This is an error as far as we are concerned.
     */
    if (!bitmap)
    {
        fprintf(stderr, "did not find bitmap for qlogic reset\n");
        return;
    }

    for (index = 0; index < XIO3D_NIO_MAX; ++index)
    {
        if ((bitmap & (1 << (PCIBASE + index))) &&
            ((devtbl[index + PCIOFFSET].vendev & 0x0000FFFF) == PCI_VENDOR_ID_QLOGIC))
        {
            if (resetQlogic(index) != GOOD)
            {
                fprintf(stderr, "Unable to reset Qlogic card (%ld) 0x%08X\n",
                           index, devtbl[index + PCIOFFSET].vendev);
            }
        }
    }

    fprintf(stderr, "QLogic cards reset\n");
}


/***
** Modelines
** vi:sw=4 ts=4 expandtab
**/
