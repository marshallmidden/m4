# $Id: cmVDisk.pm 145021 2010-08-03 14:16:38Z m4 $
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2001  Xiotech
# ======================================================================
#
# Purpose:
#   Wrapper for all the different XIOTech virtual disk commands that
#   can be sent to the XIOtech SAN system
##############################################################################
package XIOTech::cmdMgr;

use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;

use XIOTech::bigNums;
use XIOTech::error;

use XIOTech::logMgr;

use strict;

##############################################################################
# Name:     virtualDisks
#
# Desc:     Retrieves virtual disk information for all virtual disks.
#
# In:       NONE
#
# Returns:
##############################################################################
sub virtualDisks
{
    my ($self) = @_;
    my $packetVersion = VERSION_2;
    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["virtualDisks"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $version = 0;
    if (defined($self) && $self != 0) {
        $version = $self->{Controller_FW_Version};
    }
    my $cmd = PI_VDISKS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    if ($version >= 840)
    {
        $packetVersion = VERSION_3;
    }

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, $packetVersion);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_virtualDisksPacket);
}

##############################################################################
# Name:     virtualDisksCache
#
# Desc:     Retrieves virtual disk information for all virtual disks from cache.
#
# In:       NONE
#
# Returns:
##############################################################################
sub virtualDisksCache
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["virtualDisksCache"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VDISKS_FROM_CACHE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_2);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_virtualDisksPacket);
}


##############################################################################
# Name:     virtualDiskControl
#
# Desc:     Control a virtual disk.
#
# Input:    None
#
# Returns:  
##############################################################################
sub virtualDiskControl
{
    my ($self,
        $op,
        $svid,
        $dvid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0x00, 0x0E],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFFFFFFFF],
                ["virtualDiskControl"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VDISK_CONTROL_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCCSSSS",
                    $op,
                    0,0,0,
                    $svid,
                    0,
                    $dvid,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}

##############################################################################
# Name:     quickPauseBreakMirrorStart
#
# Desc:     quickPause break mirror start .
#
# Input:    None
#
# Returns:  
##############################################################################
sub quickPauseBreakMirrorStart
{
    my ($self,
        $count) = @_;

    logMsg("begin\n");

    # verify parameters
#    my $args = [['i'],
#                ['d', 0x00, 0x0E],
#                ['d', 0, 0xFFFFFFFF],
#                ['d', 0, 0xFFFFFFFF],
#                ["virtualDiskControl"]];

#    my %vp = verifyParameters(\@_, $args);
#    if (%vp)
#    {
#        return %vp;
#    }

    my $cmd = PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_START_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $count,
                    0,0,0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}

##############################################################################
# Name:     quickPauseBreakMirrorSequence
#
# Desc:     Gives a sequence command for pause break mirror.
#
# Input:    None
#
# Returns:  
##############################################################################
sub quickPauseBreakMirrorSequence
{
    my ($self,
        $op,
        $dvid) = @_;

    logMsg("begin\n");
    my $cmd = PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_SEQUENCE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CSC",
                    $op,
                    $dvid,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}

##############################################################################
# Name:     quickPauseBreakMirrorExecute
#
# Desc:     gives execute command quickPause break mirror 
#
# Input:    None
#
# Returns:  
##############################################################################
sub quickPauseBreakMirrorExecute
{
    my ($self,
        $action) = @_;

    logMsg("begin\n");

    # verify parameters
#    my $args = [['i'],
#                ['d', 0x00, 0x0E],
#                ['d', 0, 0xFFFFFFFF],
#                ['d', 0, 0xFFFFFFFF],
#                ["virtualDiskControl"]];

#    my %vp = verifyParameters(\@_, $args);
#    if (%vp)
#    {
#        return %vp;
#    }

    my $cmd = PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_EXECUTE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $action,
                    0,0,0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}








##############################################################################
# Name:     batchSnapshotCmdStart
#
# Desc:     Batch Snapshot cmd start .
#
# Input:    None
#
# Returns:  
##############################################################################
sub batchSnapshotCmdStart
{
    my ($self,
        $count) = @_;

    logMsg("begin\n");

    my $cmd = PI_BATCH_SNAPSHOT_START_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $count,
                    0,0,0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}

##############################################################################
# Name:     batchSnapshotCmdSequence
#
# Desc:     Gives a sequence command for Batch Snapshot.
#
# Input:    None
#
# Returns:  
##############################################################################
sub batchSnapshotCmdSequence
{
    my ($self,
        $svid,
        $dvid) = @_;

    logMsg("begin\n");
    my $cmd = PI_BATCH_SNAPSHOT_SEQUENCE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SSCCCC",
                    $svid,
                    $dvid,
                    0,0,0,0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}

##############################################################################
# Name:        batchSnapshotCmdExecute
#
# Desc:     execute/cacnel command for the list of batch snapshots 
#
# Input:    None
#
# Returns:  
##############################################################################
sub batchSnapshotCmdExecute
{
    my ($self,
        $action) = @_;

    logMsg("begin\n");

    my $cmd = PI_BATCH_SNAPSHOT_EXECUTE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $action,
                    0,0,0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}




##############################################################################
# Name:     virtualDiskCount
#
# Desc:     Retrieves the number of virtual disks.
#
# Input:    None
#
# Returns:  Number of virtual disks or UNDEF if an error occurred.
##############################################################################
sub virtualDiskCount
{
    my ($self) = @_;
    return $self->getObjectCount(PI_VDISK_COUNT_CMD);
}


