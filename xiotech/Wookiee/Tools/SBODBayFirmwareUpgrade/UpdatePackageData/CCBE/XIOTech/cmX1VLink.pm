# $Header$
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2002  Xiotech
# ======================================================================
# $RCSfile$
#
# Author:   Mark Schibilla
#
# Purpose:  Support for X1 VLink elated stuff
#   
#   
##############################################################################
package XIOTech::cmdMgr;

use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;

use XIOTech::error;

use XIOTech::logMgr;

use strict;



##############################################################################
# Name:     X1GetVLinkSUList
#
# Desc:     Get X1 VLink Storage Unit Count List
#
# Input:    none
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#           (lots...)
#
##############################################################################
sub X1GetVLinkSUList
{
    my ($self) = @_;
        
    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_SU_LIST, undef);

    %rsp = $self->_handleX1SyncResponse($packet,
                                        X1PKT_REPLY_SU_CNT,
                                        \&_handleResponseX1GetVLinkSUList);

    if (%rsp)
    {
        logMsg("X1GetVLinkSUList successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetVLinkSUList failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetVLinkedToList
#
# Desc:     Get X1 VLinked To Controller List
#
# Input:    none
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#           (lots...)
#
##############################################################################
sub X1GetVLinkedToList
{
    my ($self) = @_;
        
    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_VLINKED_TO, undef);

    %rsp = $self->_handleX1SyncResponse($packet,
                                        X1RPKT_VLINKED_TO,
                                        \&_handleResponseX1GetVLinkedToList);

    if (%rsp)
    {
        logMsg("X1GetVLinkedToList successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetVLinkedToList failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     X1GetVLinkLunInfo
#
# Desc:     Get X1 VLinked To Controller List
#
# Input:    none
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#           (lots...)
#
##############################################################################
sub X1GetVLinkLunInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF],
                ["X1GetVLinkLunInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("C",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_SU_LUN_INFO, $data);

    %rsp = $self->_handleX1SyncResponse($packet,
                                        X1PKT_REPLY_LUN_CNT,
                                        \&_handleResponseX1GetVLinkLunInfo);

    if (%rsp)
    {
        logMsg("X1GetVLinkLunInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetVLinkLunInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetVLinkInfo
#
# Desc:     Get X1 VLink Info
#
# Input:    none
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#           (lots...)
#
##############################################################################
sub X1GetVLinkInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1GetVLinkInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("S",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_DLINK, $data);

    %rsp = $self->_handleX1SyncResponse($packet,
                                        X1RPKT_GET_DLINK,
                                        \&_handleResponseX1GetVLinkInfo);

    if (%rsp)
    {
        logMsg("X1GetVLinkInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetVLinkInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetVLinkLockInfo
#
# Desc:     Get X1 VLink Lock Info
#
# Input:    none
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#           (lots...)
#
##############################################################################
sub X1GetVLinkLockInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1GetVLinkLockInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("S",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_DLOCK_INFO, $data);

    %rsp = $self->_handleX1SyncResponse($packet,
                                        X1RPKT_GET_DLOCK_INFO,
                                        \&_handleResponseX1GetVLinkLockInfo);

    if (%rsp)
    {
        logMsg("X1GetVLinkLockInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetVLinkLockInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}





##############################################################################
# Name:     _handleResponseX1GetVLinkSUList
#
# Desc:     Handle an X1 Get VLink Storage Unit return packet.
#
#           This information is actually sent as 1 count packet followed
#           by 0 or more separate info packets.  Since CCBE is not set up
#           to handle multiple response packets we just handle the count here.
#           The async mechanism will have to handle the info packets.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1GetVLinkSUList
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $start;

    my %parts = disassembleX1Packet($recvPacket);

    # Get the count of storage units from the start of the packet.
    (
        $info{COUNT}
    ) = unpack("C", $parts{DATA});

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetVLinkedToList
#
# Desc:     Handle an X1 Get VLinked To List return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1GetVLinkedToList
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $numControllers = 0;

    my %parts = disassembleX1Packet($recvPacket);

    if (defined($parts{DATA}))
    {
        $numControllers = length($parts{DATA}) / 2;  
    }    
    
    my @controllers;
    my $start = 0;

    # Loop and read in numControllers serial numbers
    for (my $i = 0; $i < $numControllers; $i++)
    {
        $controllers[$i] = unpack("S", substr($parts{DATA}, $start));

        $start = $start + 2;
    }

    $info{COUNT} =  $numControllers;
    $info{CONTROLLERS} = [@controllers];

    return %info;
}


##############################################################################
# Name:     _handleResponseX1GetVLinkLunInfo
#
# Desc:     Handle an X1 Get VLink LUN INfo return packet.
#
#           This information is actually sent as 1 count packet followed
#           by 0 or more separate info packets.  Since CCBE is not set up
#           to handle multiple response packets we just handle the count here.
#           The async mechanism will have to handle the info packets.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1GetVLinkLunInfo
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $start;

    my %parts = disassembleX1Packet($recvPacket);

    # Get the count of storage units from the start of the packet.
    (
        $info{SUINDEX},
        $info{COUNT}
    ) = unpack("CC", $parts{DATA});

    return %info;
}


##############################################################################
# Name:     _handleResponseX1GetVLinkInfo
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

# X1_VLINK_INFO
use constant X1_VLINK_INFO =>
           "S           # dIndex
            C           # linkStatus
            C           # type
            C           # rsvd1
            L           # devCap
            a8          # srcVName
            L           # srcSerial
            C           # srcVBlock
            C           # srcVid
            C           # srcVPort
            C           # srcNumConn

            NN          # srcNodeWwn
            NN          # srcPortWwn1
            NN          # srcPortWwn2
            L           # baseSerial
            C           # baseVBlock
            C           # baseVid
            a10         # rsvd2
            a16         # product ID
            a8          # vendId
            a8          # srcName
            a4          # firmware revision
            a12";       # serial number

sub _handleResponseX1GetVLinkInfo
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %nodeWwn;
    my %portWwn1;
    my %portWwn2;
    my $rsvd;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{DINDEX},
      $info{LINKSTATUS},
      $info{TYPE},
      $rsvd,
      $info{DEVCAP},
      $info{SRCVNAME},
      $info{SRCSN},
      $info{SRCVBLK},
      $info{SRCVID},
      $info{SRCVPORT},
      $info{SRCNUMCONN},
      $nodeWwn{LO_LONG}, $nodeWwn{HI_LONG},
      $portWwn1{LO_LONG}, $portWwn1{HI_LONG},
      $portWwn2{LO_LONG}, $portWwn2{HI_LONG},
      $info{BASESN},
      $info{BASEVBLK},
      $info{BASEVID},
      $rsvd,
      $info{PRODID},
      $info{VENDID},
      $info{SRCNAME},
      $info{FWREV},
      $info{SN}) = unpack X1_VLINK_INFO, $parts{DATA};

    # Now fixup all the 64 bit  numbers
    $info{NODEWWN} = longsToBigInt(%nodeWwn);
    $info{NODEWWN_HI} = $nodeWwn{HI_LONG};
    $info{NODEWWN_LO} = $nodeWwn{LO_LONG};

    $info{PORTWWN1} = longsToBigInt(%portWwn1);
    $info{PORTWWN1_HI} = $portWwn1{HI_LONG};
    $info{PORTWWN1_LO} = $portWwn1{LO_LONG};

    $info{PORTWWN2} = longsToBigInt(%portWwn2);
    $info{PORTWWN2_HI} = $portWwn2{HI_LONG};
    $info{PORTWWN2_LO} = $portWwn2{LO_LONG};


    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetVLinkLockInfo
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

# X1_VLINK_LOCK_INFO
use constant X1_VLINK_LOCK_INFO =>
           "S           # vid
            L           # lockSN
            C           # lockVBlock
            C           # lockVid
            S           # rsvd1
            a8          # lockVDiskName
            a8";        # lockSUName


sub _handleResponseX1GetVLinkLockInfo
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $rsvd;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{VID},
      $info{LOCKSN},
      $info{LOCKVBLOCK},
      $info{LOCKVID},
      $rsvd,
      $info{VDISKNAME},
      $info{SUNAME}) = unpack X1_VLINK_LOCK_INFO, $parts{DATA};

    return %info;
}


##############################################################################
# Name:     DisplayX1VLinkSUCount
#
# Desc:     Display the X1 VLink Storage Unit Count
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1VLinkSUCount
{
    my ($self, %info) = @_;

    printf "X1 VLink Storage Unit List - count: %d\n", $info{COUNT};

}

##############################################################################
# Name:     DisplayX1VLinkedToList
#
# Desc:     Display the X1 Linked To Controller List
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1VLinkedToList
{
    my ($self, %info) = @_;

    print  "X1 VLinked To List:\n";


    for (my $i = 0; $i < $info{COUNT}; $i++)
    {
        printf "  %d\n", $info{CONTROLLERS}[$i];
    }
}


##############################################################################
# Name:     DisplayX1VLinkLunCount
#
# Desc:     Display the X1 VLink LUN Count
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1VLinkLunCount
{
    my ($self, %info) = @_;

    printf "X1 VLink LUN Info - count: %d\n", $info{COUNT};

}

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

1;

##############################################################################
# Change log:
#
# $Log$
# Revision 1.1  2005/05/04 18:53:54  RysavyR
# Initial revision
#
# Revision 1.4  2003/04/24 13:13:13  SchibillaM
# TBolt00008156: Fix for rsvd[10] in X1VLINKINFO struct.
#
# Revision 1.3  2003/04/10 20:50:01  SchibillaM
# TBolt00007915: Fix X1VLINKINFO, changes to parms for X1VLINKCREATE.
#
# Revision 1.2  2003/03/28 15:30:07  SchibillaM
# TBolt00007915: Add support for X1 VLink Info and VLink Lock Info.
#
# Revision 1.1  2003/03/27 15:18:51  SchibillaM
# TBolt00007915: Add X1 support for VLink commands - VLinked To, VLink LUN
# Info and VLink Storage Unit Info.
#
#
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
