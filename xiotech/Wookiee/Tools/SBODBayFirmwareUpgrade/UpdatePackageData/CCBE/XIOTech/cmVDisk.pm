# $Header$
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Anthony Asleson
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

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["virtualDisks"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VDISKS_CMD;
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

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_2);

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
# Name: virtualDiskCheckAssociate
#
# Desc: Deletes all associations with a vdisk
#
# In:   Virtual Disk Owner Hash
##############################################################################
sub virtualDiskCheckAssociate
{
    my ($self, $vid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["virtualDiskCheckAssociate"]];
    
    my $i;
    my $j;
    my %rsp;
    my %rsp2;
    my %rsp3;
    
    my %retVal;
    
    $retVal{STATUS} = PI_GOOD;
    
    %rsp = $self->getObjectList(PI_SERVER_LIST_CMD);
            
    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            for $i (0..$#{$rsp{LIST}})
            {
                %rsp2 = $self->serverInfo($rsp{LIST}[$i]);
                
                if (%rsp2)
                {
                    if ($rsp2{STATUS} == PI_GOOD)
                    {
                        for (my $j = 0; $j < $rsp2{NLUNS}; ++$j)
                        {
                            if ($vid == $rsp2{LUNMAP}[$j]{VID})
                            {
                                $retVal{STATUS} = PI_ERROR;
                                last;
                            }
                        }
                        
                        if ($retVal{STATUS} == PI_ERROR)
                        {
                            last;
                        }
                    }
                }
            }
        }
    }
    
    return %retVal
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
    
    if (uc($dsptype) eq "STD")
    {
    $msg .= sprintf( "Virtual Disks ($info{COUNT} disks):\n");
    $msg .= sprintf(  "\n");

    $msg .= "                                                        DEFERED  SEC COPY\n";
        $msg .= " VID  DEVSTAT     CAPACITY     MIRROR   ATTR   RAIDCNT  RAIDCNT   % COMP    PRIORITY  OWNER   NAME\n";
        $msg .= " ---  -------  --------------  ------  ------  -------  -------  --------   --------  ------  -------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf( " %3hu     0x%02x  %14s    0x%2.2x  0x%4.4x  %7d  %7d   %7d    %7d    0x%2.2x  %-16s\n", 
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
        $msg .= sprintf( "Virtual Disks ($info{COUNT} disks):\n");
        $msg .= sprintf(  "\n");

        $msg .= " VID           CREATE TIME               LAST ACCESS TIME\n";
        $msg .= " ---      ------------------------   -------------------------\n";

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

            $msg .= sprintf( " %3hu      %-23s             %-23s\n",
                    $info{VDISKS}[$i]{VID},
                    $timeStr,
                    $wtimeStr);
        }
    }
    elsif (uc($dsptype) eq "STATS")
    {
        $msg .= sprintf( "Virtual Disks ($info{COUNT} disks):\n");
        $msg .= sprintf(  "\n");

        $msg .= "                         AVERAGE         AVERAGE \n";
        $msg .= "                         REQ/SEC         BLOCKS/SEC\n";
        $msg .= " VID  DEVSTAT  OWNER    OVER LAST HOUR  OVER LAST HOUR\n";
        $msg .= " ---  -------  -----   ---------------  --------------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf( " %3hu     0x%02x  0x%2.2x  %10u    %10u\n",
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

        $msg .= " VID  DEVSTAT   MIRROR    ATTR   OWNER    OP_STATE  PERM_FLAGS  TEMP_FLAGS\n";
        $msg .= " ---  -------   ------   ------  -------  --------  ----------  ----------\n"; 

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $opState = $info{VDISKS}[$i]{GR_B1} & 7;
            $permFlags = $info{VDISKS}[$i]{GR_B1} >> 3; # Upper 5 bits

            $msg .= sprintf( " %3hu     0x%02x     0x%2.2x   0x%4.4x   0x%2.2x  %8u %10u %10u\n",
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

    if (commandCode($recvPacket) == PI_VDISKS_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $info{COUNT}
        ) = unpack("S", $parts{DATA});

        my @vdisks;

        my $start = 4;

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
# Name:     _vdiskPRClrPacket
#
# Desc:     Parses the PR Clear packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _vdiskPRClrPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;
 
    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_VDISK_PR_CLR_CMD)
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
        logMsg("Unexpected packet: We expected a virtual disk PR Get packet\n");
    }

    return %info;
    #TODO fix this
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
    
    if (($vid < 0) || ($vid >= 1024))
    {
        return undef;
    }

    return $vid;
}