#ifdef ENGINEERING
##############################################################################
# Name:     virtualDiskCreate
#
# Desc:     Create a virtual disk.
#
# Input:    None
#
# Returns:  
##############################################################################
sub virtualDiskCreate
{
    my ($self,
        $capacity,
        $physicalDisks,
        $rtype,
        $stripe,
        $depth,
        $parity,
        $vid,
        $maxraids,
        $threshold,
        $flags,
        $minPD) = @_;

    my $bNewArgs = 0;
    my @newArgs;
    
    logMsg("begin\n");

    if (!defined($vid))
    {
        $vid = $self->_virtualDiskGetNextVid();
        
        if (defined($vid))
        {
            $bNewArgs = 1;
        }
        else
        {
            print "Could not generate a valid VID!!\n";
        }
    }

    if (!defined($maxraids))
    {
        $maxraids = 4;
        $bNewArgs = 1;
    }
    
    if (!defined($threshold))
    {
        $threshold = 10;
        $bNewArgs = 1;
    }
    
    if (!defined($flags))
    {
        $flags = 0;
        $bNewArgs = 1;
    }
    
    if (!defined($minPD))
    {
        $minPD = 0;
        $bNewArgs = 1;
    }
    
    if ($bNewArgs == 1)
    {
        push @newArgs, $self;
        push @newArgs, $capacity;
        push @newArgs, $physicalDisks;
        push @newArgs, $rtype;
        push @newArgs, $stripe;
        push @newArgs, $depth;
        push @newArgs, $parity;
        push @newArgs, $vid;
        push @newArgs, $maxraids;
        push @newArgs, $threshold;
        push @newArgs, $flags;
        push @newArgs, $minPD;
        
        @_ =  @newArgs;
    }

    # verify parameters
    my $args = [['i'],
                ['s'],
                ['a'],
                ['d', 0, 4],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['i'],
                ['i'],
                ['i'],
                ['i'],
                ['i'],
                ["virtualDiskCreate"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my %capacityBytes = bigIntTolongs($capacity);
    my @disks = @$physicalDisks;

    my $cmd = PI_VDISK_CREATE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCSSSLLCCSSCC",
                    $rtype,
                    VDISK_CREATE_OP_CREATE,
                    $vid,
                    scalar(@disks),
                    $stripe,
                    $capacityBytes{LO_LONG},
                    $capacityBytes{HI_LONG},
                    $depth,
                    $parity,
                    $maxraids,
                    $threshold,
                    $flags,
                    $minPD);

    for (my $i = 0; $i < scalar(@disks); ++$i)
    {
        $data .= pack("S", $disks[$i]);
    }

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_virtualDiskCreatePacket);
}
#endif

##############################################################################
# Name:     virtualDiskSetPriority
#
# Desc:     Set Priorities for disks.
#
# Input:    None
#
# Returns:  
##############################################################################
sub virtualDiskSetPriority
{
    my ($self,
        $numvals,
        $respcnt,
        $opt,
        @vpripairs) = @_;

    logMsg("begin\n");

    # verify parameters

    my $cmd = PI_VDISK_SET_PRIORITY_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data;

    $data = pack("S",$numvals);
    $data .= pack("S",$respcnt);
    $data .= pack("S", $opt);
    $data .= pack("S", 0);                 # Reserved

    my $j = 0;

    for (my $i = 0; $i < $numvals; ++$i)
    {
        $data .= pack("S", $vpripairs[$j]);
        $j++;
        $data .= pack("C", $vpripairs[$j]);
        $data .= pack("C", 0);              # Reserved Byte 
        $j++;
    }

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_virtualDiskSetPriorityPacket);
}

##############################################################################
# Name:     vdiskPriorityEnable
#
# Desc:     Enable/Disable VDisk Priority feature.
#
# Input:    None
#
# Returns:  
##############################################################################
sub vdiskPriorityEnable
{
    my ($self, $mode) = @_;

    logMsg("begin\n");

    my $cmd = PI_VCG_SET_VDISK_PRIORITY_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC", $mode,0,0,0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_vdiskPriorityEnablePacket);
}

##############################################################################
# Name:     vdiskPRGet
#
# Desc:     Get Vdisk persistent reserve information.
#
# Input:    None
#
# Returns:  
##############################################################################
sub vdiskPRGet
{
    my ($self, $vid) = @_;

    logMsg("begin\n");

    my $cmd = PI_VDISK_PR_GET_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $vid,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_vdiskPRGetPacket);
}

##############################################################################
# Name:     vdiskPRClr
#
# Desc:     Clear Vdisk persistent reserve information.
#
# Input:    None
#
# Returns:  
##############################################################################
sub vdiskPRClr
{
    my ($self, $vid) = @_;

    logMsg("begin\n");

    my $cmd = PI_VDISK_PR_CLR_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $vid,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}


#ifdef ENGINEERING
##############################################################################
# Name:     virtualDiskExpand
#
# Desc:     Expand a virtual disk.
#
# Input:    None
#
# Returns:  
##############################################################################
sub virtualDiskExpand
{
    my ($self,
        $vid,
        $capacity,
        $physicalDisks,
        $rtype,
        $stripe,
        $depth,
        $parity,
        $maxraids,
        $threshold,
        $flags,
        $minPD) = @_;

    my $bNewArgs = 0;
    my @newArgs;

    logMsg("begin\n");

    if (!defined($maxraids))
    {
        $maxraids = 4;
        $bNewArgs = 1;
    }
    
    if (!defined($threshold))
    {
        $threshold = 10;
        $bNewArgs = 1;
    }
    
    if (!defined($flags))
    {
        $flags = 0;
        $bNewArgs = 1;
    }
    
    if (!defined($minPD))
    {
        $minPD = 0;
        $bNewArgs = 1;
    }
    
    if ($bNewArgs == 1)
    {
        push @newArgs, $self;
        push @newArgs, $vid;
        push @newArgs, $capacity;
        push @newArgs, $physicalDisks;
        push @newArgs, $rtype;
        push @newArgs, $stripe;
        push @newArgs, $depth;
        push @newArgs, $parity;
        push @newArgs, $maxraids;
        push @newArgs, $threshold;
        push @newArgs, $flags;
        push @newArgs, $minPD;
        
        @_ =  @newArgs;
    }

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['s'],
                ['a'],
                ['d', 0, 4],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['i'],
                ['i'],
                ['i'],
                ['i'],
                ["virtualDiskExpand"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my %capacityBytes = bigIntTolongs($capacity);
    my @disks = @$physicalDisks;

    my $cmd = PI_VDISK_EXPAND_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCSSSLLCCSSCC",
                    $rtype,
                    VDISK_CREATE_OP_EXPAND,
                    $vid,
                    scalar(@disks),
                    $stripe,
                    $capacityBytes{LO_LONG},
                    $capacityBytes{HI_LONG},
                    $depth,
                    $parity,
                    $maxraids,
                    $threshold,
                    $flags,
                    $minPD);

    for (my $i = 0; $i < scalar(@disks); ++$i)
    {
        $data .= pack("S", $disks[$i]);
    }

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_virtualDiskCreatePacket);
}
#endif

