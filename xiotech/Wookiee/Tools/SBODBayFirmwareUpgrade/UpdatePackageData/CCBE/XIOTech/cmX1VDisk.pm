# $Header$
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2002  Xiotech
# ======================================================================
# $RCSfile$
# Author: Randy Rysavy, Mark Schibilla
#
# Purpose:
#   Wrapper for all the different XIOTech X1 VDisk commands
#   that can be sent to the XIOtech SAN system
##############################################################################
package XIOTech::cmdMgr;

use XIOTech::xiotechPackets;
use XIOTech::cmPDisk;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;

use XIOTech::error;

use XIOTech::logMgr;

use strict;


# X1_CONFIG_VDISK
use constant X1_CONFIG_VDISK_T1 =>
           "C          # operation
            S          # id1
            S          # id2
            C          # parm1
            C          # parm2
            C          # crypt1
            C          # crypt2
            L          # crypt3
            a16";      # sName

use constant X1_CONFIG_VDISK_T2 =>
           "C          # operation
            S          # id1
            L          # size
            C          # crypt1
            C          # crypt2
            L          # crypt3
            a16";      # sName

use constant X1_CONFIG_VDISK_T3 =>
           "C          # operation
            S          # id1
            C          # id2
            C          # id3
            C          # parm1
            C          # parm2
            C          # crypt1
            C          # crypt2
            L          # crypt3
            a16";      # sName

use constant X1_CONFIG_VDISK_T4 =>
           "C          # operation
            C          # id1
            C          # id2
            C          # id3
            C          # id4
            C          # parm1
            C          # parm2
            C          # crypt1
            C          # crypt2
            L          # crypt3
            a16";      # sName

use constant X1_CONFIG_VDISK_T5 =>
           "C          # operation
            C          # num hi
            C          # num med
            L          # rsvd
            C          # crypt1
            C          # crypt2
            L          # crypt3
            S          # rsvd
            S          # list begin
            S";        # list


use constant X1_CONFIG_VDISK_ENCRYPT =>
           "C          # p0
            C          # p1
            C          # p2
            L          # p3
            C          # crypt1
            C          # crypt2
            L          # crypt3
            a*";       # theRest

use constant X1_CONFIG_VDISK_LOG =>
           "C          # operation
            L          # eventNumber
            S          # rsvd
            C          # crypt1
            C          # crypt2
            L          # crypt3
            a16";      # sName


# X1_VDISK_MAP
use constant X1_VDISK_MAP                       => "a128";  # vdiskBitmap

# Cache mode constants
use constant CACHE_MODE_VDISK_OFF               => 0x00;    # VID cache disable    
use constant CACHE_MODE_VDISK_ON                => 0x01;    # VID cache enable     
use constant CACHE_MODE_GLOBAL_OFF              => 0x80;    # Global cache disable 
use constant CACHE_MODE_GLOBAL_ON               => 0x81;    # Global cache enable  

# Scrubbing constants
use constant SCRUB_OFF                          => 0x00;        
use constant SCRUB_ON                           => 0x01;        


##############################################################################
# Name:     X1CfgVDiskPDiskLabel
#
# Desc:     VDisk Config - PDisk Label 
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskPDiskLabel
{
    my ($self,
        $id,
        $labtype) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFF],
                ["X1CfgVDiskPDiskLabel"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T2, 

              CFG_VDISK_PDISK_LABEL,    # operation
              $id,                      # id1
              $labtype,                 # size
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName
    
    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);
    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 PDisk Label successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 PDisk Label failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1VDiskGetMap
