/* $Id: li_pci.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**  @file       li_pci.h
**
**  @brief      Definitions for Linux Interface for PCI operations
**
**  Definitions for Linux interface code for PCI operations.
**
**  Copyright 2004-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef __LI_PCI_H__
#define __LI_PCI_H__
#include <options.h>
#include <XIO_Types.h>

/*
******************************************************************************
**  Public definitions
******************************************************************************
*/

#define PCI_COMMAND_IO          0x1     /* Enable I/O space */
#define PCI_COMMAND_MEMORY      0x2     /* Enable memory space */
#define PCI_COMMAND_MASTER      0x4     /* Enable bus mastering */
#define PCI_COMMAND_SPECIAL     0x8     /* Enable special cycles */
#define PCI_COMMAND_INVALIDATE  0x10    /* Enable memory write and invalidate */
#define PCI_COMMAND_PARITY      0x40    /* Enable parity checking */
#define PCI_COMMAND_WAIT        0x80    /* Enable address/data stepping */
#define PCI_COMMAND_SERR        0x100   /* Enable SERR */
#define PCI_COMMAND_FAST_BACK   0x200   /* Enable back-to-back writes */
#define PCI_COMMAND_INTX_DISABLE    0x400   /* INTx emulation disable */

#define XIO3D_NIO_MAX   MAX_HABS  /* Maximum number of I/O devices */

#define PCIOFFSET   5
#define PCIBASE     (11 + PCIOFFSET)

#define ENET_NAME_SIZE  8
#define ENET_BUSDEVFN   0xFFFFFFFF

/*
******************************************************************************
**  Public structure definitions
******************************************************************************
*/

typedef struct pcidevtbl
{
    UINT32 vendev;     /* Vendor/device */
    UINT32 mbar;       /* Memory BAR */
    UINT32 rambase;    /* RAM base address */
} pcidevtbl;

typedef struct pci_devs
{
    unsigned long busdevfn;
    UINT16 vendor;
    UINT16 device;
    char   enetname[ENET_NAME_SIZE];
    unsigned long start[7];
    unsigned long len[7];
    unsigned long flags[7];
    INT32  cfd;        /* Config file descriptor */
} pci_devs;

/*
******************************************************************************
**  Public function prototypes
******************************************************************************
*/

extern UINT32          LI_ScanBus(pcidevtbl tbl[], UINT32 tblsize);
extern INT32           LI_GetConfig(unsigned long id, unsigned long off, INT32 nbytes, UINT8 *data);
extern INT32           LI_SetConfig(unsigned long id, UINT32 off, INT32 nbytes, UINT8 *data);
extern unsigned long   LI_AccessDevice(unsigned long devno, unsigned long space);
extern void            LI_IRQCGlue(UINT32 id);
extern unsigned long   LI_GetDMAOffset(void);
extern unsigned long   LI_GetPhysicalAddr(unsigned long value);
extern UINT32          LI_GetSharedAddr(UINT32 dmaAddr);
extern unsigned long   LI_GetDeviceID(unsigned int port);
extern pci_devs        *LI_GetPCIdev(unsigned int port);
extern INT32           get_device_info(void);

#endif  /* __LI_PCI_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
