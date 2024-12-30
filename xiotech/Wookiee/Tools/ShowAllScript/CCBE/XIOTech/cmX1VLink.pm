# $Id$
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2002  Xiotech
# ======================================================================
# Purpose:  Support for X1 VLink related stuff
##############################################################################
package XIOTech::cmdMgr;

use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;

use XIOTech::error;

use XIOTech::logMgr;

use strict;

##############################################################################
# Name:     DisplayX1VLinkInfo
#
# Desc:     Display X1 VLink info
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1VLinkInfo
{
    my ($self, %info) = @_;

    print "X1 VLink Info:\n";
    printf "  VLink ID (index):         %u\n", $info{DINDEX};
    printf "  Link Status:              0x%02X\n", $info{LINKSTATUS};
    printf "  Type:                     0x%02X\n", $info{TYPE};
    printf "  Capacity:                 %u\n", $info{DEVCAP};
    printf "  Source name:              %s\n", $info{SRCVNAME};
    printf "  Source serial number:     %u\n", $info{SRCSN};
    printf "  Source VBlock:            %u\n", $info{SRCVBLK};
    printf "  Source VID:               %u\n", $info{SRCVID};
    printf "  Source VPort:             %u\n", $info{SRCVPORT};
    printf "  Source # of connections:  %u\n", $info{SRCNUMCONN};
    printf "  Node WWN:                 %8.8X%8.8X\n", $info{NODEWWN_LO}, $info{NODEWWN_HI};
    printf "  Port WWN 1:               %8.8X%8.8X\n", $info{PORTWWN1_LO}, $info{PORTWWN1_HI};
    printf "  Port WWN 2:               %8.8X%8.8X\n", $info{PORTWWN2_LO}, $info{PORTWWN2_HI};
    printf "  Base serial number:       %u\n", $info{BASESN};
    printf "  Base VBlock:              %u\n", $info{BASEVBLK};
    printf "  Base VID:                 %u\n", $info{BASEVID};
    print  "  Product ID:               $info{PRODID}\n";
    print  "  Vendor ID:                $info{VENDID}\n";
    print  "  Source name:              $info{SRCNAME}\n";
    print  "  Firmware revision:        $info{FWREV}\n";
    print  "  Serial number:            $info{SN}\n";
    print "\n";
}

##############################################################################
# Name:     DisplayX1VLinkLockInfo
#
# Desc:     Display X1 VLink Lock info
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1VLinkLockInfo
{
    my ($self, %info) = @_;

    print "X1 VLink Lock Info:\n";
    printf "  VID:                      %u\n", $info{VID};
    printf "  Lock serial number:       %u\n", $info{LOCKSN};
    printf "  Lock VBlock:              %u\n", $info{LOCKVBLOCK};
    printf "  Lock VID:                 %u\n", $info{LOCKVID};
    print  "  Lock VDisk name:          $info{VDISKNAME}\n";
    print  "  Lock SU name:             $info{SUNAME}\n";
    print "\n";
}

##############################################################################
1;
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