#
# Desc:     Get the VDisk Map
#           Note: This is NOT a configuration command so it does not 
#           use encryption.
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1VDiskGetMap
{
    my ($self) = @_;

    logMsg("begin\n");


    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_VMAP, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_VMAP,
            \&_handleResponseX1GetVMap);

    if (%rsp)
    {
        logMsg("X1VDiskGetMap successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1VDiskGetMap failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1VDiskGetCopyMap
#
# Desc:     Get the VDisk Copy Map
#           Note: This is NOT a configuration command so it does not 
#           use encryption.
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1VDiskGetCopyMap
{
    my ($self) = @_;

    logMsg("begin\n");


    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_COPY_STATUS, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_COPY_STATUS,
            \&_handleResponseX1GetVDiskCopyMap);

    if (%rsp)
    {
        logMsg("X1VDiskGetCopyMap successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1VDiskGetCopyMap failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     X1VDiskGetMirrorMap
#
# Desc:     Get the VDisk Mirror Map
#           Note: This is NOT a configuration command so it does not 
#           use encryption.
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1VDiskGetMirrorMap
{
    my ($self) = @_;

    logMsg("begin\n");


    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_MIRROR_STATUS, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_MIRROR_STATUS,
            \&_handleResponseX1GetVDiskMirrorMap);

    if (%rsp)
    {
        logMsg("X1VDiskGetMirrorMap successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1VDiskGetMirrorMap failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1VDiskGetMirrorIOStatusMap
#
# Desc:     Get the VDisk Mirror IO Status Map
#           Note: This is NOT a configuration command so it does not 
#           use encryption.
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1VDiskGetMirrorIOStatusMap
{
    my ($self) = @_;

    logMsg("begin\n");


    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_MIRROR_IO_STATUS, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_MIRROR_IO_STATUS,
            \&_handleResponseX1GetVDiskMirrorIOStatusMap);

    if (%rsp)
    {
        logMsg("X1VDiskGetMirrorIOStatusMap successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1VDiskGetMirrorIOStatusMap failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetFreeSpace
#
# Desc:     Get the free space for various RAID types
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GetFreeSpace
{
    my ($self) = @_;

    logMsg("begin\n");


    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_FREE, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_FREE,
            \&_handleResponseX1GetFreeSpace);

    if (%rsp)
    {
        logMsg("X1GetFreeSpace successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetFreeSpace failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     DisplayX1VDiskMap
#
# Desc:     Display an X1 VDisk map
#
# Input:    hash containing X1 account data
#
##############################################################################
sub DisplayX1VDiskMap
{
    my (%info) = @_;

    my @vDisks = ParseBitmap($info{VDISKBITMAP});

    print "VDisk Bitmap: @vDisks\n";
}

##############################################################################
# Name:     DisplayX1VDiskCopyMap
#
# Desc:     Display an X1 VDisk Copy map
#
# Input:    
#
##############################################################################
sub DisplayX1VDiskCopyMap
{
    my (%info) = @_;

    my @vDisks = ParseBitmap($info{VDISKCOPYBITMAP});

    print "VDisk Copy Bitmap: @vDisks\n";
}


##############################################################################
# Name:     DisplayX1VDiskMirrorMap
#
# Desc:     Display an X1 VDisk Mirror map
#
# Input:    
#
##############################################################################
sub DisplayX1VDiskMirrorMap
{
    my (%info) = @_;

    my @vDisks = ParseBitmap($info{VDISKMIRRORBITMAP});

    print "VDisk Mirror Bitmap: @vDisks\n";
}

##############################################################################
# Name:     DisplayX1VDiskMirrorIOStatusMap
#
# Desc:     Display an X1 VDisk Mirror IO Status map
#
# Input:    
#
##############################################################################
sub DisplayX1VDiskMirrorIOStatusMap
{
    my (%info) = @_;

    my @vDisks = ParseBitmap($info{VDISKMIRRORIOSTATUSBITMAP});

    print "VDisk Mirror IO Status Bitmap: @vDisks\n";
}


##############################################################################
# Name:     DisplayX1FreeSpace
#
# Desc:     Display X1 Free Space
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1FreeSpace
{
    my (%info) = @_;

    print "Free space:\n";
    print "  RAID 0 :   $info{RAID0}\n";
    print "  RAID 5 :   $info{RAID5}\n";
    print "  RAID 10:   $info{RAID10}\n";

}

##############################################################################
# Name:     DisplayX1PDiskInfo
#
# Desc:     Display an X1 PDisk info
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1PDiskInfo
{
    my ($self, %info) = @_;

    print "X1 PDisk Info:\n";
    printf "  PDisk ID:                 %u\n", $info{PDISKID};
    printf "  Class:                    0x%02X\n", $info{CLASS};
    print  "  Name:                     $info{NAME}\n";
    printf "  System serial number:     %hu\n", $info{SYSTEMSN};
    printf "  Misc status:              0x%02X\n", $info{MISCSTAT};
    printf "  Post status:              0x%02X\n", $info{POSTSTAT};
    printf "  Device status:            0x%02X\n", $info{DEVSTAT};
    printf "  Percent left in rebuild:  %hu\n", $info{PERCENTLEFT};
    print  "  Device capacity:          $info{DEVCAP}\n";
    print  "  Product ID:               $info{PRODUCTID}\n";
    print  "  Vendor ID:                $info{VENDORID}\n";
    print  "  Firmware revision:        $info{FWREVISION}\n";
    print  "  PDisk serial number:      $info{PDISKSN}\n";
    print  "  Total available space:    $info{TAS}\n";
    print  "  Largest available space:  $info{LAS}\n";
    printf "  Enclosure:                %u\n", $info{ENCLNUM};
    printf "  Slot:                     %u\n", $info{SLOTNUM};
    printf "  Path count:               %u\n", $info{PATHCOUNT};
    printf "  Paths:                    0x%02X  0x%02X  0x%02X  0x%02X\n", $info{PATH0}, $info{PATH1}, $info{PATH2}, $info{PATH3};
    printf "  WWN:                      %8.8X%8.8X\n", $info{WWN_LO}, $info{WWN_HI};
    printf "  Fibre ID:                 0x%08X\n", $info{FIBREID};
    printf "  Device Type:              0x%X - %s\n", $info{DEVTYPE}, _getString_PDDT($info{DEVTYPE});

    print "\n";
}

##############################################################################
# Name:     DisplayX1BayInfo
#
# Desc:     Display an X1 Bay info
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1BayInfo
{
    my ($self, %info) = @_;

    print "X1 Disk Bay Info:\n";
    printf "  Bay ID:                   %u\n", $info{BAYID};
    printf "  Device status:            0x%02X\n", $info{DEVSTAT};
    print  "  Product ID:               $info{PRODUCTID}\n";
    print  "  Vendor ID:                $info{VENDORID}\n";
    print  "  Firmware revision:        $info{FWREVISION}\n";
    print  "  Bay serial number:        $info{BAYSN}\n";
    printf "  WWN:                      %8.8X%8.8X\n", $info{WWN_LO}, $info{WWN_HI};
    printf "  Loop status:              0x%02X\n", $info{LOOPSTATUS};
    printf "  IO Module 1 status:       0x%02X\n", $info{IOMODSTATUS1};
    printf "  IO Module 2 status:       0x%02X\n", $info{IOMODSTATUS2};
    printf "  Path count:               %u\n", $info{PATHCOUNT};
    printf "  Paths:                    0x%02X  0x%02X  0x%02X  0x%02X\n", $info{PATH0}, $info{PATH1}, $info{PATH2}, $info{PATH3};
    printf "  Shelf ID:                 %d\n", $info{SHELFID};
    printf "  Device Type:              0x%X - %s\n", $info{DEVTYPE}, _getString_PDDT($info{DEVTYPE});

    print "\n";

}

##############################################################################
# Name:     DisplayX1VDiskInfo
#
# Desc:     Display an X1 VDisk info
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1VDiskInfo
{
    my ($self, %info) = @_;

    my @raids = ParseBitmap($info{RAIDMAP});

    print "X1 VDisk Info:\n";
    printf "  VDisk ID:                 %u\n", $info{VDISKID};
    printf "  Device status:            0x%02X\n", $info{DEVSTAT};
    print  "  Device capacity:          $info{DEVCAP}\n";
    print  "  RAID Bitmap:              @raids\n";
    printf "  Attribute 1:              0x%02X\n", $info{ATTRIB1};
    print  "  Name:                     $info{NAME}\n";
    printf "  Attribute 2:              0x%02X\n", $info{ATTRIB2};
    printf "  Device Type:              0x%X - %s\n", $info{DEVTYPE}, _getString_PDDT($info{DEVTYPE});

    print "\n";
}

##############################################################################
# Name:     DisplayX1VDiskCopyInfo
#
# Desc:     Display an X1 VDisk Copy info
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1VDiskCopyInfo
{
    my ($self, %info) = @_;


    print "X1 VDisk Copy Info:\n";
    printf "  Destination VDisk ID:     %u\n", $info{DESTVID};
    printf "  Source VDisk ID:          %u\n", $info{SRCVID};
    printf "  Copy percent complete:    %u\n", $info{PERCENTCMPL};
    printf "  Device status:            0x%02X\n", $info{DEVSTAT};

    print "\n";
}


##############################################################################
# Name:     DisplayX1RaidInfo
#
# Desc:     Display an X1 Raid info
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1RaidInfo
{
    my ($self, %info) = @_;

    my @pdiskexist = ParseBitmap($info{EXISTMAP});
    my @pdiskfail = ParseBitmap($info{FAILMAP});
    my @pdiskrebuild = ParseBitmap($info{REBUILDMAP});

    print "X1 RAID Info:\n";
    printf "  RAID ID:                  %u\n", $info{RAIDID};
    printf "  Owning VDisk ID:          %u\n", $info{VDISKID};
    printf "  Class:                    0x%02X\n", $info{CLASS};
    printf "  Change count:             %u\n", $info{CHGCNT};
    printf "  Device status:            0x%02X\n", $info{DEVSTAT};
    printf "  Additional status:        0x%02X\n", $info{STATUS};
    printf "  Mirror depth:             %u\n", $info{MIRRORDEPTH};
    printf "  Stripe width:             %u\n", $info{STRIPEWIDTH};
    print  "  Device capacity:          $info{DEVCAP}\n";
    printf "  Sectors per stripe:       %u\n", $info{SECTORSPERSTRIPE};
    print  "  PDisk exist bitmap:       @pdiskexist\n";
    print  "  PDisk fail bitmap:        @pdiskfail\n";
    print  "  PDisk rebuild bitmap:     @pdiskrebuild\n";
    printf "  Device Type:              0x%X - %s\n", $info{DEVTYPE}, _getString_PDDT($info{DEVTYPE});

    print "\n";

}

##############################################################################
# Name:     DisplayX1PDiskStats
#
# Desc:     Display an X1 PDisk stats
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1PDiskStats
{
    my ($self, %info) = @_;

    print "X1 PDisk Stats:\n";
    printf "  PDisk ID:                         %u\n", $info{PDISKID};
    printf "  Queue depth:                      %u\n", $info{QDEPTH};
    printf "  Average requests per second:      %u\n", $info{AVGRPS};
    printf "  Average sector count per request: %u\n", $info{AVGSCPR};
    print  "  Write request count:              $info{WRTREQCNT}\n";
    print  "  Read request count:               $info{RDREQCNT}\n";
    printf "  Correctable errors:               %u\n", $info{CORRERRS};
    printf "  Enclosure:                        %u\n", $info{ENCLNUM};
    printf "  Slot:                             %u\n", $info{SLOTNUM};

    print "\n";
}

##############################################################################
# Name:     DisplayX1PDiskPState
#
# Desc:     Display an X1 PDisk PState
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1PDiskPState
{
    my ($self, %info) = @_;

    my $i;
    my $j;

    print "X1 PDisk PState:\n";
    printf "  Scrubbing State:                  0x%x\n", $info{SCRUB_STATE};
    printf "  Copy Rebuild Priority:            0x%x\n", $info{COPY_REBUILD_PRI};
    printf "  SCSI Bitmap:                      0x%x\n", $info{SCSI_BITMAP};
    printf "  Version:                          0x%x\n", $info{VERSION};
    printf "  Revision:                         0x%x\n", $info{REVISION};
    printf "  Heartbeat:                        0x%x\n", $info{HEARTBEAT};
    print  "  Drive States:\n";

    for ($i = 0; $i < 32; $i++)
    {
        printf "    Drives (%3lu-%3lu):  ", $i*8, ($i*8) + 7;
        for ($j = 0; $j < 8; $j++)
        {
            printf "0x%x  ", $info{STATES}[($i*8) + $j];
        }

        print "\n";
    }

    print "\n";
}

##############################################################################
# Name:     DisplayX1PDiskDefragStatus
#
# Desc:     Display an X1 PDisk Defrag Status
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1PDiskDefragStatus
{
    my ($self, %info) = @_;

    my $msg = "";

    $msg .= sprintf "Physical Disk Defrag Status:\n";
    $msg .= sprintf "  PID:             ";
    
    if ($info{PDISKID} == 0xFFFF)
    {
        $msg .= sprintf "Defrag ALL PDisks\n";        
    }
    else
    {
        $msg .= sprintf "%u\n", $info{PDISKID};    
    }
    
    $msg .= sprintf "  FLAGS:           0x%X\n", $info{FLAGS};
    $msg .= sprintf "\n";

    return $msg;
}

##############################################################################
# Name:     DisplayX1VDiskStats
#
# Desc:     Display an X1 PDisk stats
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1VDiskStats
{
    my ($self, %info) = @_;

    print "X1 VDisk Stats:\n";
    printf "  VDisk ID:                         %u\n", $info{VDISKID};
    printf "  Queue depth:                      %u\n", $info{QDEPTH};
    printf "  Average requests per second:      %u\n", $info{AVGRPS};
    printf "  Average sector count per request: %u\n", $info{AVGSCPR};
    print  "  Write request count:              $info{WRTREQCNT}\n";
    print  "  Read request count:               $info{RDREQCNT}\n";

    print "\n";
}

##############################################################################
# Name:     DisplayX1RaidStats
#
# Desc:     Display an X1 PDisk stats
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1RaidStats
{
    my ($self, %info) = @_;

    print "X1 RAID Stats:\n";
    printf "  RAID ID:                          %u\n", $info{RAIDID};
    printf "  Queue depth:                      %u\n", $info{QDEPTH};
    printf "  Average requests per second:      %u\n", $info{AVGRPS};
    printf "  Average sector count per request: %u\n", $info{AVGSCPR};
    print  "  Write request count:              $info{WRTREQCNT}\n";
    print  "  Read request count:               $info{RDREQCNT}\n";
    printf "  Correctable errors:               %u\n", $info{CORRERRS};
    printf "  Percent remaining to initialize:  %u\n", $info{PCTREMAIN};

    print "\n";
}

##############################################################################
# Name:     X1CfgVDiskDelete
#
# Desc:     VDisk Config - VDisk Delete 
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskDelete
{
    my ($self,
        $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1CfgVDiskDelete"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              CFG_VDISK_DELETE,         # operation
              $id,                      # id1
              0,                        # id2
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Delete successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Delete failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgRaidRecover
#
# Desc:     VDisk Config - Raid Recover 
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgRaidRecover
{
    my ($self,
        $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1CfgRaidRecover"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              CFG_VDISK_RAID_RECOVER,   # operation
              $id,                      # id1
              0,                        # id2
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 Raid Recovery successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 Raid Recovery failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskServerDelete
#
# Desc:     VDisk Config - Server Delete 
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskServerDelete
{
    my ($self,
        $sid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1CfgVDiskServerDelete"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              CFG_VDISK_DELETE_SERVER,  # operation
              $sid,                     # id1
              0,                        # id2
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Server Delete successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Server Delete failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskVLinkBreak
#
# Desc:     VDisk Config - Break a VLink
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskVLinkBreak
{
    my ($self,
        $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1CfgVDiskVLinkBreak"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              CFG_VDISK_VLINK_BREAK,    # operation
              $id,                      # id1
              0,                        # id2  
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VLink Break was successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VLink Break failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskErase
#
# Desc:     VDisk Config - VDisk Erase (Initialize) 
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskErase
{
    my ($self,
        $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1CfgVDiskErase"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              CFG_VDISK_ERASE_VDISK,    # operation
              $id,                      # id1
              0,                        # id2
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Erase successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Erase failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskCreate
#
# Desc:     VDisk Config - VDisk Create 
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskCreate
{
    my ($self,
        $id,
        $type,
        $capacity,
        $flag,
        $name) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d'],
                ['d'],
                ['d', 0, 1],
                ['i'],
                ["X1CfgVDiskCreate"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    if (!defined($name))
    {
        $name = "";
    }

    my $rc;
    my %rsp;
    my $operation;

    # Determine if this is a create or expand
    if ($flag == 1)
    {
        # Expand was requested
        if ($type == 0)
        {
            $operation = CFG_VDISK_EXP_RAID_0;
        }
        elsif ($type == 5)
        {
            $operation = CFG_VDISK_EXP_RAID_5;
        }
        elsif ($type == 10)
        {
            $operation = CFG_VDISK_EXP_RAID_10;
        }
        else
        {
            print "Invalid RAID type: 0, 5 and 10 are supported.\n";
            return %rsp;
        }
    }
    else
    {
        # Default to Create
        if ($type == 0)
        {
            $operation = CFG_VDISK_ADD_RAID_0;
        }
        elsif ($type == 5)
        {
            $operation = CFG_VDISK_ADD_RAID_5;
        }
        elsif ($type == 10)
        {
            $operation = CFG_VDISK_ADD_RAID_10;
        }
        else
        {
            print "Invalid RAID type: 0, 5 and 10 are supported.\n";
            return %rsp;
        }
    }

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T2, 

              $operation,               # operation
              $id,                      # id1
              $capacity,                # size
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              $name));                  # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Create successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Create failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskVLinkCreate
#
# Desc:     VDisk Config - Create a VLink
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskVLinkCreate
{
    my ($self, $id, $suIndex, $lun) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFF],
                ['d', 0, 0xFF],
                ["X1CfgVDiskVLinkCreate"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T3, 

              CFG_VDISK_VLINK_CREATE,   # operation
              $id,                      # id1
              $suIndex,                 # id2  
              $lun,                     # id3  
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName


    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VLink Create was successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VLink Create failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskControl
#
# Desc:     VDisk Config - VDisk Create 
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskControl
{
    my ($self,
        $subType,
        $srcVid,
        $destVid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0x80, 0x87],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ["X1CfgVDiskControl"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              $subType,                 # operation
              $srcVid,                  # id1
              $destVid,                 # id2
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Control successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Control failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskSetAttr
#
# Desc:     VDisk Config - Set attribute
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskSetAttr
{
    my ($self,
        $id,
        $attr) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 2],
                ["X1CfgVDiskSetAttr"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T2, 

              CFG_VDISK_SET_ATTRIBUTE,  # operation
              $id,                      # id1
              $attr,                    # size
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Set Attribute successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Set Attribute failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskSetLock
#
# Desc:     VDisk Config - Set lock
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskSetLock
{
    my ($self,
        $id,
        $lock) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 1],
                ["X1CfgVDiskSetLock"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T2, 

              CFG_VDISK_SET_LOCK,       # operation
              $id,                      # id1
              $lock,                    # size
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Set Lock successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Set Lock failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskSetGeoRaid
#
# Desc:     VDisk Config - Set GeoRaid attribute
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskSetGeoRaid
{
    my ($self,
        $id,
        $geoRaid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 1],
                ["X1CfgVDiskSetLock"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T3, 

              CFG_VDISK_SET_GEORAID,    # operation
              $id,                      # id1
              $geoRaid,                 # geoRaid - 1=ON, 0=OFF
              0,0,0,                    # reserved
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Set Lock successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Set Lock failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskSelectTarget
#
# Desc:     VDisk Config - Select Target
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskSelectTarget
{
    my ($self,
        $sid,
        $tid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ["X1CfgVDiskSelectTarget"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              CFG_VDISK_SELECT_TARGET,  # operation
              0,                        # id1
              $sid,                     # id2
              0, 	                    # parm1
              $tid,                     # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Select Target successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Select Target failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskAssignServer
#
# Desc:     VDisk Config - Add or remove a server in a workset.
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskAssignServer
{
    my ($self,
        $sid,
        $wid,
        $flag) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0x0001],
                ["X1CfgVDiskServerInWorkset"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              CFG_VDISK_SERVER_IN_WORKSET,	# operation
              0,                        	# id1
              $sid,                     	# id2
              $wid,                      	# parm1
              $flag,                     	# parm2
              0,                        	# crypt1
              0,                        	# crypt2
              0,                        	# crypt3
              0));                      	# sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse($packet,
            							X1RPKT_CONFIG_VDISK,
            							\&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Server In Workset successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Server In Workset failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1WorksetAssignVBlock
#
# Desc:     VDisk Config - Assign a VBlock to a Workset
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1WorksetAssignVBlock
{
    my ($self,
        $wid,
        $vBlkID,
        $flag) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0x000F],
                ['d', 0, 0x000F],
                ['d', 0, 0x000F],
                ["X1WorksetAssignVBlock"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T4, 

              CFG_VDISK_ASSIGN_VBLOCK,  # operation
              $wid,                     # id1
              $vBlkID,                  # id2
              $flag,                    # flag
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Select Target successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Select Target failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskSetMask
#
# Desc:     VDisk Config - Set Mask
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskSetMask
{
    my ($self,
        $vid,
        $sid,
        $lun,
        $flag) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 63],
                ['d', 0, 1],
                ["X1CfgVDiskSetMask"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              CFG_VDISK_SET_MASK,       # operation
              $vid,                     # id1
              $sid,                     # id2
              $flag,                    # parm1
              $lun,                     # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Set Mask successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Set Mask failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskSetLun
#
# Desc:     VDisk Config - Set Lun
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskSetLun
{
    my ($self,
        $vid,
        $sid,
        $tid,
        $lun) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFF],
                ["X1CfgVDiskSetLun"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              CFG_VDISK_SET_LUN,        # operation
              $vid,                     # id1
              $sid,                     # id2
              $tid,                     # tid
              $lun,                     # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Set Lun successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Set Lun failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     X1CfgVDiskSetVPort
#
# Desc:     VDisk Config - Set the default VPort for a Workset
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskSetVPort
{
    my ($self,
        $wid,
        $vPortID) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF],
                ['d', 0, 0xFF],
                ["X1CfgVDiskSetVPort"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T4, 

              CFG_VDISK_SET_VPORT,      # operation
              $wid,                     # id1
              $vPortID,                 # id2
              0,                        # flag
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Set Default VPort successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Set Default VPort failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskSetCache
#
# Desc:     VDisk Config - Set cache
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskSetCache
{
    my ($self,
        $id,
        $mode) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 1],
                ["X1CfgVDiskSetCache"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my $operation;
    my %rsp;

    # Expand was requested
    if ($mode == 0)
    {
        $operation = CFG_VDISK_VDISK_SET_CACHE_OFF;
    }
    elsif ($mode == 1)
    {
        $operation = CFG_VDISK_VDISK_SET_CACHE_ON;
    }
    else
    {
        print "Invalid cache mode.  Specify 0=VDisk cache off, 1=VDisk cache on\n";
        return %rsp;
    }

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              $operation,               # operation
              $id,                      # id1
              0,                        # id2
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Set Cache successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Set Cache failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskSetGlobalCache
#
# Desc:     VDisk Config - Set cache
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskSetGlobalCache
{
    my ($self, $mode) = @_;

    logMsg("begin\n");


    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              $mode,                    # operation
              0,                        # id1
              0,                        # id2
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1CfgVDiskSetGlobalCache successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1CfgVDiskSetGlobalCache failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskSetName
#
# Desc:     VDisk Config - Set Name
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskSetName
{
    my ($self,
        $cmd,
        $id,
        $name) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF],
                ['d', 0, 0xFFFF],
                ['s'],
                ["X1CfgVDiskSetName"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my $id1 = 0;
    my $id2 = 0;
    my %rsp;

    if ($cmd == CFG_VDISK_SET_SERVER_NAME)
    {
        $id1 = 0;
        $id2 = $id;
    }
    elsif ($cmd == CFG_VDISK_VDISK_SET_NAME)
    {
        $id1 = $id;
        $id2 = 0;
    }
    elsif ($cmd == CFG_VDISK_SET_WORKSET_NAME)
    {
        $id1 = $id;
        $id2 = 0;
    }
    else
    {
        print "Invalid command code.\n";
        return %rsp;
    }

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              $cmd,                     # operation
              $id1,                     # id1
              $id2,                     # id2
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              $name));                  # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Set Name successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Set Name failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskPDiskDefrag
#
# Desc:     VDisk Config - Defrag a PDisk
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskPDiskDefrag
{
    my ($self,
        $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1CfgVDiskPDiskDefrag"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              CFG_VDISK_PDISK_DEFRAG,   # operation
              $id,                      # id1
              0,                        # id2
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 PDisk Defrag successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 PDisk Defrag failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     X1PDiskDefragStatus
#
# Desc:     PDisk defrag status
#
# Input:    none
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1PDiskDefragStatus
{
    my ($self) = @_;

    logMsg("begin\n");

    my $rc;
    my %rsp;


    my $packet = assembleX1Packet(X1PKT_GET_DEFRAG_STATUS, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_DEFRAG_STATUS,
            \&_handleResponseX1PDiskDefragStatus);

    if (%rsp)
    {
        logMsg("X1 PDisk Defrag successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 PDisk Defrag failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     X1CfgVDiskSetScrubbing
#
# Desc:     VDisk Config - Set scrubbing state
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskSetScrubbing
{
    my ($self,
        $id,
        $mode) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 1],
                ["X1CfgVDiskSetScrubbing"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              CFG_VDISK_PDISK_SCRUB,    # operation
              $id,                      # id1
              0,                        # id2
              $mode,                    # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 VDisk Set Scrubbing State successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 VDisk Set Scrubbing State failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskPDiskFail
#
# Desc:     VDisk Config - Fail physical disk
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskPDiskFail
{
    my ($self,
        $id,
        $options,
        $hotSpareId,
        $cmd) = @_;

    logMsg("begin\n");


    # If the X1 op code was not passed in, assume a Pdisk Fail command.
    if (!defined($cmd))
    {
        $cmd = CFG_VDISK_PDISK_FAIL;
    }

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 8],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFF],
                ["X1CfgVDiskPDiskFail"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              $cmd,                     # operation
              $id,                      # id1
              $hotSpareId,              # id2
              $options,                 # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 Fail/Spindown PDisk was successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 Fail/Spindown PDisk failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskPDiskUnfail
#
# Desc:     VDisk Config - Unfail physical disk
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskPDiskUnfail
{
    my ($self,
        $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1CfgVDiskPDiskUnfail"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              CFG_VDISK_PDISK_UNFAIL,   # operation
              $id,                      # id1
              0,                        # id2
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 Unfail PDisk was successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 Unfail PDisk failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskPDiskBeacon
#
# Desc:     VDisk Config - Beacon a physical disk
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskPDiskBeacon
{
    my ($self,
        $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1CfgVDiskPDiskBeacon"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              CFG_VDISK_PDISK_BEACON,   # operation
              $id,                      # id1
              0,                        # id2
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 Physical Disk Beacon was successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 Physical Disk Beacon failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1CfgVDiskPDiskDelete
#
# Desc:     VDisk Config - Delete a physical disk
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskPDiskDelete
{
    my ($self,
        $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1CfgVDiskPDiskDelete"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_T1, 

              CFG_VDISK_PDISK_DELETE,   # operation
              $id,                      # id1
              0,                        # id2
              0,                        # parm1
              0,                        # parm2
              0,                        # crypt1
              0,                        # crypt2
              0,                        # crypt3
              0));                      # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 Physical Disk Delete was successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 Physical Disk Delete failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     X1CfgVDiskAcknowledgeLog
#
# Desc:     VDisk Config - Acknowledge a log (?? Don't ask me ??)
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1CfgVDiskAcknowledgeLog
{
    my ($self,
        $eventNumber) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ["X1CfgVDiskAcknowledgeLog"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $data = _CfgVDiskEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_VDISK_LOG, 

                          CFG_VDISK_LOG_ACKNOWLEDGE, # operation
                          $eventNumber,              # eventNumber
                          0,                         # rsvd
                          0,                         # crypt1
                          0,                         # crypt2
                          0,                         # crypt3
                          0));                       # sName

    my $packet = assembleX1Packet(X1PKT_CONFIG_VDISK, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_VDISK,
            \&_handleResponseX1StatusOnlyGood0);


    $rc = 0;

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            logMsg("X1 Acknowledge log was successful\n");
            $rc = 1;
        }
    }
    else
    {
        logMsg("X1 Acknowledge log failed\n");
    }

    return %rsp;
}


##############################################################################
# Name:     X1HabConfig
#
# Desc:     Mag Hab Config
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
use constant X1_CONFIG_HAB =>
           "C          # operation
            C          # target
            C          # loopId
            C          # crypt1
            C";        # crypt2

sub X1HabConfig
{
    my ($self,
        $opType,
        $target,
        $loopId) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF],
                ['d', 0, 0xFF],
                ['d', 0, 0xFF],
                ["X1HabConfig"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my $operation;
    my %rsp;

    my $data = _CfgHabEncrypt($self->{MASTER_CRYPT}, pack(X1_CONFIG_HAB, 

              $opType,               # operation
              $target,               # target
              $loopId,               # loopId
              0,                     # crypt1
              0));                   # crypt2

    my $packet = assembleX1Packet(X1PKT_CONFIG_HAB, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_CONFIG_HAB,
            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1 Hab Config successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1 Hab Config failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetPDiskInfo
#
# Desc:     Get the Physical Disk Info
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GetPDiskInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1GetPDiskInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("S",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_PINFO, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_PINFO,
            \&_handleResponseX1GetPDiskInfo);

    if (%rsp)
    {
        logMsg("X1GetPDiskInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetPDiskInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     X1GetBayInfo
#
# Desc:     Get the Disk Bay Info
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GetBayInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1GetBayInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("S",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_BAY_INFO, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_BAY_INFO,
            \&_handleResponseX1GetBayInfo);

    if (%rsp)
    {
        logMsg("X1GetBayInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetBayInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetVDiskInfo
#
# Desc:     Get the Virtual Disk Info
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GetVDiskInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1GetVDiskInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("S",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_VINFO, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_VINFO,
            \&_handleResponseX1GetVDiskInfo);

    if (%rsp)
    {
        logMsg("X1GetVDiskInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetVDiskInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetVDiskCopyInfo
#
# Desc:     Get the Virtual Disk Copy Info
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GetVDiskCopyInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1GetVDiskCopyInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("S",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_COPY_INFO, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_COPY_INFO,
            \&_handleResponseX1GetVDiskCopyInfo);

    if (%rsp)
    {
        logMsg("X1GetVDiskCopyInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetVDiskCopyInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetRaidInfo
#
# Desc:     Get the RAID Info
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GetRaidInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1GetRaidInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("S",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_RINFO, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_RINFO,
            \&_handleResponseX1GetRaidInfo);

    if (%rsp)
    {
        logMsg("X1GetRaidInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetRaidInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetPDiskStats
#
# Desc:     Get the PDisk Stats
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GetPDiskStats
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1GetPDiskStats"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("S",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_PSTATS, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_PSTATS,
            \&_handleResponseX1GetPDiskStats);

    if (%rsp)
    {
        logMsg("X1GetPDiskStats successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetPDiskStats failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetPDiskPState
#
# Desc:     Get the PDisk PState
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GetPDiskPState
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["X1GetPDiskPState"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_PSTATE, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_PSTATE,
            \&_handleResponseX1GetPDiskPState);

    if (%rsp)
    {
        logMsg("X1GetPDiskPState successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetPDiskPState failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetVDiskStats
#
# Desc:     Get the VDisk Stats
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GetVDiskStats
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1GetVDiskStats"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("S",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_VSTATS, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_VSTATS,
            \&_handleResponseX1GetVDiskStats);

    if (%rsp)
    {
        logMsg("X1GetVDiskStats successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetVDiskStats failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetRaidStats
#
# Desc:     Get the RAID Stats
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GetRaidStats
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1GetRaidStats"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("S",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_RSTATS, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_RSTATS,
            \&_handleResponseX1GetRaidStats);

    if (%rsp)
    {
        logMsg("X1GetRaidStats successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetRaidStats failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     _handleResponseX1StatusOnlyGood0
#
# Desc:     Handle an X1 return packet (status only, Good == 0).
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       STATUS   Status of the command
#
##############################################################################
sub _handleResponseX1StatusOnlyGood0
{
    my ($self,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    my $status = unpack "C", $parts{DATA};
    if ($status == 0) {
        $info{STATUS} = PI_GOOD;
    }
    else {
        $info{STATUS} = PI_ERROR;
    }

    return %info;
}


##############################################################################
# Name:     _handleResponseX1GetPDiskInfo
#
# Desc:     Handle an X1 Get PDisk Info return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################

# X1_PDISK_INFO
use constant X1_PDISK_INFO =>
           "S           # pdiskNumber
            C           # class
            a4          # name
            L           # system serial number
            C           # miscStat
            C           # postStat
            C           # devStat
            C           # percentLeft
            LL          # device capacity
            a16         # product ID
            a8          # vendor ID
            a4          # PDisk firmware revision
            a12         # PDisk serial number
            LL          # total available space
            LL          # largest available space
            C           # enclosure number
            C           # slot number
            C           # path count
            C           # path 0
            C           # path 1
            C           # path 2
            C           # path 3 
            LL          # WWN
            L           # fibre port ID
            C           # devType
            a10";       # RESERVED

sub _handleResponseX1GetPDiskInfo
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %devcap;
    my %tas;
    my %las;
    my %wwn;
    my $rsvd;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{PDISKID},
      $info{CLASS},
      $info{NAME},
      $info{SYSTEMSN},
      $info{MISCSTAT},
      $info{POSTSTAT},
      $info{DEVSTAT},
      $info{PERCENTLEFT},
      $devcap{LO_LONG}, $devcap{HI_LONG},
      $info{PRODUCTID},
      $info{VENDORID},
      $info{FWREVISION},
      $info{PDISKSN},
      $tas{LO_LONG}, $tas{HI_LONG},
      $las{LO_LONG}, $las{HI_LONG},
      $info{ENCLNUM},
      $info{SLOTNUM},
      $info{PATHCOUNT},
      $info{PATH0},
      $info{PATH1},
      $info{PATH2},
      $info{PATH3},
      $wwn{LO_LONG}, $wwn{HI_LONG},
      $info{FIBREID},
      $info{DEVTYPE},
      $rsvd) = unpack X1_PDISK_INFO, $parts{DATA};

    # Now fixup all the 64 bit  numbers
    $info{DEVCAP} = longsToBigInt(%devcap);
    $info{DEVCAP_HI} = $devcap{HI_LONG};
    $info{DEVCAP_LO} = $devcap{LO_LONG};

    $info{TAS} = longsToBigInt(%tas);
    $info{TAS_HI} = $tas{HI_LONG};
    $info{TAS_LO} = $tas{LO_LONG};

    $info{LAS} = longsToBigInt(%las);
    $info{LAS_HI} = $las{HI_LONG};
    $info{LAS_LO} = $las{LO_LONG};

    $info{WWN} = longsToBigInt(%wwn);
    $info{WWN_HI} = $wwn{HI_LONG};
    $info{WWN_LO} = $wwn{LO_LONG};
    
    return %info;
}


##############################################################################
# Name:     _handleResponseX1GetBayInfo
#
# Desc:     Handle an X1 Get Disk Bay Info return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################

# X1_BAY_INFO
use constant X1_BAY_INFO =>
           "S           # bayNumber
            C           # devStat
            a16         # product ID
            a8          # vendor ID
            a4          # firmware revision
            a12         # serial number
            LL          # WWN
            C           # loopStatus
            C           # ioModuleStatus1
            C           # ioModuleStatus2
            C           # path count
            C           # path 0
            C           # path 1
            C           # path 2
            C           # path 3 
            C           # shelf ID 
            C           # devType
            a16";       # RESERVED

sub _handleResponseX1GetBayInfo
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %wwn;
    my $rsvd;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{BAYID},
      $info{DEVSTAT},
      $info{PRODUCTID},
      $info{VENDORID},
      $info{FWREVISION},
      $info{BAYSN},
      $wwn{LO_LONG}, $wwn{HI_LONG},
      $info{LOOPSTATUS},
      $info{IOMODSTATUS1},
      $info{IOMODSTATUS2},
      $info{PATHCOUNT},
      $info{PATH0},
      $info{PATH1},
      $info{PATH2},
      $info{PATH3},
      $info{SHELFID},
      $info{DEVTYPE},
      $rsvd) = unpack X1_BAY_INFO, $parts{DATA};

    # Now fixup all the 64 bit  numbers
    $info{WWN} = longsToBigInt(%wwn);
    $info{WWN_HI} = $wwn{HI_LONG};
    $info{WWN_LO} = $wwn{LO_LONG};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetVDiskInfo
#
# Desc:     Handle an X1 Get VDisk Info return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################

# X1_VDISK_INFO
use constant X1_VDISK_INFO =>
           "S           # vdisk ID
            C           # devStat
            LL          # device capacity
            a256        # RAID map
            C           # attribute 1
            a16         # name
            C           # attribute 2
            C";         # device type

sub _handleResponseX1GetVDiskInfo
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %devcap;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{VDISKID},
      $info{DEVSTAT},
      $devcap{LO_LONG}, $devcap{HI_LONG},
      $info{RAIDMAP},
      $info{ATTRIB1},
      $info{NAME},
      $info{ATTRIB2},
      $info{DEVTYPE}) = unpack X1_VDISK_INFO, $parts{DATA};

    # Now fixup all the 64 bit  numbers
    $info{DEVCAP} = longsToBigInt(%devcap);
    $info{DEVCAP_HI} = $devcap{HI_LONG};
    $info{DEVCAP_LO} = $devcap{LO_LONG};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetVDiskCopyInfo
#
# Desc:     Handle an X1 Get VDisk Info return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################

# X1_VDISK_INFO
use constant X1_VDISK_COPY_INFO =>
           "S           # Destination VDisk ID
            S           # Source VDisk ID
            C           # percent complete
            C";         # devStat
            
sub _handleResponseX1GetVDiskCopyInfo
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %devcap;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{DESTVID},
      $info{SRCVID},
      $info{PERCENTCMPL},
      $info{DEVSTAT}) = unpack X1_VDISK_COPY_INFO, $parts{DATA};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetRaidInfo
#
# Desc:     Handle an X1 Get RAID Info return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################

# X1_RAID_INFO
use constant X1_RAID_INFO =>
           "S           # raid ID
            S           # vdisk owning this raid ID
            C           # raid class
            C           # change count (whatever that is)
            C           # devStat
            C           # mirrorDepth
            C           # stripeWidth
            C           # status
            LL          # device capacity
            L           # sectors per stripe
            a128        # pdisk exist bitmap
            a128        # pdisk fail bitmap
            a128        # pdisk rebuild bitmap
            C";         # device type

sub _handleResponseX1GetRaidInfo
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %devcap;
    my $rsvd;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{RAIDID},
      $info{VDISKID},
      $info{CLASS},
      $info{CHGCNT},
      $info{DEVSTAT},
      $info{MIRRORDEPTH},
      $info{STRIPEWIDTH},
      $info{STATUS},
      $devcap{LO_LONG}, $devcap{HI_LONG},
      $info{SECTORSPERSTRIPE},
      $info{EXISTMAP},
      $info{FAILMAP},
      $info{REBUILDMAP},
      $info{DEVTYPE}) = unpack X1_RAID_INFO, $parts{DATA};

    # Now fixup all the 64 bit  numbers
    $info{DEVCAP} = longsToBigInt(%devcap);
    $info{DEVCAP_HI} = $devcap{HI_LONG};
    $info{DEVCAP_LO} = $devcap{LO_LONG};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetPDiskStats
#
# Desc:     Handle an X1 Get PDisk Stats return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################

# X1_PDISK_STATS
use constant X1_PDISK_STATS =>
           "S           # pdisk ID
            L           # queue depth
            L           # Average requests per second
            L           # Average sector count per request
            LL          # Write request count
            LL          # Read request count
            L           # Correctable errors ???
            C           # Enclosure number
            C";         # Slot number

sub _handleResponseX1GetPDiskStats
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %wrtreqcnt;
    my %rdreqcnt;
    my $rsvd;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{PDISKID},
      $info{QDEPTH},
      $info{AVGRPS},
      $info{AVGSCPR},
      $wrtreqcnt{LO_LONG}, $wrtreqcnt{HI_LONG},
      $rdreqcnt{LO_LONG}, $rdreqcnt{HI_LONG},
      $info{CORRERRS},
      $info{ENCLNUM},
      $info{SLOTNUM}) = unpack X1_PDISK_STATS, $parts{DATA};

    # Now fixup all the 64 bit  numbers
    $info{WRTREQCNT} = longsToBigInt(%wrtreqcnt);
    $info{WRTREQCNT_HI} = $wrtreqcnt{HI_LONG};
    $info{WRTREQCNT_LO} = $wrtreqcnt{LO_LONG};

    $info{RDREQCNT} = longsToBigInt(%rdreqcnt);
    $info{RDREQCNT_HI} = $rdreqcnt{HI_LONG};
    $info{RDREQCNT_LO} = $rdreqcnt{LO_LONG};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1PDiskDefragStatus
#
# Desc:     Handle an X1 Get PDisk Stats return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1PDiskDefragStatus
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $rsvd;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{PDISKID},
      $info{FLAGS},
      $rsvd) = unpack "SCC", $parts{DATA};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetPDiskPState
#
# Desc:     Handle an X1 Get PDisk PState return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1GetPDiskPState
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %wrtreqcnt;
    my %rdreqcnt;
    my $rsvd;
    my @states;
    my $i;

    my %parts = disassembleX1Packet($recvPacket);

    for ($i = 0; $i < 256; $i++)
    {
        $states[$i] = unpack "C", substr($parts{DATA}, $i);
    }

    $info{STATES} = [@states];

    (
    $info{SCRUB_STATE},
    $info{COPY_REBUILD_PRI},
    $info{SCSI_BITMAP},
    $info{VERSION},
    $info{REVISION},
    $info{HEARTBEAT}
    ) = unpack "CCCSSL", substr($parts{DATA}, 256);

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetVDiskStats
#
# Desc:     Handle an X1 Get VDisk Stats return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################

# X1_VDISK_STATS
use constant X1_VDISK_STATS =>
           "S           # vdisk ID
            L           # queue depth
            L           # Average requests per second
            L           # Average sector count per request
            LL          # Write request count
            LL";        # Read request count

sub _handleResponseX1GetVDiskStats
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %wrtreqcnt;
    my %rdreqcnt;
    my $rsvd;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{VDISKID},
      $info{QDEPTH},
      $info{AVGRPS},
      $info{AVGSCPR},
      $wrtreqcnt{LO_LONG}, $wrtreqcnt{HI_LONG},
      $rdreqcnt{LO_LONG}, $rdreqcnt{HI_LONG}) = unpack X1_VDISK_STATS, $parts{DATA};

    # Now fixup all the 64 bit  numbers
    $info{WRTREQCNT} = longsToBigInt(%wrtreqcnt);
    $info{WRTREQCNT_HI} = $wrtreqcnt{HI_LONG};
    $info{WRTREQCNT_LO} = $wrtreqcnt{LO_LONG};

    $info{RDREQCNT} = longsToBigInt(%rdreqcnt);
    $info{RDREQCNT_HI} = $rdreqcnt{HI_LONG};
    $info{RDREQCNT_LO} = $rdreqcnt{LO_LONG};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetRaidStats
#
# Desc:     Handle an X1 Get RAID Stats return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################

# X1_RAID_STATS
use constant X1_RAID_STATS =>
           "S           # RAID ID
            L           # queue depth
            L           # Average requests per second
            L           # Average sector count per request
            LL          # Write request count
            LL          # Read request count
            L           # Correctable errors ???
            C";         # % storage remaining to be initialized

sub _handleResponseX1GetRaidStats
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %wrtreqcnt;
    my %rdreqcnt;
    my $rsvd;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{RAIDID},
      $info{QDEPTH},
      $info{AVGRPS},
      $info{AVGSCPR},
      $wrtreqcnt{LO_LONG}, $wrtreqcnt{HI_LONG},
      $rdreqcnt{LO_LONG}, $rdreqcnt{HI_LONG},
      $info{CORRERRS},
      $info{PCTREMAIN}) = unpack X1_RAID_STATS, $parts{DATA};

    # Now fixup all the 64 bit  numbers
    $info{WRTREQCNT} = longsToBigInt(%wrtreqcnt);
    $info{WRTREQCNT_HI} = $wrtreqcnt{HI_LONG};
    $info{WRTREQCNT_LO} = $wrtreqcnt{LO_LONG};

    $info{RDREQCNT} = longsToBigInt(%rdreqcnt);
    $info{RDREQCNT_HI} = $rdreqcnt{HI_LONG};
    $info{RDREQCNT_LO} = $rdreqcnt{LO_LONG};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetVMap
#
# Desc:     Handle an X1 Get VMAP (VDisk Map) return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       
#
##############################################################################
sub _handleResponseX1GetVMap
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    $info{VDISKBITMAP} = unpack X1_VDISK_MAP, $parts{DATA};

    return %info;
}


##############################################################################
# Name:     _handleResponseX1GetVDiskCopyMap
#
# Desc:     Handle an X1 Get VDisk Copy return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       
#
##############################################################################
sub _handleResponseX1GetVDiskCopyMap
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    $info{VDISKCOPYBITMAP} = unpack X1_VDISK_MAP, $parts{DATA};

    return %info;
}


##############################################################################
# Name:     _handleResponseX1GetVDiskMirrorMap
#
# Desc:     Handle an X1 Get VDisk Mirror Map return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       
#
##############################################################################
sub _handleResponseX1GetVDiskMirrorMap
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    $info{VDISKMIRRORBITMAP} = unpack X1_VDISK_MAP, $parts{DATA};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetVDiskMirrorIOStatusMap
#
# Desc:     Handle an X1 Get VDisk Mirror Map return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       
#
##############################################################################
sub _handleResponseX1GetVDiskMirrorIOStatusMap
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    $info{VDISKMIRRORIOSTATUSBITMAP} = unpack X1_VDISK_MAP, $parts{DATA};

    return %info;
}


##############################################################################
# Name:     _handleResponseX1GetFreeSpace
#
# Desc:     Handle an X1 Get Free Space return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       
#
##############################################################################
use constant X1_FREE_SPACE =>
           "LL          # RAID 0
            LL          # RAID 5
            LL";        # RAID 10


sub _handleResponseX1GetFreeSpace
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %raid0;
    my %raid5;
    my %raid10;

    my %parts = disassembleX1Packet($recvPacket);

    ( $raid0{LO_LONG}, $raid0{HI_LONG},
      $raid5{LO_LONG}, $raid5{HI_LONG},
      $raid10{LO_LONG}, $raid10{HI_LONG}
    ) = unpack X1_FREE_SPACE, $parts{DATA};

    # Now fixup all the 64 bit  numbers
    $info{RAID0} = longsToBigInt(%raid0);
    $info{RAID0_HI} = $raid0{HI_LONG};
    $info{RAID0_LO} = $raid0{LO_LONG};
    
    # Now fixup all the 64 bit  numbers
    $info{RAID5} = longsToBigInt(%raid5);
    $info{RAID5_HI} = $raid5{HI_LONG};
    $info{RAID5_LO} = $raid5{LO_LONG};
    
    # Now fixup all the 64 bit  numbers
    $info{RAID10} = longsToBigInt(%raid10);
    $info{RAID10_HI} = $raid10{HI_LONG};
    $info{RAID10_LO} = $raid10{LO_LONG};
    
    return %info;
}

##############################################################################
# Name:     _CfgVDiskEncrypt
#
# Desc:     "Encrypt" the VDisk Config data 
#
# Input:    the data to encrypt
#
# Returns:  the encrypted data
#
##############################################################################
sub _CfgVDiskEncrypt
{
    my ($masterCrypt, $data) = @_;
    
    my ($p0, $p1, $p2, $p3, $crypt1, $crypt2, $crypt3, $theRest) = 
        unpack X1_CONFIG_VDISK_ENCRYPT, $data;

    $crypt1 = ($masterCrypt & 0xFF) ^ $p1;
    $crypt2 = (($masterCrypt >> 8) & 0xFF) ^ $p2;
    $crypt3 = $masterCrypt ^ $p3;
        
    $data = pack X1_CONFIG_VDISK_ENCRYPT, 
        $p0, $p1, $p2, $p3, $crypt1, $crypt2, $crypt3, $theRest;

    return $data;
}


##############################################################################
# Name:     _CfgHabEncrypt
#
# Desc:     "Encrypt" the HAB Config data 
#
# Input:    the data to encrypt
#
# Returns:  the encrypted data
#
##############################################################################
sub _CfgHabEncrypt
{
    my ($masterCrypt, $data) = @_;
    
    my ($opType, $target, $loopId, $crypt1, $crypt2) = 
        unpack X1_CONFIG_HAB, $data;

    $crypt1 = ($masterCrypt & 0xFF) ^ $target;
    $crypt2 = (($masterCrypt >> 8) & 0xFF) ^ $loopId;
        
    $data = pack(X1_CONFIG_HAB, 

              $opType,               # operation
              $target,               # id1
              $loopId,               # id2
              $crypt1,               # crypt1
              $crypt2);              # crypt2

    return $data;
}




1;

##############################################################################
# Change log:
# $Log$
# Revision 1.3  2005/05/25 10:09:44  BharadwajS
# Undo vpri changes
#
# Revision 1.2  2005/05/24 14:18:35  BharadwajS
# TBolt00000 vpri
#
# Revision 1.1.1.1  2005/05/04 18:53:54  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.39  2005/03/22 18:08:47  SchibillaM
# TBolt0000000: Add support for X1VDISKCOPYINFO.
#
# Revision 1.38  2005/01/31 15:45:06  SchibillaM
# TBolt00012221: Add support for X1PKT_GET_MIRROR_IO_STATUS.
# Reviewed by Chris.
#
# Revision 1.37  2004/08/19 20:42:47  SchibillaM
# TBolt00011069: Add support for Defrag Status command.
#
# Revision 1.36  2004/07/14 21:01:46  NigburC
# TBolt00009884 - Added code to set the virtual disk name if passed in on
# the X1 vdisk create request.  The name will be set if the create operation
# was successful and the name is not empty.
# Reviewed by Mark Schibilla.
#
# Revision 1.35  2004/06/15 18:43:05  SchibillaM
# TBolt00010632: Add support for X1 Server and HBA stats.  New Stats Manager
# component.  HBA stats framework done, waiting for proc support.  Reviewed by Chris.
#
# Revision 1.34  2004/03/23 18:46:02  SchibillaM
# TBolt00010148: Add support for devType field in X1 RAID and VDisk info.
# Reviewed by Chris.
#
# Revision 1.33  2004/03/08 22:17:15  RysavyR
# TBolt00010199: Add MRP - MRRAIDRECOVER to recover an inoperative raid.
# Rev by Tim Swatosh..
#
# Revision 1.32  2004/03/08 14:54:45  SchibillaM
# TBolt00010148: SATA changes - devType in PDisk and Bay Info, new environmental
# info for SATA bays.  Reviewed by Randy.
#
# Revision 1.31  2003/10/16 19:51:00  SchibillaM
# TBolt00009396: Report defrag status in X1 RAID Info.
#
# Revision 1.30  2003/09/30 14:21:59  SchibillaM
# TBolt00009208: Add support for the X1 CFG_VDISK_DELETE_SERVER command.
# This command takes an X1Sid as input and deletes all associated procSids.
# Reviewed by Chris.
#
# Revision 1.29  2003/08/28 16:30:10  SchibillaM
# TBolt00009060: Remove worksetID from CfgVDisk Select VPort for Server.  Add
# cmd to add-remove a server in a workset.  Reviewed by Bryan.
#
# Revision 1.28  2003/08/20 18:44:36  SchibillaM
# TBolt00009016: Add X1VDiskCfg subcommand for GeoRaid attribute.
#
# Revision 1.27  2003/08/19 20:58:44  SchibillaM
# TBolt00008962: Add defaultVPort to workset.
#
# Revision 1.26  2003/08/05 18:58:57  SchibillaM
# TBolt00008793: Complete GeoPool support in CCB and CCBE.
# Reviewed by Randy.
#
# Revision 1.25  2003/07/18 19:16:15  SchibillaM
# TBolt00008030: Complete X1 Workset support.  Reviewed by Randy.
#
# Revision 1.24  2003/04/22 13:39:31  TeskeJ
# tbolt00008122 - 'pdisks loop' updates and changed 'channel' to 'port'
# rev by Chris
#
# Revision 1.23  2003/04/08 21:00:10  SchibillaM
# TBolt00007915: Fixes to X1VLink Create and Break.
#
# Revision 1.22  2003/03/27 20:51:12  SchibillaM
# TBolt00007915: Starting point for X1 VLink Create and Break.
#
# Revision 1.21  2003/01/27 22:54:28  NigburC
# TBolt00006950 - Added a new X1 command to get the PDISK PState
# information from the CCBE.
# Reviewed by Jeff Williams.
#
# Revision 1.20  2003/01/21 21:17:50  SchibillaM
# TBolt00006884: Add UINT8 shelfId field to the end of the X1_BAY_INFO_RSP
# and change compatibility from 0x0A to 0x0B.  Reviewed by Chris.
#
# Revision 1.19  2003/01/13 16:51:22  SchibillaM
# TBolt00006514: Add support for BayInfo and VDisk Erase.
#
# Revision 1.18  2002/12/23 14:05:02  SchibillaM
# TBolt00000000: Added support for X1 PDisk, VDisk and RAID info and Stats.
#
# Revision 1.17  2002/12/20 20:08:16  SchibillaM
# TBolt00000000: Add CCBE support for X1 PDisk, VDisk and RAID Info.
#
# Revision 1.16  2002/12/17 19:53:46  NigburC
# TBolt00006471, TBolt00006472 - Fixed these two defects.  The first was
# to enable the X1 PDisk Beacon handler to use the duration value.  The
# second was to correct the byte used for the lock/unlock value in the X1
# VDisk Set Lock command.
# Reviewed By Mark Schibilla.
#
# Revision 1.15  2002/12/13 22:13:47  SchibillaM
# TBolt00006408: Checked in support for X1VDiskSetLun.
#
# Revision 1.14  2002/12/11 19:24:36  SchibillaM
# TBolt00000000: Add support for X1FreeSpace cmd.  Reviewed by Chris.
#
# Revision 1.13  2002/12/11 16:22:43  NigburC
# TBolt00006452, TBolt00006451 - Added code to support the SET_LOCK
# operation for X1 VDISK CONFIG.  This code required changes to the
# underlying structures and MRPs for virtual disk information, setting cache,
# and setting attributes.
#
# - PI_VDISK_SET_CACHE_CMD has been removed.
# - PI_VDISK_SET_ATTRIBUTE_CMD is used to set attributes, including
# virtual disk cache.
# - PI_VCG_SET_CACHE_CMD is used to set global caching.
# Reviewed by Mark Schibilla and Randy Rysavy.
#
# Revision 1.12  2002/12/10 21:22:56  SchibillaM
# TBolt00006408: Implement 0xAB operation in VDisk Config to Select a Target.
# Reviewed by Chris.
#
# Revision 1.11  2002/12/06 21:37:50  NigburC
# TBolt00006392, TBolt00006394, TBolt00006429 - Lots of changes to enjoy.
# - Added code to support the new NAME_DEVICE MRP.
# - Added code to support setting server, vdisk and controller names.
# - Updated the SERVERASSOC and SERVERDELETE commands to allow
# additional options.
# Reviewed by Mark Schibilla.
#
# Revision 1.10  2002/12/05 19:32:22  NigburC
# TBolt00006359 - Updated the CCBE code for the X1 label to match the data
# the service sends.
# Reviewed by Mark Schibilla (virtually).
#
# Revision 1.9  2002/12/03 18:29:44  SchibillaM
# TBolt00000000: Add support for X1RPKT_GET_COPY_STATUS (copy map)
# and X1PKT_GET_MIRROR_STATUS (mirror map).
#
# Revision 1.8  2002/11/22 17:53:34  HoltyB
# TBolt00006369: Added X1 interface to annotate and acknowledge logs.
#
# Revision 1.7  2002/11/01 19:53:33  SchibillaM
# TBolt00000000: Add X1VDiskMap support.
#
# Revision 1.6  2002/10/29 22:38:40  RysavyR
# TBolt00006013: Added "encryption" bytes to X1 HAB Config. Rev by Tim Swatosh.
#
# Revision 1.5  2002/10/29 22:04:16  SwatoshT
# TBolt00006013: Added support for X1Hab Config packet. Added new modifier
# field to Target Properties structures.
# Reviewed by Randy R.
#
# Revision 1.4  2002/10/28 19:07:08  SchibillaM
# TBolt00006013: Finish support for Config VDisk X1 packet.
# Reviewed by Randy.
#
# Revision 1.3  2002/10/25 16:04:41  RysavyR
# TBolt00006013: Added X1 config "encryption" support and added intermediate
# VDisk config ACK.  Rev by Mark S.
#
# Revision 1.2  2002/10/25 14:07:42  SchibillaM
# TBolt00006013: Added support for most Config VDisk requests.
# Reviewed by Randy.
#
# Revision 1.1  2002/10/22 21:37:10  RysavyR
# TBolt00006013:  Started adding VDISK Config support on the X1 port.
#
# Revision 1.1  2002/10/16 21:02:42  RysavyR
# TBolt00006136: Added support for X1GETACCOUNT and X1SETACCOUNT.
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