##############################################################################
# Name:     virtualDiskPrepare
#
# Desc:     Prepare a virtual disk.
#
# Input:    None
#
# Returns:  
##############################################################################
sub virtualDiskPrepare
{
    my ($self,
        $capacity,
        $physicalDisks,
        $rtype,
        $stripe,
        $depth,
        $parity,
        $maxraids,
        $threshold,
        $flags,
        $minPD) = @_;

    my $bNewArgs = 0;
    my @newArgs;

    logMsg("begin\n");

    if (!defined($maxraids))
    {
        $maxraids = 4;
        $bNewArgs = 1;
    }
    
    if (!defined($threshold))
    {
        $threshold = 10;
        $bNewArgs = 1;
    }
    
    if (!defined($flags))
    {
        $flags = 0;
        $bNewArgs = 1;
    }
    
    if (!defined($minPD))
    {
        $minPD = 0;
        $bNewArgs = 1;
    }
    
    if ($bNewArgs == 1)
    {
        push @newArgs, $self;
        push @newArgs, $capacity;
        push @newArgs, $physicalDisks;
        push @newArgs, $rtype;
        push @newArgs, $stripe;
        push @newArgs, $depth;
        push @newArgs, $parity;
        push @newArgs, $maxraids;
        push @newArgs, $threshold;
        push @newArgs, $flags;
        push @newArgs, $minPD;
        
        @_ =  @newArgs;
    }

    # verify parameters
    my $args = [['i'],
                ['s'],
                ['a'],
                ['d', 0, 4],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['i'],
                ['i'],
                ['i'],
                ['i'],
                ["virtualDiskPrepare"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my %capacityBytes = bigIntTolongs($capacity);
    my @disks = @$physicalDisks;

    my $cmd = PI_VDISK_PREPARE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCSSSLLCCSSCC",
                    $rtype,
                    VDISK_CREATE_OP_PREPARE,
                    0,
                    scalar(@disks),
                    $stripe,
                    $capacityBytes{LO_LONG},
                    $capacityBytes{HI_LONG},
                    $depth,
                    $parity,
                    $maxraids,
                    $threshold,
                    $flags,
                    $minPD);

    for (my $i = 0; $i < scalar(@disks); ++$i)
    {
        $data .= pack("S", $disks[$i]);
    }

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_virtualDiskCreatePacket);
}

##############################################################################
# Name:     virtualDiskDelete
#
# Desc:     Delete a virtual disk
#
# In:       ID of a virtual disk
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub virtualDiskDelete
{
    my ($self, $vid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["virtualDiskDelete"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VDISK_DELETE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $vid,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}

##############################################################################
# Name: virtualDiskInfo
#
# Desc: Get information about a physical disk
#
# In:       ID of a virtual disk
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub virtualDiskInfo
{
    my ($self, $vid) = @_;

    my $packetVersion = VERSION_2;
    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["virtualDiskInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VDISK_INFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $vid,
                    0);

    my $version = 0;
    if (defined($self) && $self != 0) {
        $version = $self->{Controller_FW_Version};
    }

    if ($version >= 840)
    {
        $packetVersion = VERSION_3;
    }

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, $packetVersion);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_virtualDiskInfoPacket);
}

##############################################################################
# Name: vdiskBayRedundant
#
# Desc: Gets information about the redundancy of the given virtual disk 
#
# In:       ID of a virtual disk
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub vdiskBayRedundant
{
    my ($self, $vid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["virtualDiskInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VDISK_BAY_REDUNDANT_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $vid,
                    0);
    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_bayRedundantResponsePkt);
}

