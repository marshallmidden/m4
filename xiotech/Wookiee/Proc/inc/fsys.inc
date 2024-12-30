# $Id: fsys.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: fsys.inc (File System Definition)
#
#  DESCRIPTION:
#
#       To provide a common means of defining the file system for
#       Thunderbolt.  This is used for keeping internal information
#       on the disk drives which are part of the system.
#
#  Copyright (c) 2000 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
# File IDs
#
#    The file IDs that the BE processor needs to recognize are limited
#    to the file ID of the directory itself and the files that it uses.
#    This is a complete list for completeness and will probably be
#    replicated in the CCB code.
#
        .set    fiddir,        0x00     # Directory FID
        .set    fidlabel,      0x01     # Label sector
        .set    fidbenvram,    0x02     # Back end NVRAM
        .set    fidfenvram,    0x03     # Front end NVRAM
        .set    fidemcnvram,   0x04     # CCB NVRAM
        .set    fidscratch,    0x05     # Scratch area for POST
        .set    fidcopy,       0x0B     # COPY NVRAM
#


        .set    fiddirsiz,     0x09                        # Directory file size
        .set    fiddirstrt,    0x00                        # Directory file start
#
        .set    fidlabelsiz,   LABELSIZE+1                 # Label file size
        .set    fidlabelstrt,  fiddirsiz+fiddirstrt        # Starting LBA
#
        .set    fidbenvramsiz, NVRP2SSIZ+1                 # BE NVRAM size
        .set    fidbenvramstrt,fidlabelsiz+fidlabelstrt    # BE NVRAM start
#
        .set    fidfenvramsiz, NVRP2SSIZ+1                 # FE NVRAM size
        .set    fidfenvramstrt,fidbenvramsiz+fidbenvramstrt# BE NVRAM start
#
        .set    fidemcnvramsiz,NVRP2SSIZ+1                 # CCB NVRAM size
        .set    fidemcnvramstrt,fidfenvramsiz+fidfenvramstrt#CCB NVRAM start
#
        .set    fidscratchsiz, SCRATCHSIZE+1               # Scratch size (256 sectors)
        .set    fidscratchstrt,fidemcnvramsiz+fidemcnvramstrt# Scratch start
#
# --- System wide parameters for reserved area.  NOTE THAT THE FILE
# --- SYSTEM UPDATE WILL BE TRUNCATED TO ROUGHLY WHERE THE LAST FILE
# --- LIES.  IF THERE ARE ADDITIONAL FILES USED, THIS MUST BE ADJUSTED!
#
        .set    SYSSRESERVE,128*DSKSALLOC# Max size of the reserved area
        .set    FSUPDATELEN,SYSSRESERVE/2# Max size for file system update
        .set    LABELSIZE,1             # Size of device label in sectors
        .set    SCRATCHSIZE,256         # Size of scratch area
#
# Magic numbers and constants for the file system
#
        .set    fsdnamelen,10           # Directory name length
#
# --- Begin directory entry structure ---------------------------------
#                                                                  *****
        .set    fsd_name,0              # File name (ASCII)fsdnamelen<b>
        .set    fsd_count,fsd_name+fsdnamelen # LBA count           <s>
        .set    fsd_offset,fsd_count+2  # LBA offset                <w>
#                                                                  *****
# --- End structure ---------------------------------------------------
#
        .set    fsdsiz,fsd_offset+4     # Size of Directory entry



#
# --- Begin file header structure -------------------------------------
#                                                                  *****
        .set    fsh_name,0              # File name (ASCII)       10<b>
#
#       reserved 6
#                                                                  *****
        .set    fsh_fid,fsh_name+16     # File ID                   <w>
        .set    fsh_len,fsh_fid+4       # File length               <w>
#
#       reserved 4
#
        .set    fsh_dcrc,fsh_len+8      # Data CRC                  <w>
#                                                                  *****
        .set    fsh_tstamp,fsh_dcrc+4   # Time stamp                <l>
#
#       reserved 468
#
        .set    fsh_hcrc,fsh_tstamp+476 # Header CRC                <w>
#                                                                  *****
# --- End structure ---------------------------------------------------
#
        .set    fshsiz,fsh_hcrc+4       # Size of header
        .set    fshcrcsiz,fsh_hcrc      # Size of CRC check
#
# --- Begin Multi-write record structure ------------------------------
#                                                                  *****
        .set    fsm_active,0            # Active write counter      <w>
        .set    fsm_good,fsm_active+4   # Good write counter        <w>
        .set    fsm_error,fsm_good+4    # Write error counter       <w>
        .set    fsm_goodmap,fsm_error+4 # Address of good write map <w>
#                                                                  *****
# --- End structure ---------------------------------------------------
#
        .set    fsmsiz,fsm_goodmap+4    # Size of record
#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
