# $Id: pci.as 143007 2010-06-22 14:48:58Z m4 $
#******************************************************************************
#
#  NAME: pci.as
#
#  PURPOSE:
#
#       To provide configuration and other miscellaneous PCI services for the
#       secondary PCI bus on the 80960Rx.
#
#  FUNCTIONS:
#
#       PCI$scanbus     - Scan secondary bus for attached devices
#       PCI$getconfig   - Get PCI configuration data for a device
#                         on the secondary bus
#       PCI$setconfig   - Set PCI configuration data for a device
#                         on the secondary bus
#
#  Copyright (c) 1996 - 2008 Xiotech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Global references -------------------------------------------------------
#
        .globl  PCI$scanbus
        .globl  PCI$getconfig
        .globl  PCI$setconfig
#
# --- beginning of code -------------------------------------------------------
#
#******************************************************************************
#
#  NAME: PCI$scanbus
#
#  PURPOSE:
#
#       To provide a means of detecting devices on the secondary PCI bus.
#
#  DESCRIPTION:
#
#       This routine returns a bitmap signifying that a PCI device exists
#       at this device address.  Devices 12 through 32 (21 devices) are
#       scanned and the corresponding bit is set if a device is detected.
#       The position in a supplied table (normalized from 11-31 to 0-20) is
#       updated with the vendor/device ID retrieved from the PCI bus.  Data
#       from empty slots in the bus are undefined.
#
#  CALLING SEQUENCE:
#
#       call    PCI$scanbus
#
#  INPUT:
#
#       g0 = address of table to place vendor and device ID information.
#            Must be 21 entries each PCISTSIZE (see <pci.inc>) in size.
#
#  OUTPUT:
#
#       g0 = bitmap. 1 = device present.
#
#  REGS DESTROYED:
#
#       g0
#       g1
#       g2
#       g3
#       g4
#
#
#******************************************************************************
#
PCI$scanbus:
c       fprintf(stderr, "LI_ScanBus g0=%p, PCIMAXDEV=%d\n", (void *)g0, PCIMAXDEV);
c       g0 = LI_ScanBus((void *)g0, PCIMAXDEV);
c       fprintf(stderr, "LI_ScanBus bitmap=%08lX\n", g0);
        ret                             # done
#
#******************************************************************************
#
#  NAME: PCI$getconfig
#
#  PURPOSE:
#
#       To provide a means of reading PCI configuration data on the
#       secondary bus.
#
#  DESCRIPTION:
#
#       This routine is called with a device ID (11-31) and an offset
#       of configuration data to retrieve.
#       If no device is present FALSE is returned in register g4.
#       Otherwise the configuration data requested is returned in register g3.
#
#  CALLING SEQUENCE:
#
#       call    PCI$getconfig
#
#  INPUT:
#
#       g0 = PCI offset of header to read.
#       g1 = Device ID (11-31) to interrogate
#       g2 = Number of bytes to read.
#
#  OUTPUT:
#
#       g3 = Data as requested.
#       g4 = T/F; T = device present
#
#  REGS DESTROYED:
#
#       g3
#       g4
#
#
#******************************************************************************
#
PCI$getconfig:
#
# --- clear Secondary ATU Status Register
#
c       g4 = LI_GetConfig(g1, g0, (INT32)g2, (unsigned char *)&g3);
        ret
#
#******************************************************************************
#
#  NAME: PCI$setconfig
#
#  PURPOSE:
#
#       To provide a means of writing PCI configuration data on the
#       secondary bus.
#
#  DESCRIPTION:
#
#       This routine is called with a device ID (11-31) and an offset
#       of configuration data to retrieve.
#       If no device is present FALSE is returned in register g2.
#       Otherwise the configuration data in register g3 is written to the offset
#       indicated.
#
#  CALLING SEQUENCE:
#
#       call    PCI$setconfig
#
#  INPUT:
#
#       g0 = PCI offset of header to set.
#       g1 = Device ID (11-31) to set
#       g2 = number of bytes (1-4) to write.
#       g3 = Data to write to config header.
#
#
#  OUTPUT:
#
#       g4 = T/F; T = device present
#
#  REGS DESTROYED:
#
#       g4
#
#
#******************************************************************************
#
PCI$setconfig:
c       g4 = LI_SetConfig(g1, g0, (INT32)g2, (unsigned char *)&g3);
        ret
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