##############################################################################
# Name:     virtualDiskInit
#
# Desc:     Initialize a virtual disk.
#
# In:       ID of a virtual disk
#
# Returns:
##############################################################################
sub virtualDiskInit
{
    my ($self, $vid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["virtualDiskInit"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_RAID_INIT_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $vid,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}

##############################################################################
# Name:     virtualDiskList
#
# Desc:     Retrieves an array containing the identifiers of the virtual disks.
#
# Input:    None
#
# Returns:
##############################################################################
sub virtualDiskList
{
    my ($self) = @_;
    return $self->getObjectList(PI_VDISK_LIST_CMD);
}

##############################################################################
# Name: virtualDiskOwner
#
# Desc: Get information about a virtual disks owners
#
# In:       ID of a virtual disk
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub virtualDiskOwner
{
    my ($self, $vid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["virtualDiskOwner"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VDISK_OWNER_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $vid,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_virtualDiskOwnerPacket);
}

##############################################################################
# Name:     virtualDiskSetCache
#
# Desc:     Set caching mode for a virtual disk.
#
# INPUT:    ID      = identifier of a virtual disk
#           MODE    = Caching Mode
#                       0x00 = Caching disabled
#                       0x01 = Caching enabled
#
# OUTPUT:   Status Response Packet Hash
##############################################################################
sub virtualDiskSetCache
{
    my ($self, $vid, $mode) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0x00, 0x01],
                ["virtualDiskSetCache"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my %vdiskinfo = $self->virtualDiskInfo($vid);

    if (%vdiskinfo && $vdiskinfo{STATUS} == PI_GOOD)
    {
        # If the mode is 0 (cache disabled) then mask off
        # the high bit from the attribute and set the
        # mode to that value.  If the mode is not 0 (cache
        # enabled) then set the high bit.
        if ($mode == 0)
        {
            $mode = ($vdiskinfo{ATTR} & (~0x0100));
        }
        else
        {
            $mode = ($vdiskinfo{ATTR} | 0x0100);
        }

        return $self->virtualDiskSetAttributes($vid, $mode);
    }
    else
    {
        return %vdiskinfo;
    }
}

##############################################################################
# Name:     virtualDiskSetAttributes
#
# Desc:     Set attributes a virtual disk.
#
# INPUT:    ID      = identifier of a virtual disk
#           MODE    = Caching Mode
#                       0x00 = Caching disabled
#                       0x01 = Caching enabled
#
# OUTPUT:   Status Response Packet Hash
##############################################################################
sub virtualDiskSetAttributes
{
    my ($self, $vid, $mode) = @_;
    
    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ["virtualDiskSetAttributes"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VDISK_SET_ATTRIBUTE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("Ca3SS",
                    0,
                    0,
                    $vid,
                    $mode);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}

##############################################################################
# Name: displayVirtualDisks
#
# Desc: Print the virtual disks
#
# In:   Virtual Disks Information Hash
##############################################################################
sub displayVirtualDisks
{
    my ($self, $dsptype, %info) = @_;

    logMsg("begin\n");

    my $msg = "";
    my $opState;
    my $permFlags;
    my $timeStr;
    my $wtimeStr;
    my $btimeStr;
    
    if (uc($dsptype) eq "STD")
    {
    $msg .= sprintf( "Virtual Disks ($info{COUNT} disks):\n");
    $msg .= sprintf(  "\n");

    $msg .= "                                                        DEFERED  SEC COPY\n";
        $msg .= " VID   DEVSTAT     CAPACITY     MIRROR   ATTR   RAIDCNT  RAIDCNT   % COMP    PRIORITY  OWNER   NAME\n";
        $msg .= " ----  -------  --------------  ------  ------  -------  -------  --------   --------  ------  -------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf( " %4hu     0x%02x  %14s    0x%2.2x  0x%4.4x  %7d  %7d   %7d    %7d    0x%2.2x  %-16s\n", 
                    $info{VDISKS}[$i]{VID},
                    $info{VDISKS}[$i]{DEVSTAT},
                    $info{VDISKS}[$i]{CAPACITY},
                    $info{VDISKS}[$i]{MIRROR},
                    $info{VDISKS}[$i]{ATTR},
                    $info{VDISKS}[$i]{RAIDCNT},
                    $info{VDISKS}[$i]{DRAIDCNT},
                    $info{VDISKS}[$i]{SCPCOMP},
                    $info{VDISKS}[$i]{PRIORITY},
                    $info{VDISKS}[$i]{OWNER},
                    $info{VDISKS}[$i]{NAME});
        }
    }
    elsif (uc($dsptype) eq "TIME")
    {
        my $version = 0;
        if (defined($self) && $self != 0) {
            $version = $self->{Controller_FW_Version};
        }

        $msg .= sprintf( "Virtual Disks ($info{COUNT} disks):\n");
        $msg .= sprintf(  "\n");

        if ($version >= 840)
        {
            $msg .= " VID            CREATE TIME               LAST ACCESS TIME               BREAK TIME\n";
            $msg .= " ----      ------------------------   -------------------------      ------------------\n";
        }
        else
        {
            $msg .= " VID            CREATE TIME               LAST ACCESS TIME\n";
            $msg .= " ----      ------------------------   -------------------------\n";
        }
        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            if ($info{VDISKS}[$i]{TIMESTAMP} != 0)
            {
                $timeStr   = scalar (gmtime ($info{VDISKS}[$i]{TIMESTAMP}));
            }
            else
            {
                $timeStr = "Unknown";
            }
            if ($info{VDISKS}[$i]{LACCESS} != 0)
            {
                $wtimeStr   = scalar (gmtime ($info{VDISKS}[$i]{LACCESS}));
            }
            else
            {
                $wtimeStr = "Unknown";
            }

            if ($version >= 840)
            {
                if (defined($info{VDISKS}[$i]{BREAKTIME}) && $info{VDISKS}[$i]{BREAKTIME} != 0)
                {
                    $btimeStr   = scalar (gmtime ($info{VDISKS}[$i]{BREAKTIME}));
                }
                else
                {
                    $btimeStr = "Unknown";
                }
            }

            if ($version >= 840)
            {
                $msg .= sprintf( " %4hu      %-23s             %-23s  %-20s\n",
                                 $info{VDISKS}[$i]{VID},
                                 $timeStr,
                                 $wtimeStr,
                                 $btimeStr);
            }
            else
            {
                $msg .= sprintf( " %4hu      %-23s             %-23s\n",
                                 $info{VDISKS}[$i]{VID},
                                 $timeStr,
                                 $wtimeStr);
            }
        }
    }
    elsif (uc($dsptype) eq "STATS")
    {
        $msg .= sprintf( "Virtual Disks ($info{COUNT} disks):\n");
        $msg .= sprintf(  "\n");

        $msg .= "                         AVERAGE         AVERAGE \n";
        $msg .= "                         REQ/SEC         BLOCKS/SEC\n";
        $msg .= " VID   DEVSTAT  OWNER    OVER LAST HOUR  OVER LAST HOUR\n";
        $msg .= " ----  -------  -----   ---------------  --------------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf( " %4hu     0x%02x  0x%2.2x  %10u    %10u\n",
                    $info{VDISKS}[$i]{VID},
                    $info{VDISKS}[$i]{DEVSTAT},
                    $info{VDISKS}[$i]{OWNER},
                    $info{VDISKS}[$i]{AVERAGEIO},
                    $info{VDISKS}[$i]{AVERAGESC});
        }
    }
    elsif (uc($dsptype) eq "GEOFLAGS")
    {
        $msg .= sprintf( "Virtual Disks ($info{COUNT} disks):\n");
        $msg .= sprintf(  "\n");

        $msg .= " VID   DEVSTAT   MIRROR    ATTR   OWNER    OP_STATE  PERM_FLAGS  TEMP_FLAGS\n";
        $msg .= " ----  -------   ------   ------  -------  --------  ----------  ----------\n"; 

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $opState = $info{VDISKS}[$i]{GR_B1} & 7;
            $permFlags = ($info{VDISKS}[$i]{GR_B1} >> 3) & 15;  

            $msg .= sprintf( " %4hu     0x%02x     0x%2.2x   0x%4.4x   0x%2.2x  %8u %10u %10u\n",
                    $info{VDISKS}[$i]{VID},
                    $info{VDISKS}[$i]{DEVSTAT},
                    $info{VDISKS}[$i]{MIRROR},
                    $info{VDISKS}[$i]{ATTR},
                    $info{VDISKS}[$i]{OWNER},
                    $opState,
                    $permFlags,
                    $info{VDISKS}[$i]{GR_B2});
        }
    }

    $msg .= sprintf( "\n");

    return $msg;
}

