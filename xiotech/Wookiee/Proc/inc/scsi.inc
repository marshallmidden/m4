# $Id: scsi.inc 161041 2013-05-08 15:16:49Z marshall_midden $
#******************************************************************************
#
#  NAME: scsi.inc
#
#  DESCRIPTION:
#
#       To provide definitions for SCSI structures.
#
#  Copyright (c) 1996 - 2000 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Equates -----------------------------------------------------------------
#
#
# --- CDB codes
#
# --- Function codes
#
        .set    testunr,0x0             # TEST UNIT READY command
        .set    reqsense,0x03           # REQUEST SENSE command
        .set    inquiry,0x12            # INQUIRY command
        .set    modeslc,0x15            # MODE SELECT command
        .set    modesns,0x1A            # MODE SENSE command
        .set    readcap,0x25            # READ CAPACITY command
        .set    readext,0x28            # READ EXTENDED command
        .set    writext,0x2A            # WRITE EXTENDED command
        .set    writeverify,0x2E        # WRITE/VERIFY command
        .set    verimed,0x2F            # VERIFY MEDIA command
        .set    synccache,0x35          # SYNCHRONIZE CACHE command
        .set    repluns,0xA0            # REPORT LUNS command
#
# --- SCSI status codes
#
        .set    scnorm,0                # Normal status
        .set    scechk,0x2              # Check Condition
        .set    scncmg,0x4              # Condition Met/Good
        .set    scbusy,0x8              # Busy
        .set    scning,0x10             # Intermediate/Good
        .set    scnicm,0x14             # Intermediate/Condtion Met
        .set    scresc,0x18             # Reservation Conflict
        .set    sccmdt,0x22             # Command Terminated
        .set    scquef,0x28             # Queue Full
        .set    scacac,0x30             # ACA Active
#
# --- SCSI keys
#
        .set    skmask,       0x0F      # Mask for sense key byte 2 (sskey)

        .set    sknone,       0x00      # No sense
        .set    skrecovered,  0x01      # Recovered
        .set    sknotrdy,     0x02      # Not ready
        .set    skmedium,     0x03      # Medium
        .set    skhardware,   0x04      # Hardware
        .set    skillegal,    0x05      # Illegal
        .set    skunitatt,    0x06      # Unit attention
        .set    skprotect,    0x07      # Data protect
        .set    skblank,      0x08      # Blank check
        .set    skvendor,     0x09      # Vendor specific
        .set    skcopyab,     0x0A      # Copy abort
        .set    skabort,      0x0B      # Abort command
        .set    skobsolete,   0x0C      # Obsolete
        .set    skoverflow,   0x0D      # Volume overflow
        .set    skmiscompare, 0x0E      # Miscompare
        .set    skreserved,   0x0F      # Reserved
#
# --- SCSI sense definitions
#
        .set    sserr,0                 # Error code
        .set    sssegment,0x01          # Segment
        .set    sskey,0x02              # Key
        .set    sslba3,0x03             # LBA MSB
        .set    sslba2,0x04             #
        .set    sslba1,0x05             #
        .set    sslba0,0x06             # LBA LSB
        .set    ssaddlength,0x07        # Additional sense length
        .set    sscs3,0x08              # Command specific MSB
        .set    sscs2,0x09              #
        .set    sscs1,0x0A              #
        .set    sscs0,0x0B              # Command specific LSB
        .set    sscode,0x0C             # Additional sense code
        .set    ssqual,0x0D             # Additional sense qualifier
        .set    ssres,0x0E              # Reserved
        .set    sssks2,0x0F             # Sense key specific MSB
        .set    sssks1,0x10             #
        .set    sssks0,0x11             # Sense key specific LSB

        .set    sensesize,0x12          # size of SCSI sense data
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
