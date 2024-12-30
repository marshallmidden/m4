# $Header$
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2002  Xiotech
# ======================================================================
# $RCSfile$
# Author: Randy Rysavy, Mark Schibilla
#
# Purpose:
#   Wrapper for all the different XIOTech X1 Map commands
#   that can be sent to the XIOtech SAN system
##############################################################################
package XIOTech::cmdMgr;

use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;

use XIOTech::error;

use XIOTech::logMgr;

use strict;


# X1_PDISK_MAP
use constant X1_PDISK_MAP       => "a128";  # pdiskBitmap

# X1_RAID_MAP
use constant X1_RAID_MAP        => "a256";  # raidBitmap

# X1_HAB_MAP
use constant X1_HAB_MAP         => "a1";    # habBitmap

# X1_BAY_MAP
use constant X1_BAY_MAP         => "a2";    # bayBitmap

# X1_VPORT_MAP
use constant X1_VPORT_MAP       => "a32";   # vportBitmap


##############################################################################
# Name:     X1PDiskGetMap
#
# Desc:     Get the PDisk Map
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
sub X1PDiskGetMap
{
    my ($self) = @_;

    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_PMAP, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_PMAP,
            \&_handleResponseX1GetPMap);

    if (%rsp)
    {
        logMsg("X1PDiskGetMap successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1PDiskGetMap failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1RAIDGetMap
#
# Desc:     Get the RAID Map
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
sub X1RAIDGetMap
{
    my ($self) = @_;

    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_RMAP, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_RMAP,
            \&_handleResponseX1GetRMap);

    if (%rsp)
    {
        logMsg("X1RAIDGetMap successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1RAIDGetMap failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1HABGetMap
#
# Desc:     Get the HAB Map
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
sub X1HABGetMap
{
    my ($self) = @_;

    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_HMAP, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_HMAP,
            \&_handleResponseX1GetHMap);

    if (%rsp)
    {
        logMsg("X1HABGetMap successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1HABGetMap failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1BayGetMap
#
# Desc:     Get the Disk Bay Map
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
sub X1BayGetMap
{
    my ($self) = @_;

    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_BAY_MAP, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_BAY_MAP,
            \&_handleResponseX1GetBayMap);

    if (%rsp)
    {
        logMsg("X1BayGetMap successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1BayGetMap failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1VPortGetMap
#
# Desc:     Get the VPort (Target) Map
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
sub X1VPortGetMap
{
    my ($self) = @_;

    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_VPORT_MAP, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_VPORT_MAP,
            \&_handleResponseX1GetVPortMap);

    if (%rsp)
    {
        logMsg("X1VPortGetMap successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1VPortGetMap failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     DisplayX1PDiskMap
#
# Desc:     Display an X1 VDisk map
#
# Input:    
#
##############################################################################
sub DisplayX1PDiskMap
{
    my (%info) = @_;

    my @pDisks = ParseBitmap($info{PDISKBITMAP});

    print "PDisk Bitmap: @pDisks\n";
}

##############################################################################
# Name:     DisplayX1RAIDMap
#
# Desc:     Display an X1 RAID map
#
# Input:    
#
##############################################################################
sub DisplayX1RAIDMap
{
    my (%info) = @_;

    my @raids = ParseBitmap($info{RAIDBITMAP});

    print "RAID Bitmap: @raids\n";
}

##############################################################################
# Name:     DisplayX1HABMap
#
# Desc:     Display an X1 HAB map
#
#
##############################################################################
sub DisplayX1HABMap
{
    my (%info) = @_;

    my @habs = ParseBitmap($info{HABBITMAP});

    print "HAB Bitmap: @habs\n";
}


##############################################################################
# Name:     DisplayX1BayMap
#
# Desc:     Display an X1 Disk Bay map
#
#
##############################################################################
sub DisplayX1BayMap
{
    my (%info) = @_;

    my @bays = ParseBitmap($info{BAYBITMAP});

    print "Disk Bay Bitmap: @bays\n";
}

##############################################################################
# Name:     DisplayX1VPortMap
#
# Desc:     Display an X1 VPort map
#
#
##############################################################################
sub DisplayX1VPortMap
{
    my (%info) = @_;

    my @vports = ParseBitmap($info{VPORTBITMAP});

    print "VPORT (Target) Bitmap: @vports\n";
}

##############################################################################
# Name:     _handleResponseX1GetPMap
#
# Desc:     Handle an X1 Get PMAP (PDisk Map) return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       
#
##############################################################################
sub _handleResponseX1GetPMap
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    $info{PDISKBITMAP} = unpack X1_PDISK_MAP, $parts{DATA};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetRMap
#
# Desc:     Handle an X1 Get RMAP (RAID Map) return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       
#
##############################################################################
sub _handleResponseX1GetRMap
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    $info{RAIDBITMAP} = unpack X1_RAID_MAP, $parts{DATA};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetHMap
#
# Desc:     Handle an X1 Get HMAP (HAB Map) return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       
#
##############################################################################
sub _handleResponseX1GetHMap
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    $info{HABBITMAP} = unpack X1_HAB_MAP, $parts{DATA};

    return %info;
}


##############################################################################
# Name:     _handleResponseX1GetBayMap
#
# Desc:     Handle an X1 Get Disk Bay Map return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       
#
##############################################################################
sub _handleResponseX1GetBayMap
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    $info{BAYBITMAP} = unpack X1_BAY_MAP, $parts{DATA};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetVPortMap
#
# Desc:     Handle an X1 Get VPort (Target) Map return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       
#
##############################################################################
sub _handleResponseX1GetVPortMap
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    $info{VPORTBITMAP} = unpack X1_VPORT_MAP, $parts{DATA};

    return %info;
}



1;

##############################################################################
# Change log:
# $Log$
# Revision 1.1  2005/05/04 18:53:54  RysavyR
# Initial revision
#
# Revision 1.3  2002/12/24 17:13:25  SchibillaM
# TBolt00006514: Add support for X1 Bay Map and VPort Map.  Reviewed by Chris.
#
# Revision 1.2  2002/12/17 20:06:45  SchibillaM
# TBolt00000000: Removed old log history.
#
# Revision 1.1  2002/12/17 20:05:45  SchibillaM
# TBolt00000000: Added support for PMAP, RMAP and HMAP.
#
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