##############################################################################
# Name: displayVirtualDiskInfo
#
# Desc: Print the virtual disk information
#
# In:   Virtual Disk Information Hash
##############################################################################
sub displayVirtualDiskInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $i;
    my $msg = "";
    my $totalRaids = $info{RAIDCNT} + $info{DRAIDCNT};

    my $opState = $info{GR_B1} & 7;
    my $permFlags = $info{GR_B1} >> 3; # Upper 5 bits
    my $timeStr;
    my $wtimeStr;
    my $btimeStr;

    my $temp = $info{TIMESTAMP};

    if ($temp != 0)
    {
        $timeStr = scalar(gmtime($info{TIMESTAMP}));
    }
    else
    {
        $timeStr = "Unknown";
    }

    $temp = $info{LACCESS};

    if ($temp != 0)
    {
        $wtimeStr = scalar(gmtime($info{LACCESS}));
    }
    else
    {
        $wtimeStr = "Unknown";
    }

    my $version = 0;
    if (defined($self) && $self != 0) {
        $version = $self->{Controller_FW_Version};
    }

    if ($version >= 840)
    {
        $temp = $info{BREAKTIME};

        if ($temp != 0)
        {
            $btimeStr = scalar(gmtime($info{BREAKTIME}));
        }
        else
        {
            $btimeStr = "Unknown";
        }
    }

    $msg .= sprintf( "Virtual Disk Information:\n");
    $msg .= sprintf( "  STATUS:               0x%x\n", $info{STATUS_MRP});
    $msg .= sprintf( "  LEN:                  %lu\n", $info{LEN});
    $msg .= sprintf( "  VID:                  %hu\n", $info{VID});
    $msg .= sprintf( "  MIRROR:               %hu\n", $info{MIRROR});
    $msg .= sprintf( "  DEVSTAT:              0x%02x\n", $info{DEVSTAT});
    $msg .= sprintf( "  SCORVID:              %hu\n", $info{SCORVID});
    $msg .= sprintf( "  SCPCOMP:              %hu\n", $info{SCPCOMP});
    $msg .= sprintf( "  RAIDCNT:              %hu\n", $info{RAIDCNT});
    $msg .= sprintf( "  CAPACITY:             $info{CAPACITY}\n");
    $msg .= sprintf( "  ERROR:                %lu\n", $info{ERROR});
    $msg .= sprintf( "  QD:                   %lu\n", $info{QD});
    $msg .= sprintf( "  RPS:                  %lu\n", $info{RPS});
    $msg .= sprintf( "  AVGSC:                %lu\n", $info{AVGSC});
    $msg .= sprintf( "  RREQ:                 $info{RREQ}\n");
    $msg .= sprintf( "  WREQ:                 $info{WREQ}\n");
    $msg .= sprintf( "  ATTR:                 0x%4.4x\n", $info{ATTR});
    $msg .= sprintf( "  DRAIDCNT:             %hu\n", $info{DRAIDCNT});
    $msg .= sprintf( "  OWNER:                0x%2.2x\n", $info{OWNER});
    $msg .= sprintf( "  PRIORITY:             %hu\n", $info{PRIORITY});
    $msg .= sprintf( "  SPRC:                 %lu\n", $info{SPRC});
    $msg .= sprintf( "  SPSC:                 %lu\n", $info{SPSC});
    $msg .= sprintf( "  SCDHEAD:              0x%8.8x\n", $info{SCHEAD});
    $msg .= sprintf( "  SCDTAIL:              0x%8.8x\n", $info{SCTAIL});
    $msg .= sprintf( "  DCD:                  0x%8.8x\n", $info{CPSCMT});
    $msg .= sprintf( "  VLINKS:               %lu\n", $info{VLINKS});
    $msg .= sprintf( "  NAME:                 $info{NAME}\n");
    $msg .= sprintf( "  CREATE TIME:          %s\n", $timeStr);
    $msg .= sprintf( "  LAST ACCESS TIME:     %s\n", $wtimeStr);
    $msg .= sprintf( "  OP_STATE:             %3u\n", $opState);
    $msg .= sprintf( "  PERM_FLAGS:           %3u\n", $permFlags);
    $msg .= sprintf( "  TEMP_FLAGS:           %3u\n", $info{GR_B2});
    $msg .= sprintf( "  AVERAGE IO:           %lu\n", $info{AVERAGEIO});
    $msg .= sprintf( "  AVERAGE MB:           %lu\n", $info{AVERAGESC});
    if ($version >= 840)
    {
        $msg .= sprintf( "  MIRROR BREAK TIME:    %s\n", $btimeStr);
    }
    $msg .= sprintf(  "  Raid IDs:\n");
    for ($i = 0; $i < $totalRaids; $i++)
    {
        $msg .= sprintf( "    %hu\n", $info{RIDS}[$i]);
    }

    $msg .= sprintf( "\n");

    return $msg;
}

