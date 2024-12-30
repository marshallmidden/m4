# $Header$
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Randy Rysavy
#
# Purpose:
#   Wrapper for all the different XIOTech snapshot management commands
#   that can be sent to the XIOtech SAN system
##############################################################################
package XIOTech::cmdMgr;

use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;

use XIOTech::error;

use XIOTech::logMgr;

use strict;

##############################################################################
# Name:     takeSnapshot
#
# Desc:     Take a system configuration snapshot.
#
# Input:    type of snapshot
#           description
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub takeSnapshot
{
    my ($self, $type, $description) = @_;

    logMsg("begin takeSnapshot\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 15],
                ["takeSnapshot"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $descLen = 0;
    if(defined($description)) {
        $descLen = length($description);
    }

    my %rsp;
    my $data;
    
    my $cmd = PI_SNAPSHOT_TAKE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    $data = pack("LLLLL", 0, $type, 0, 0, $descLen);
    if(defined($description)) {
        $data .= $description;
    }
    
    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

	return $self->_handleSyncResponse($seq,
		                                $packet,
		                                \&_genericResponsePacket);
}


##############################################################################
# Name:     loadSnapshot
#
# Desc:     Load a system configuration snapshot.
#
# Input:    directory index of snapshot
#           flags indicating which fids to reload
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub loadSnapshot
{
    my ($self, $index, $flags) = @_;

    logMsg("begin loadSnapshot\n");

    # verify parameters
    my $args = [['i'],
                ['d', 100, 0xFFFFFFFF],
                ['d', 1, 0xffffffff],
                ["loadSnapshot"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my %rsp;
    my $data;
    
    my $cmd = PI_SNAPSHOT_LOAD_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    $data = pack("LLLLL", $index, 0, $flags, 0, 0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

	return $self->_handleSyncResponse($seq,
		                                $packet,
		                                \&_genericResponsePacket);
}


##############################################################################
# Name:     changeSnapshot
#
# Desc:     Change a system configuration snapshot.
#
# Input:    directory index of snapshot
#           status (READ_ONLY, DELETE etc)
#           description
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub changeSnapshot
{
    my ($self, $index, $status, $description) = @_;

    logMsg("begin changeSnapshot\n");

    # verify parameters
    my $args = [['i'],
                ['d', 100, 0xFFFFFFFF],
                ['d', 0, 5],
                ["changeSnapshot"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $descLen = 0;
    if(defined($description)) {
        $descLen = length($description);
    }

    my %rsp;
    my $data;
    
    my $cmd = PI_SNAPSHOT_CHANGE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    $data = pack("LLLLL", $index, 0, 0, $status, $descLen);
    if(defined($description)) {
        $data .= $description;
    }

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

	return $self->_handleSyncResponse($seq,
		                                $packet,
		                                \&_genericResponsePacket);
}


##############################################################################
# Name:     readdirSnapshot
#
# Desc:     Read up the snapshot directory.
#
# Input:    none
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#       ERROR_CODE              Associated error code 
#       ENTRY (array)           A hash for each entry containing:
#           MASTER_CFG_FID
#           CTRL_MAP_FID
#           BE_NVRAM_FID
#           SEQ_NUMBER
#           TIME_STR
#           CCB_RT
#           CCB_BOOT
#           FE_RT
#           FE_BOOT
#           FE_DIAG
#           BE_RT
#           BE_BOOT
#           BE_DIAG
#           TYPE
#           STATUS
#           FLAGS
#           DESCRIPTION
#           
##############################################################################
sub readdirSnapshot
{
    my ($self) = @_;

    logMsg("begin readdirSnapshot\n");

    # verify parameters
    my $args = [['i'],
                ["readdirSnapshot"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my %rsp;
    
    my $cmd = PI_SNAPSHOT_READDIR_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

	return $self->_handleSyncResponse($seq,
		                                $packet,
		                                \&_readdirSnapshotResponsePacket);
}

##############################################################################
#
#   All commands that start with a _ should not be used and are considered
#   private.
#
##############################################################################

##############################################################################
# Name:  _readdirSnapshotResponsePacket
#
# Desc: Parses the system information packet and places the information in a
#       hash
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: Empty hash on error, else a hash with the following elements:
#
#       STATUS                  Status of the command
#       ERROR_CODE              Associated error code 
#       ENTRY (array)           A hash for each entry containing:
#           MASTER_CFG_FID
#           CTRL_MAP_FID
#           BE_NVRAM_FID
#           SEQ_NUMBER
#           TIME_STR
#           CCB_RT
#           CCB_BOOT
#           FE_RT
#           FE_BOOT
#           FE_DIAG
#           BE_RT
#           BE_BOOT
#           BE_DIAG
#           TYPE
#           STATUS
#           FLAGS
#           DESCRIPTION
#
##############################################################################
use constant TIMESTAMP_t =>
            "H2             # year1
             H2             # year2
             H2             # month
             H2             # date
             H2             # day
             H2             # hours
             H2             # minutes
             H2             # seconds
             L";            # system seconds

sub _readdirSnapshotResponsePacket
{
    my ($self, $seq, $recvPacket) = @_;

    my %snapDir;

    if (!(defined($recvPacket)))
    {
        $recvPacket = $self->_receivePacketSync($seq);
    }

    if (defined($recvPacket))
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $snapDir{STATUS} = $parts{STATUS};
        $snapDir{ERROR_CODE} = $parts{ERROR_CODE};

        # Skip most of the directory header
        my $data = substr($parts{DATA}, 52);
        
        my $i;
        
        my $eyecatcher;
        my $year1;
        my $year2;
        my $ccbRT;
        my $ccbBoot;
        my $feRT;
        my $feBoot;
        my $feDiag;
        my $beRT;
        my $beBoot;
        my $beDiag;
        my $flags;
        my $status;
        my $reserved;
        my $description;
        my $crc;

        for ($i = 0; $i < 32; $i++) {
            (
             $eyecatcher,
             $snapDir{$i}{MASTER_CFG_FID},
             $snapDir{$i}{CTRL_MAP_FID},
             $snapDir{$i}{BE_NVRAM_FID},
             $snapDir{$i}{SEQ_NUMBER},
             $year1,
             $year2,
             $snapDir{$i}{TS_MONTH},
             $snapDir{$i}{TS_DATE},
             $snapDir{$i}{TS_DAY},
             $snapDir{$i}{TS_HOURS},
             $snapDir{$i}{TS_MINUTES},
             $snapDir{$i}{TS_SECONDS},
             $snapDir{$i}{TS_UPTIME},
             $ccbRT,
             $ccbBoot,
             $feRT,
             $feBoot,
             $feDiag,
             $beRT,
             $beBoot,
             $beDiag,
             $snapDir{$i}{TYPE},
             $status,
             $flags,
             $reserved,
             $snapDir{$i}{DESCRIPTION},
             $crc
            ) = unpack("a8 L L L L".
            TIMESTAMP_t.
            "a20 a20 a20 a20 a20 a20 a20 a20 L L L a40 A196 L", $data);
            $data = substr($data, 448);
            
            $snapDir{$i}{TS_YEAR} = $year2 . $year1;
            $snapDir{$i}{FLAGS} = "";
            $snapDir{$i}{FLAGS} .= $flags & 0x01 ? "M-" : "";
            $snapDir{$i}{FLAGS} .= $flags & 0x02 ? "C-" : "";
            $snapDir{$i}{FLAGS} .= $flags & 0x04 ? "N-" : "";
            chop $snapDir{$i}{FLAGS} if length $snapDir{$i}{FLAGS};

            $snapDir{$i}{STATUS} = "OPEN   " if $status == 1;
            $snapDir{$i}{STATUS} = "DEL    " if $status == 2;
            $snapDir{$i}{STATUS} = "IN USE " if $status == 3;
            $snapDir{$i}{STATUS} = "SSV ERR" if $status == 4;
            $snapDir{$i}{STATUS} = "KEEP   " if $status == 5;
            $snapDir{$i}{STATUS} = "CRC ERR" if $status == 6;
        }
    }

    return %snapDir;
}

##############################################################################
# Name: displaySnapshot
#
# Desc: Print the snapshot entry information
#
# In: 
#
##############################################################################
sub displaySnapshot
{
    my ($self, $index, %snapDir) = @_;
    my $i;
    
    logMsg("displaySnapshot...begin\n");

    print "\nSnapshot Directory\n\n";
    print "Saved FIDS:\n";
    print "'M' = Master Config\n";
    print "'C' = Controller Map\n";
    print "'N' = BE NVRAM\n\n";
    print "Idx   Seq#          Date/Time      FIDS   Status   Description\n\n";
    for ($i = 0; $i < 32; $i++) {
        printf(" %2u %6u    %s/%s/%s %s:%s:%s  %-5s  %-5s  %s\n",
        $i,
        $snapDir{$i}{SEQ_NUMBER},
        $snapDir{$i}{TS_MONTH},
        $snapDir{$i}{TS_DATE},
        $snapDir{$i}{TS_YEAR},
        $snapDir{$i}{TS_HOURS},
        $snapDir{$i}{TS_MINUTES},
        $snapDir{$i}{TS_SECONDS},
        $snapDir{$i}{FLAGS},
        $snapDir{$i}{STATUS},
        $snapDir{$i}{DESCRIPTION});
    }
}


1;

##############################################################################
# Change log:
# $Log$
# Revision 1.2  2006/07/17 20:38:32  RustadM
# TBolt00014770
# Move 750 branch onto main.
#
# Revision 1.1.1.1.30.1  2006/04/26 09:06:20  BharadwajS
# CCBE changes for PI Versioning
#
# Revision 1.1.1.1  2005/05/04 18:53:54  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.6  2005/04/22 20:46:53  RysavyR
# TBolt00012443: Cleanup of old MPX commands and addition of a -S option FIDREAD and a -N option to FIDWRITE. Rev by Holty.
#
# Revision 1.5  2004/07/28 14:46:51  RysavyR
# TBolt00010818: Enhanced the "Config Journaling" user interface both from the
# serial console and from the CCBE. Rev by Tim Swatosh.
#
# Revision 1.4  2003/02/11 21:03:29  RysavyR
# TBolt00007098:  Other miscellaneous (minor) config journal/snapshot bug fixes.
# Rev by TimSw.
#
# Revision 1.3  2002/12/10 17:24:47  RysavyR
# TBolt00003598: Add additional config journaling support. Rev by TimSw
#
# Revision 1.2  2002/09/20 16:38:42  RysavyR
# TBolt00003598: Additional Snapshot/Journaling support.
#
# Revision 1.1  2002/06/04 19:18:07  RysavyR
# TBolt00003598: Added the first pass at configuration snapshotting.
#
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
