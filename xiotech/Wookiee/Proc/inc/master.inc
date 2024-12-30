# $Id: master.inc 145079 2010-08-04 19:09:12Z mdr $
#**********************************************************************
#
#  NAME: master.inc
#
#  PURPOSE:
#
#       To provide a means of obtaining all of the include files that are
#       used to build the system.
#
#  Copyright (c) 1997-2009 Xiotech Corporation. All rights reserved.
#
#**********************************************************************
#
# --- include files ---------------------------------------------------
#
        .include "globalOptions.inc"  # VPRI Code
        .include "system.inc"    # SYSTEM definitions
        .include "options.inc"   # Options
#
        .include "cev.inc"       # Create / expand virtual device definitions
        .include "chn.inc"       # Channel definitions
        .include "ci.inc"        # Cache Information definitions
        .include "cimt.inc"      # Cache Information definitions
        .include "cor.inc"       # Copy registration definitions
        .include "daml.inc"      # Disk Allocation Map Link
        .include "datagram.inc"  # Datagram services definitions
        .include "def.inc"       # DEFine definitions
        .include "dev.inc"       # DEVice definitions
        .include "ddr.inc"       # Debug Data Retrieval definitions
        .include "dlmio.inc"     # Data-link manager I/O definitions
        .include "dma.inc"       # DMA definitions
        .include "drp.inc"       # Datagram Request Packet definitions
        .include "dspif.inc"     # Datagram services provider interface
        .include "dtmt.inc"      # Data-link manager target management tables
        .include "ecodes.inc"    # Error CODE definitions
        .include "err.inc"       # Error Trap definitions
        .include "fh.inc"        # Firmware header
        .include "ficb.inc"      # F/W Initialization Ctl Block definitions
        .include "fls.inc"       # FCAL Link Status
        .include "fr.inc"        # Flight recorder
        .include "ftdd.inc"      # Foreign Target Device Database definitions
        .include "ftdt.inc"      # Foreign Target descriptor table
        .include "ftospb.inc"    # Foreign Target Open session parameter block
        .include "icb.inc"       # QLogic ICB definitions
        .include "OS_II.inc"     # Internal Information definitions
        .include "ilt.inc"       # InterLayer Transport definitions
        .include "iltdefs.inc"   # InterLayer Transport sub-definitions
        .include "imt.inc"       # IMT definitions
        .include "iocb.inc"      # QLogic IOCB definitions
        .include "irp.inc"       # Initiator request packet definitions
        .include "isp2100.inc"   # QLogic ISP2x00 definitions
        .include "ldd.inc"       # Linked Device Data definitions
        .include "lldmt.inc"     # Link-level driver management table
        .include "LL_LinuxLinkLayer.inc"
        .include "LL_Stats.inc"  # link960 interface stats
        .include "loop.inc"      # Loop constants
        .include "lvm.inc"       # LUN to VDisk mapping definitions
        .include "magdd.inc"     # MAGNITUDE Link Device Database definitions
        .include "magdt.inc"     # MAGNITUDE descriptor table definitions
        .include "memory.inc"    # Memory Management definitions
        .include "mlmt.inc"      # MAGNITUDE Link Management Table definitions
        .include "mlospb.inc"    # MAGNITUDE Link Open session parameter block
        .include "nmi.inc"       # NMI Stats NVRAM definitions
        .include "nva.inc"       # Non-Volatile Activity definitions
        .include "nvac.inc"      # Non-Volatile Activity Ctl definitions
        .include "nvd.inc"       # Non-Volatile Diagnostic definitions
        .include "pcb.inc"       # Process Control Block definitions
        .include "pci.inc"       # PCI definitions
        .include "pdd.inc"       # Physical Device Description definitions
        .include "pdx.inc"       # Physical Device Table definitions
        .include "portdb.inc"    # QLogic Port Database defs
        .include "prp.inc"       # Physical Request Packet definitions
        .include "psd.inc"       # Physical Segment Description definitions
        .include "qcb.inc"       # Queue Control Block definitions
        .include "qcs.inc"       # Queue Control definitions
        .include "qrp.inc"       # QLogic Request Packet
        .include "qu.inc"        # Queue definitions
        .include "r5s.inc"       # Raid 5 Striping definitions
        .include "rbr.inc"       # Rebuild Record definitions
        .include "rdd.inc"       # RAID Device Description definitions
        .include "rdx.inc"       # RAID Device Table definitions
        .include "rpn.inc"       # RAID Parity Node definitions
        .include "rrb.inc"       # RAID Request Block definitions
        .include "rrp.inc"       # RAID Request Packet definitions
        .include "scsi.inc"      # SCSI definitions
        .include "sdd.inc"       # Server Device Description Definitions
        .include "sdx.inc"       # Server Device Table Definitions
        .include "sgl.inc"       # Scatter/Gather List definitions
        .include "sos.inc"       # Segment optimization structures definitions
        .include "srp.inc"       # SRP definitions
        .include "ssms.inc"      # Snapshot information
        .include "target.inc"    # Target descriptions
        .include "tpmt.inc"      # Target Path Management Table definitions
        .include "vcd.inc"       # Virtual Cache Description definitions
        .include "vdd.inc"       # Virtual Device Description definitions
        .include "vdx.inc"       # Virtual Device Index definitions
        .include "vlar.inc"      # VLink Association Record definitions
        .include "vldefs.inc"    # Virtual Link definitions
        .include "vlop.inc"      # VLink operation management table
        .include "vqd.inc"       # Virtual inQuiry Data definitions
        .include "vrp.inc"       # Virtual Request Packet definitions
        .include "xdl.inc"       # Xiotech Device Label definitions
        .include "xl.inc"        # Translation Layer definitions
#
        .include "iram.inc"      # IRAM definitions

.ifdef BACKEND
        .include "ccsm.inc"
        .include "cm.inc"
        .include "copymap.inc"
        .include "dcd.inc"
        .include "fsys.inc"      # File System definitions
        .include "nvabe.inc"     # NVA BE common definitions
        .include "p6.inc"
        .include "pcp.inc"
        .include "qpt.inc"
        .include "rcc.inc"
        .include "scd.inc"
        .include "scr.inc"
        .include "WC_WRP.inc"    # Get the WRP definitions
.endif

#******************************************************************************
#
####
## Modelines:
## Local Variables:
## tab-width: 4
## indent-tabs-mode: nil
## End:
## vi:sw=4 ts=4 expandtab