##############################################################################
# Name: displayVirtualDiskCreate
#
# Desc: Print the virtual disk information
#
# In:   Virtual Disk Create Information Hash
##############################################################################
sub displayVirtualDiskCreate
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    print "Virtual Disk Prepare/Create/Expand Results:\n";
    printf "  STATUS:                0x%x\n", $info{STATUS_MRP};
    printf "  LEN:                   %lu\n", $info{LEN};
    printf "  VID:                   %hu\n", $info{VID};
    print  "  CAPACITY:              $info{CAPACITY}\n";
    print "\n";
}

##############################################################################
# Name: displayVirtualDiskSetPriority
#
# Desc: Print the virtual disk information
#
# In:   Virtual Disk Set Priority Information Hash
##############################################################################
sub displayVirtualDiskSetPriority
{
    my ($self, %info) = @_;
    my $i;
    my $j=0;
    my $k;

    logMsg("begin\n");

    print "Virtual Disk Set Priority Results:\n";

    for ($i = 0; $i < $info{COUNT}; $i++)
    {
        $j = $i*2;
        $k = $j+1;

        if ($info{VIDPAIRS}[$k] == 0)
        {
            printf "\nPriority succesfully set for VID $info{VIDPAIRS}[$j] ";
        }
        else
        {
            if ($info{VIDPAIRS}[$k] == 1)
            {
                printf "\nInvalid VID $info{VIDPAIRS}[$j] ";
            }
            else
            {
                printf "\nFailed to set priority for VID $info{VIDPAIRS}[$j] ";
            }
        }
    }

    print "\n";
}

##############################################################################
# Name: displayVdiskPriorityEnable
#
# Desc: Print the virtual disk information
#
# In:   Virtual Disk Set Priority Information Hash
##############################################################################
sub displayVdiskPriorityEnable
{
    my ($self, %info) = @_;
    my $i;
    my $j=0;
    my $k;

    logMsg("begin\n");

    if ($info{RESP} == 0)
    {
        printf "\nGlobal VDisk Priority Disabled";
    }
    else
    {
        printf "\nGlobal VDisk Priority Enabled";
    }

    print "\n";
}
##############################################################################
# Name: displayVdiskPRGet
#
# Desc: Print the virtual disk persistent reserve information
#
# In:   Virtual Disk PR Get Information Hash
##############################################################################
sub displayVdiskPRGet
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $i;
    my $msg = "";


    $msg .= sprintf( "Virtual Disk Persistent Reserve Information:\n");
    $msg .= sprintf( "  STATUS:               0x%x\n", $info{STATUS_MRP});
    $msg .= sprintf( "  LEN:                  %lu\n", $info{LEN});
    $msg .= sprintf( "  VID:                  %hu\n", $info{VID});
    $msg .= sprintf( "  SID:                  %hu\n", $info{SID});
    $msg .= sprintf( "  TID:                  %u\n", $info{TID});
    $msg .= sprintf( "  LUN:                  %u\n", $info{LUN});
    $msg .= sprintf( "  SCOPE:                0x%x\n", $info{SCOPE});
    $msg .= sprintf( "  KEY COUNT:            %u\n", $info{KEY_COUNT});

    $msg .= sprintf(  "\nRegistered Keys\n");
    $msg .= sprintf(  "--------------------------------------------------------------------------------------------\n");
    
    my $buffer = $info{REG_KEYS};
#    for ($i = 0; $i < 64; $i++)
#    {
#        my $key = substr($buffer,$i,1);
#        $msg .= sprintf( "  %x ",ord($key));
#        if($i%8)
#        {
#            $msg .= sprintf("\n");
#        }
#    }

    $msg .= sprintf( "\n");

    print $msg;
}

##############################################################################
# Name: displayVirtualDiskOwner
#
# Desc: Print the virtual disk owner information
#
# In:   Virtual Disk Owner Hash
##############################################################################
sub displayVirtualDiskOwner
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $i;

    print "Virtual Disk Owner Information:\n";
    printf "  STATUS:                0x%x\n", $info{STATUS_MRP};
    printf "  LEN:                   %lu\n", $info{LEN};
    printf "  NDEVS:                 %hu\n", $info{NDEVS};

    print  "  Owners (port, tid, lun, sid):\n";
    for ($i = 0; $i < $info{NDEVS}; $i++)
    {
        printf "    0x%x  0x%x  0x%x  0x%x\n",
                $info{OWNERS}[$i]{CHANNEL},
                $info{OWNERS}[$i]{TID},
                $info{OWNERS}[$i]{LUN},
                $info{OWNERS}[$i]{SID};
    }

    print "\n";
}

##############################################################################
#
#   All commands that start with a _ should not be used and are considered
#   private.
#
##############################################################################