1;

##############################################################################
#
# Change log:
# $Log$
# Revision 1.17  2006/12/14 12:14:15  BalemarthyS
# TBolt00017286 - Fixed the formatting with the CCBCL information for the VDISKS command
#
# Revision 1.16  2006/12/06 16:06:37  BharadwajS
# TBolt00017136 Committing back the stats vdisk changes
#
# Revision 1.14  2006/12/05 22:52:14  BharadwajS
# TBolt00017136 Adding logic to compute vdisk statistics for the last one hour
#
# Revision 1.13  2006/11/29 22:17:50  BharadwajS
# TBolt00017125 adding VDisk create time and last access time.
#
# Revision 1.12  2006/08/17 17:32:44  NigburC
# TBolt00000000
# Fixed some formatting issues with vdisks and fixed indentation issues in code
# itself.
#
# Revision 1.11  2006/07/17 20:38:32  RustadM
# TBolt00014770
# Move 750 branch onto main.
#
# Revision 1.7.20.2  2006/04/26 09:06:20  BharadwajS
# CCBE changes for PI Versioning
#
# Revision 1.7.20.1  2006/02/24 14:17:24  MiddenM
#
# Merge from WOOKIEE_EGGS_GA_BR into MODEL750_BR
#
# Revision 1.10  2006/01/26 13:39:18  BoddukuriV
# TBolt00000000 - Changes made to display OP-STATE correctly
#
# Revision 1.9  2006/01/12 11:58:23  ChannappaS
# Adding GeoLocation to VdiskInfo Packet
#
# Revision 1.7  2005/08/16 08:12:16  BalemarthyS
# Added PRIORITY Feild in vdisks command display by gopi
#
# Revision 1.6  2005/07/30 07:15:23  BharadwajS
# X1 Code for Serviceability
#
# Revision 1.5  2005/06/01 12:18:24  BharadwajS
# VDisk Priority Chagnes
#
# Revision 1.4  2005/05/25 10:24:24  BharadwajS
# Undo Vpri changes
#
# Revision 1.3  2005/05/25 10:09:44  BharadwajS
# Undo vpri changes
#
# Revision 1.2  2005/05/24 14:18:35  BharadwajS
# TBolt00000 vpri
#
# Revision 1.1.1.1  2005/05/04 18:53:54  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.41  2004/09/21 15:24:30  WilliamsJ
# TBolt00011344 - Merge of resync into main.
#
# Revision 1.40.2.1  2004/09/14 19:42:46  WilliamsJ
# TBolt00011260 -  Added ownership checking and tracking.  Reviewed by
# Chris.
#
# Revision 1.40  2004/08/16 20:40:47  RysavyR
# TBolt00011053: Parse vdisk NAME as a NUL terminated string.
#
# Revision 1.39  2004/08/16 19:58:21  RysavyR
# TBolt00011053: Fix the FID 280 (vdisk cache) decoder. Also added "NAME"
# to the vdisk display output.
#
# Revision 1.38  2004/04/28 18:17:38  HoltyB
# TBolt00000000:  Added Wookiee andling for the CCBE amd CCBCL.
#
# Revision 1.37  2003/08/05 18:58:57  SchibillaM
# TBolt00008793: Complete GeoPool support in CCB and CCBE.
# Reviewed by Randy.
#
# Revision 1.36  2003/06/03 19:46:17  MenningC
# TBOLT00000000: Changed many of the 'display' functions to fill a string rather than print to the screen. This supports use by the test scripts. Reviewed by Randy R.
#
# Revision 1.35  2003/05/28 12:28:39  HoltyB
# TBolt00008320:  Changed the way the vdisk attributes are set and logged.
# No longer uses MRP pass through, we now determine exactly what the
# attribute change is so that it can be logged properly.
# Reviewed by Mark Schibilla.
#
# Revision 1.34  2003/04/23 14:21:46  TeskeJ
# tbolt00000000 - added devstat and raidcount to vdisks display
# rev by Jim
#
# Revision 1.33  2003/04/22 13:39:32  TeskeJ
# tbolt00008122 - 'pdisks loop' updates and changed 'channel' to 'port'
# rev by Chris
#
# Revision 1.32  2002/12/18 19:05:50  NigburC
# TBolt00000000 - Fixed a huge bug that Jeff Williams found for me.  I was
# not parsing the vdisk info packet correctly in the CCBE...huge bug!!!
# Reviewed by Jeff Williams.
#
# Revision 1.31  2002/12/13 15:51:00  NigburC
# TBolt00006462, TBolt00006408 - Modified the create vdisk structure to handle the
# threshold and max raids.  Added handling in the CCBE to support the these
# two new requirements.  Added handling for the X1 create vdisk operation to
# convert the X1 values and pass them to the PROC.
# Reviewed by Jeff Williams.
#
# Revision 1.30  2002/12/11 16:22:43  NigburC
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
# Revision 1.29  2002/12/06 21:37:50  NigburC
# TBolt00006392, TBolt00006394, TBolt00006429 - Lots of changes to enjoy.
# - Added code to support the new NAME_DEVICE MRP.
# - Added code to support setting server, vdisk and controller names.
# - Updated the SERVERASSOC and SERVERDELETE commands to allow
# additional options.
# Reviewed by Mark Schibilla.
#
# Revision 1.28  2002/10/28 14:28:20  NigburC
# TBolt00000000 - Added the percent complete to the VDISKS display.
# Reviewed by Jeff Williams.
#
# Revision 1.27  2002/10/22 11:58:57  HoltyB
# TBolt00006202: Fixed vdiskPrepare, Create, and Expand, to allow a miiror
# depth of less than 2 down to the CCB.
#
# Revision 1.26  2002/10/08 12:46:55  HoltyB
# TBolt00006093:  Chnaged vdiskCreate to allow setting the VID explicitly.
# If VID is not defined the first available VID is determined and used.
#
# Revision 1.25  2002/09/03 14:32:52  SchibillaM
# TBolt00005858: Changes to CCBCL files to allow a subset of function to be built
# for field use.  The tool BuildCCBCLSubset.pl builds the subset.  These changes
# also remove ENVSTATS which is replaced by STATSENV.
# Reviewed by Randy and Craig.
#
# Revision 1.24  2002/06/18 20:36:42  HoltyB
# TBolt00004524: Added ability to retrieve SOS table for a pdisk
# TBolt00004836: Placed a check in vdiskDelete to check for server
#                            associations before deletion
#
# Revision 1.23  2002/04/16 14:30:53  NigburC
# TBolt00003594 - Added VDISKS, RAIDS and PORTLIST commands.
# Modified the DEVSTAT calls to use the new VDISKS and RAIDS commands
# to make them faster.
#
# Revision 1.22  2002/04/01 16:43:35  HoltyB
# Changed the parameter validation for mirror depth on virtualDiskCreate,
# virtualDiskExpand, and virtualDiskPrepare to allow for a larger mirror depth
#
# Revision 1.21  2002/03/12 15:17:12  HoltyB
# Made some changes to deviceStatus to fix some of the return codes
#
# Revision 1.20  2002/02/26 16:30:41  NigburC
# TBolt00003161 - When determining the number of raids in a vdisk we were
# not accounting for deferred raids.  This change affected CCB, CCBE and
# UMC code for the EM group.
#
# Revision 1.19  2002/02/12 13:43:25  NigburC
# TBOLT00000000 - No defect associated with this fix.
# Increased the parameter limit for the VID parameter in the VirtualDiskExpand
# function.
#
# Revision 1.18  2002/02/11 19:08:03  HoltyB
# moved command codes to PI_CommandCodes.pm
#
# Revision 1.17  2002/02/05 23:20:09  NigburC
# TBOLT00002954,
# TBOLT00002877,
# TBOLT00002730
#
# This set of changes include the above work items and defects.  Also are
# changes associated with MRP packet structure changes.
# We have moved many functions from CPS_INIT.C to PI_VCG.C in order
# to start returning error codes from the functions that work on the VCG.
# New functions have been added including but not limited to:
#   RESCANDEVICE
#   TARGETRESLIST
#   SERVERLOOKUP (not yet implemented in CCBE and CCBCL)
#   VDISKOWNER
#
# Revision 1.16  2002/01/31 13:42:22  HoltyB
# fixed problem with virtualDiskPrepare. allowed a higher mirror depth
# to be allowed
#
# Revision 1.15  2002/01/22 23:17:30  HoltyB
# updated virtualDiskCreate to accept larger number for depth
#
# Revision 1.14  2002/01/08 13:11:26  HoltyB
# moved displayVirtualDiskRaidInfo to cmRaid.pm
#
# Revision 1.13  2002/01/08 13:07:59  HoltyB
# moved virtualDiskRaidInfo and _virtualDiskRaidInfoPacket to cmRaid.pm
#
# Revision 1.12  2001/12/20 22:33:53  NigburC
# Modified the CCBCL to handle the checking for active connection in the
# big switch statement instead of in each of the command handlers.  This way
# we could use the code from just one area and print the same message in the
# case a command needs an active connection and one is not available.
#
# Added getObjectCount and getObjectList functions to the CCBE and now
# use them for all count and list operations.  These functions now match the
# changes to the packets where LIST no longer takes any input and returns
# a count value along with the list of values.
#
# Revision 1.11  2001/12/10 20:59:53  NigburC
# Fixed display routines, STATUS_MRP was not being displayed.
#
# Revision 1.10  2001/12/07 17:11:04  NigburC
# Added VLINK commands.
# Added DEVSTATUS command.
# Added RAIDCOUNT and RAIDLIST commands.
# Reverted the byte swapping done on capacity and count 64 bit integer values
# since they really did not need to be swapped.  Only WWNs should be
# swapped.
# Fixed other bugs found during debugging.
#
# Revision 1.9  2001/12/04 17:34:45  NigburC
# Modified the raid info packet handling to split the PIDS array into the two
# real pieces (PID and STATUS).
#
# Revision 1.8  2001/11/30 17:19:54  NigburC
# Added VDISKSETCACHE functionality.
#
# Revision 1.7  2001/11/27 17:40:44  NigburC
# Added the vdiskControl command line handler and it associated functions.
#
# Revision 1.6  2001/11/27 15:58:04  NigburC
# Added the following command handlers:
# VDISKPREPARE
# VDISKEXPAND
# VDISKINIT
# VDISKRAIDINFO
#
# Revision 1.5  2001/11/15 19:39:23  NigburC
# Added VDISKDELETE.
#
# Revision 1.4  2001/11/14 12:57:37  NigburC
# Modified verify parameters to return a hash instead of just the message.
# This will allow the code to return it to the command line where it will be
# processed like any other error, except there is a MESSAGE in the
# hash which will be displayed describing the parameter error that occurred.
#
# Revision 1.3  2001/11/13 23:02:12  NigburC
# Modified the command line interface to always expect a response packet
# returned from the commands called in the command manager.  This will
# enable the command line to check the STATUS to determine if the command
# was good or bad and then interrogate the ERROR_CODE to determine what
# the underlying error really was.
#
# Revision 1.2  2001/11/13 15:41:58  NigburC
# Completed the VDISKCREATE code and updated the other VDISK
# functions.
# Switched the usage for LO_LONG and HI_LONG when dealing with 64 bit
# integers since I had them reversed.
#
# Revision 1.1  2001/11/08 13:32:11  NigburC
# Initial integration of the virtual disk command manager module.
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
