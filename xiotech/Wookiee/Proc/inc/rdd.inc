# $Id: rdd.inc 152890 2011-01-20 21:01:48Z m4 $
#**********************************************************************
#
#  NAME: rdd.inc (RAID Device Description)
#
#  DESCRIPTION:
#       To provide a common means of defining the RAID Device Description
#       (RDD) structure.
#
#  Copyright (c) 1996-2010 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# ATTENTION:
#   If fields of these data structures are changed, it is probably necessary
#   to update the Direct Memory Copy (DMC) raid information via:
#       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
#
#
# --- Equates ---------------------------------------------------------
#
# --- rd_type definitions
#
        .set    rdstd,0                 # Standard device type
        .set    rdraid0,1               # RAID 0 device type - striped
        .set    rdraid1,2               # RAID 1 device type - mirrored
        .set    rdraid5,3               # RAID 5 device type - parity
        .set    rdraid10,4              # RAID 10 device type - mirror & stripe
        .set    rdlinkdev,5             # Linked RAID device type
        .set    rdslinkdev,6            # Snapshot linked device type
#
# --- rd_status definitions
#
        .set    rdnonx,0x00             # Non-existent
        .set    rdinop,0x01             # Inoperative
        .set    rduninit,0x02           # Uninitialized
                                        # 0x03 - (previously sched for init)
        .set    rdinit,0x04             # Initializing
        .set    rderror,0x05            # Error occurred
        .set    rdop,0x10               # Operational
        .set    rddegraded,0x11         # Degraded
#--     .set    rdrebuild,0x12          # Rebuild needed

#
# --- rd_astatus bit definitions
#
        .set    rdaparity,0             # 0x01 RAID 5 requires parity check
        .set    rdarebuild,1            # 0x02 Requires rebuild
        .set    rdauninit,2             # 0x04 Uninitialized
        .set    rdatermbg,3             # 0x08 Terminate all background processing
        .set    rdar5srip,4             # 0x10 RAID 5 Stripe Resync In Progress
        .set    rdarebuildwrites,5      # 0x20 Rebuild before writing
        .set    rdalocalimageip,6       # 0x40 Local Image Update is in progress
                                        # 0x00 No Writes allowed to the RAID while
                                        #      this bit is on.
#
# --- Parity/mirror checker bit definitions (R_pcctrl)
#
        .set    rdpcnewcmd,31           # 0x8000000 New command (change request)
#
        .set    rdpcdefault,7           # 0x0000080 Use default value
        .set    rdpcmarked,6            # 0x0000040 Scan all raids marked in rd_astatus
        .set    rdpccorrupt,5           # 0x0000020 Enable parity corruption - MUST
                                        #           enable CHKDEBUG compiler flag
        .set    rdpcspecific,4          # 0x0000010 Test a specific RID
#
        .set    rdpcclearlog,3          # 0x0000008 Clear log before start
        .set    rdpc1pass,2             # 0x0000004 Single pass (versus infinite)
        .set    rdpccorrect,1           # 0x0000002 Correct errors
        .set    rdpcenable,0            # 0x0000001 Enable parity checking
#
        .set    rdpcdefctl,0X8000004f   # Default control word at POR:
                                        #  enable, correct, 1 pass, clear logs,
                                        #  marked RDDs,
#
# --- Begin structure -------------------------------------------------
#                                                                  *****
        .set    rd_rid,0                # RAID ID                   <s>
        .set    rd_type,rd_rid+2        # RAID type                 <b>
        .set    rd_status,rd_type+1     # Device status             <b>
#
        .set    rd_depth,rd_status+1    # Mirror depth/stripe width <b>
                                        #  For RAID 1/10 it contains
                                        #  the mirror depth.
                                        #  For RAID 5 it contains the
                                        #  stripe width of 3, 5 or 9.
        .set    rd_pctrem,rd_depth+1    # Initialize % remaining    <b>
        .set    rd_psdcnt,rd_pctrem+1   # Count of PSD entries used <s>
                                        #  to define this device
#
        .set    rd_sps,rd_psdcnt+2      # Sectors/stripe            <w>
                                        #  (RAID 0, 5 and 10 only)
        .set    rd_spu,rd_sps+4         # Sectors per unit          <w>
                                        #  For RAID 10 it contains
                                        #  (rd_psdcnt*rd_sps)
                                        #  For RAID 5 it contains
                                        #  (rd_depth-1)*rd_sps
#                                                                  *****
        .set    rd_devcap,rd_spu+4      # Device capacity           <l>
        .set    rd_nvrdd,rd_devcap+8    # Next RAID in this Vdisk   <w>
#
        .set    rd_vid,rd_nvrdd+4       # Virtual ID of owner       <s>
        .set    rd_frcnt,rd_vid+2       # Failed/rebuild count      <s>
#                                                                  *****
        .set    rd_error,rd_frcnt+2     # Error count               <w>
        .set    rd_qd,rd_error+4        # Current queue depth       <w>
        .set    rd_rps,rd_qd+4          # Avg req/sec (last sec)    <w>
        .set    rd_avgsc,rd_rps+4       # Avg sector cnt (last sec) <w>
#                                                                  *****
        .set    rd_rreq,rd_avgsc+4      # Read request count        <l>
        .set    rd_wreq,rd_rreq+8       # Write request count       <l>
#                                                                  *****
        .set    rd_llsda,rd_wreq+8      # Locked LSDA (RAID 1, 10)  <l>
        .set    rd_lleda,rd_llsda+8     # Locked LEDA (RAID 1, 10)  <l>
#                                                                  *****
        .set    rd_iprocs,rd_lleda+8    # Init raid processes       <w>
        .set    rd_ierrors,rd_iprocs+4  # Init raid errors          <w>
        .set    rd_isectors,rd_ierrors+4# Init raid sectors         <l>
#                                                                  *****
        .set    rd_miscomp,rd_isectors+8# Parity errs in this RDD   <w>
        .set    rd_pardrv,rd_miscomp+4  # Par checks w/ missing drv <w>
        .set    rd_deflock,rd_pardrv+4  # Defrag lock count         <s>
        .set    rd_astatus,rd_deflock+2 # Additional status         <b>
        .set    rd_r5srout,rd_astatus+1 # R5 Stripe Resync Outs Cnt <b>
        .set    rd_notMirroringCSN,rd_r5srout+1 # Cntrl SN not Mirroring <w>
#                                                                  *****
        .set    rd_sprc,rd_notMirroringCSN+4 # Sample period req count   <w>
        .set    rd_spsc,rd_sprc+4       # Sample period sec count   <w>
        .set    rd_rpnhead,rd_spsc+4    # RPN thread head (RAID 5)  <w>
        .set    rd_vlop,rd_rpnhead+4    # Assoc. VLOP address       <w>
#                                                                  *****
        .set    rd_psd,rd_vlop+4        # PSD array                 <w>
#
# --- End structure ---------------------------------------------------
#
        .set    rddsiz,rd_psd           # Size of base RDD, add 4
                                        #  bytes for each associated
                                        #  PSD
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