##############################################################################
# Name:     _virtualDisksPacket
#
# Desc:     Parses the virtual disks packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
##############################################################################
sub _virtualDisksPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if ((commandCode($recvPacket) == PI_VDISKS_CMD) ||
        (commandCode($recvPacket) == PI_VDISKS_FROM_CACHE_CMD))
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $info{COUNT}
        ) = unpack("S", $parts{DATA});

        my @vdisks;

        my $start = 4;

        my $version = 0;
        if (defined($self) && $self != 0) {
            $version = $self->{Controller_FW_Version};
        }

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            my $rsvd;
            my %capacity;
            my %rreq;
            my %wreq;

            # Unpack the data
            (
            $rsvd,
            $vdisks[$i]{STATUS_MRP},
            $vdisks[$i]{LEN},
            $vdisks[$i]{VID},
            $vdisks[$i]{MIRROR},
            $vdisks[$i]{DEVSTAT},
            $vdisks[$i]{SCORVID},
            $vdisks[$i]{SCPCOMP},
            $vdisks[$i]{RAIDCNT},

            $capacity{LO_LONG}, $capacity{HI_LONG},
            $vdisks[$i]{ERROR},
            $vdisks[$i]{QD},

            $vdisks[$i]{RPS},
            $vdisks[$i]{AVGSC},
            $rreq{LO_LONG}, $rreq{HI_LONG},

            $wreq{LO_LONG}, $wreq{HI_LONG},
            $vdisks[$i]{ATTR},
            $vdisks[$i]{DRAIDCNT},
            $vdisks[$i]{OWNER},
            $vdisks[$i]{PRIORITY},

            $vdisks[$i]{GR_B1},
            $vdisks[$i]{GR_B2},
            $vdisks[$i]{GR_B3},
            
            $vdisks[$i]{SPRC},
            $vdisks[$i]{SPSC},
            $vdisks[$i]{SCHEAD},
            $vdisks[$i]{SCTAIL},

            $vdisks[$i]{CPSCMT},
            $vdisks[$i]{VLINKS},
            $vdisks[$i]{NAME},
            $vdisks[$i]{TIMESTAMP},
            $vdisks[$i]{LACCESS},
            $vdisks[$i]{AVERAGEIO},
            $vdisks[$i]{AVERAGESC}
            ) = unpack("a3CLSCCSCC LLLL LLLL LLSCCCCCC LLLL LLZ16LLLL", substr($parts{DATA}, $start));

            # Now fixup all the 64 bit  numbers
            $vdisks[$i]{CAPACITY} = longsToBigInt(%capacity);
            $vdisks[$i]{RREQ} = longsToBigInt(%rreq);
            $vdisks[$i]{WREQ} = longsToBigInt(%wreq);


            $start += 120;

            my @rids;
            my $totalRaids = $vdisks[$i]{RAIDCNT} + $vdisks[$i]{DRAIDCNT};

            if ($totalRaids > 0)
            {
                for (my $j = 0; $j < $totalRaids; $j++)
                {
                    $rids[$j] = unpack("S", substr($parts{DATA}, $start, 2));
                    $start += 2;
                }
            }
            else
            {
                $start += 2;
            }

            $vdisks[$i]{RIDS} = [@rids];

            # Make sure information is present.
            if ($vdisks[$i]{LEN} > (120 + 2 * $totalRaids))
            {
              my $extndvinfo = substr ($parts{DATA},$start,24);
              (
               $vdisks[$i]{BREAKTIME},
               $rsvd,
               $rsvd,
               $rsvd,
               $rsvd,
               $rsvd,
               ) = unpack("LLLLLL",$extndvinfo);
              $start += 24;
            }
        }
        $info{VDISKS} = [@vdisks];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual disks packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _virtualDiskCreatePacket
#
# Desc:     Parses the virtual disk info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _virtualDiskCreatePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_VDISK_CREATE_CMD ||
        commandCode($recvPacket) == PI_VDISK_EXPAND_CMD ||
        commandCode($recvPacket) == PI_VDISK_PREPARE_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;
        my %capacity;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{VID},
        $rsvd,
        $capacity{LO_LONG}, $capacity{HI_LONG}
        ) = unpack("a3CLSa2LL", $parts{DATA});

        # Now fixup all the 64 bit  numbers
        $info{CAPACITY} = longsToBigInt(%capacity);
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual disk create packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _virtualDiskSetPriorityPacket
#
# Desc:     Parses the virtual disk info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _virtualDiskSetPriorityPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;
 
    logMsg("begin\n");

    my %info;
    my $i;
    my $j;
    my @vpripairs;
    my $start = 8;

    if (commandCode($recvPacket) == PI_VDISK_SET_PRIORITY_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;
        my %capacity;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};


        # Unpack the data
        (
        $info{COUNT}
        ) = unpack("S", substr($parts{DATA}, $start, 2));

        $j=0;
        $start = $start + 8;           # 6 Reserved Bytes 

        for ($i = 0; $i < $info{COUNT}; $i++)
        {
            $vpripairs[$j] = unpack("S", substr($parts{DATA}, $start, 2));
            $j++;
            $start = $start + 2;

            $vpripairs[$j] = unpack("C", substr($parts{DATA}, $start, 2));
            $j++;

            $start = $start + 2;
        }

        $info{VIDPAIRS} = [@vpripairs];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual disk set priority packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _vdiskPriorityEnablePacket
#
# Desc:     Parses the vdisk priority enable packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _vdiskPriorityEnablePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;
 
    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_VCG_SET_VDISK_PRIORITY_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $rsvd,
        $info{RESP}
        ) = unpack("a8L", $parts{DATA});
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual disk set priority packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _vdiskPRGetPacket
#
# Desc:     Parses the PR Get packet and places the information
#           in a hash and prints it out.
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _vdiskPRGetPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_VDISK_PR_GET_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $prdat = substr($parts{DATA}, 0, 1040);

        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};
        

        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{VID},
        $info{SID},
        $info{TID},
        $info{LUN},
        $info{SCOPE},
        $info{KEY_COUNT}
              ) = unpack("a3CLSSCCCC", $prdat);
   

        printf( "Virtual Disk Persistent Reserve Information:\n");
        printf( "  STATUS:               0x%x\n",$info{STATUS_MRP});
        printf( "  LEN:                  %lu\n", $info{LEN});
        printf( "  VID:                  %hu\n",$info{VID});
        printf( "  SID:                  %hu\n",$info{SID});
        printf( "  TID:                  %u\n", $info{TID});
        printf( "  LUN:                  %u\n", $info{LUN});
        printf( "  SCOPE:                0x%x\n", $info{SCOPE});
        printf( "  KEY COUNT:            %u\n", $info{KEY_COUNT});

        printf(  "\nRegistered Keys\n");
        printf(  "---------------------------------------------------\n");
    
        my $j =16;
        my $key_cnt = $info{KEY_COUNT};
        
        while(($j<528)&&($key_cnt>0))
        {
            my $k = substr ($prdat,$j,1);
            $j++;
            printf("%2.2x ",ord ($k));
            if(($j%8)==0)
            {
                printf("\n");
                $key_cnt--;
            }
        }
        
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a perstistent reserve data packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _virtualDiskInfoPacket
#
# Desc:     Parses the virtual disk info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _virtualDiskInfoPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;

    my $version = 0;
    if (defined($self) && $self != 0) {
        $version = $self->{Controller_FW_Version};
    }

    if (commandCode($recvPacket) == PI_VDISK_INFO_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $vdev = substr($parts{DATA}, 0, 120);

        my $rsvd;
        my %capacity;
        my %rreq;
        my %wreq;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{VID},
        $info{MIRROR},
        $info{DEVSTAT},
        $info{SCORVID},
        $info{SCPCOMP},
        $info{RAIDCNT},

        $capacity{LO_LONG}, $capacity{HI_LONG},
        $info{ERROR},
        $info{QD},

        $info{RPS},
        $info{AVGSC},
        $rreq{LO_LONG}, $rreq{HI_LONG},

        $wreq{LO_LONG}, $wreq{HI_LONG},
        $info{ATTR},
        $info{DRAIDCNT},
        $info{OWNER},
        $info{PRIORITY},

        $info{GR_B1},
        $info{GR_B2},
        $info{GR_B3},

        $info{SPRC},
        $info{SPSC},
        $info{SCHEAD},
        $info{SCTAIL},

        $info{CPSCMT},
        $info{VLINKS},
        $info{NAME},
        $info{TIMESTAMP},
        $info{LACCESS},
        $info{AVERAGEIO},
        $info{AVERAGESC}
        ) = unpack("a3CLSCCSCC LLLL LLLL LLSCCCCCC LLLL LLa16LLLL", $vdev);

        my $rid = substr($parts{DATA}, 120);
        my @rids;
        my $totalRaids = $info{RAIDCNT} + $info{DRAIDCNT};

        for ($i = 0; $i < $totalRaids; $i++)
        {
            my $start = 2 * $i;
            $rids[$i] = unpack("S", substr($rid, $start, 2));
        }

        $info{RIDS} = [@rids];
        # Now fixup all the 64 bit  numbers
        $info{CAPACITY} = longsToBigInt(%capacity);
        $info{RREQ} = longsToBigInt(%rreq);
        $info{WREQ} = longsToBigInt(%wreq);

        if ($version >= 840)
        {
            #start the extended packet 
            my $extndvinfo = substr($parts{DATA},(120 + ($totalRaids * 2)));
            (
             $info{BREAKTIME},
             $rsvd,
             $rsvd,
             $rsvd,
             $rsvd,
             $rsvd,
             ) = unpack("LLLLLL",$extndvinfo);
        }
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual disk info packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _virtualDiskOwnerPacket
#
# Desc:     Parses the virtual disk owner packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _virtualDiskOwnerPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;

    if (commandCode($recvPacket) == PI_VDISK_OWNER_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{NDEVS},
        $rsvd
        ) = unpack("a3CLSS", $parts{DATA});

        my @owners;

        for ($i = 0; $i < $info{NDEVS}; $i++)
        {
            my $start = 12 + (8 * $i);

            (
            $owners[$i]{TID},
            $owners[$i]{CHANNEL},
            $owners[$i]{SID},
            $owners[$i]{LUN}
            ) = unpack("SSSS", substr($parts{DATA}, $start));
        }

        $info{OWNERS} = [@owners];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual disk info packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _bayRedundantResponsePkt
#
# Desc:     Retrieves the redundancy status of the specified VDisk 
#
# In:       None
#
# Returns:
#           Redundancy status 
#
##############################################################################
sub _bayRedundantResponsePkt
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");
    my %info;
    my $rsvd;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    # Load into hash
        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{REDUNDANCY_STATUS}
                        ) = unpack ("a3CLL", $parts{DATA});

    return %info;
}



##############################################################################
# Name:     _virtualDiskGetNectVid
#
# Desc:     Retrieves the next available Vid to use
#
# In:       None
#
# Returns:
#       The Vid:
#
##############################################################################
sub _virtualDiskGetNextVid
{
    my ($self) = @_;
    my $vid = 0;
    my $current = 0;
    my $i = 0;
    
    my %rsp = $self->getObjectList(PI_VDISK_LIST_CMD);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my @unsortedvids;
            
            for $i (0..$#{$rsp{LIST}})
            {
                $unsortedvids[$i] = $rsp{LIST}[$i];
            } 

            my @vids = sort {$a <=> $b} @unsortedvids;

            for ($i = 0; $i < scalar(@vids); ++$i)
            {
                my $count = 0;

                while ($current < $vids[$i])
                {
                    $count++;
                    $current++;
                }
                
                if ($count > 0)
                {  
                    $vid = ($vids[$i] - $count);
                    last;
                }
                
                if ($current == $vids[$i])
                {
                    $current++;
                }
            }
            
            if ($i == scalar(@vids))
            {
                $vid = $current;
            }
        }
        else
        {
            return undef;
        }
    }
    else
    {
        return undef;
    } 
    
    if (($vid < 0) || ($vid >= 4096))
    {
        return undef;
    }

    return $vid;
}

##############################################################################

1;

##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
