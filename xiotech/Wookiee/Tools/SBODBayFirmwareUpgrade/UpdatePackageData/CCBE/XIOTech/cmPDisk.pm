# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Anthony Asleson
#
# Purpose:
#   Wrapper for all the different XIOTech commands that can be sent
#   to the XIOtech SAN system
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
# Name:     physicalDisks
#
# Desc:     Retrieves physical disk information for all physical disks.
#
# In:       NONE
#
# Returns:
##############################################################################
sub physicalDisks
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["physicalDisks"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISKS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_physicalDisksPacket);
}

##############################################################################
# Name:     physicalDiskBeacon
#
# Desc:     Beacon a physical disk
#
# In:       ID of the physical disk
#
# Returns:
##############################################################################
sub physicalDiskBeacon
{
    my ($self, $pid, $dur) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ["physicalDiskBeacon"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_BEACON_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $pid,
                    $dur);

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
# Name:     physicalDiskCount
#
# Desc:     Retrieves the number of physical disks.
#
# Input:    None
#
# Returns:
##############################################################################
sub physicalDiskCount
{
    my ($self) = @_;
    return $self->getObjectCount(PI_PDISK_COUNT_CMD);
}

##############################################################################
# Name:     physicalDiskDefrag
#
# Desc:     Defragement a physical disk
#
# In:       ID of the physical disk
#
# Returns:
##############################################################################
sub physicalDiskDefrag
{
    my ($self, $pid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["physicalDiskDefrag"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_DEFRAG_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $pid,
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
# Name:     physicalDiskDefragStatus
#
# Desc:     Defragementation status of physical disk(s)
#
# In:       ID of the physical disk
#
# Returns:
##############################################################################
sub physicalDiskDefragStatus
{
    my ($self) = @_;

    logMsg("begin\n");

    my $cmd = PI_PDISK_DEFRAG_STATUS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    
    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_pDiskDefragStatusPacket);
}

##############################################################################
# Name:     physicalDiskFail
#
# Desc:     Fail a physical disk
#
# In:       ID of the physical disk
#           FORCE option
#
# Returns:
##############################################################################
sub physicalDiskFail
{
    my ($self, $id, $hspid, $force) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0x0F],
                ["physicalDiskFail"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_FAIL_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SSCCCC",
                    $id,
                    $hspid,
                    $force,
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

#SERVICEABILITY42
##############################################################################
# Name:     physicalDiskAutoFailBack
#
# Desc:     Enables/Disables the auto failback feature 
#
# In:          option
#
# Returns:
##############################################################################
sub physicalDiskAutoFailBack
{
    my ($self, $option) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0x0F],
                ["physicalDiskAutoFailBack"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $option,
                    0,0,0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
#                                        \&_statusResponsePacket);
                                        \&_pDiskAutoFailbackPacket);
}
#SERVICEABILITY42


#SERVICEABILITY42
##############################################################################
# Name:     physicalDiskFailBack
#
# Desc:     Failback an used hotspare physical disk
#
# In:       ID of the used hotspare physical disk
#           option
#
# Returns:
##############################################################################
sub physicalDiskFailBack
{
    my ($self, $hspid, $option) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0x0F],
                ["physicalDiskFailBack"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_FAILBACK_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SCCCC",
                    $hspid,
                    $option,
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
#SERVICEABILITY42

##############################################################################
# Name:     physicalDiskInfo
#
# Desc:     Get information about a physical disk
#
# In:       ID of the physical disk
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       DISK_ID                 Physical disk id
#       NAME                    Name of disk
#       CLASS                   Values
#                               0 = Unlabeled
#                               1 = Labeled device with redundancy
#                               2 = hotspare
#
#       SYSTEM_SERIAL_NUMBER    San box serial number   (perl BigInt)
#       CURRENT_POST_STATUS
#       DRIVE_STATUS            Values
#                               0 = Nonexistent device
#                               1 = Operational drive
#                               2 = Inoperative drive
#       CAPACITY                Capacity in bytes (perl BigInt)
#       PRODUCT_ID
#       VENDER_ID
#       REVISION
#       SERIAL_NUMBER
#       AVAILABLE_SPACE         In bytes (perl BigInt)
#       LARGEST_AVAIL_SEG       Largest available segment
#       WWN                     Disk WWN    (perl BigInt)
#       LUN
#       CHANNEL
#       LOOPMAP
#       LOOPID
#       CONTAINER_ID
##############################################################################
sub physicalDiskInfo
{
    my ($self, $pid, $options) = @_;

    logMsg("begin\n");

    my $bNewArgs = 0;
    my @newArgs;

    if (!defined($options))
    {
        $options = 0;
        $bNewArgs = 1;
    }

    if ($bNewArgs == 1)
    {
        push @newArgs, $self;
        push @newArgs, $pid;
        push @newArgs, $options;
        
        @_ =  @newArgs;
    }

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['i'],
                ["physicalDiskInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_INFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SCC",
                    $pid,
                    $options,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_physicalDiskInfoPacket);
}

##############################################################################
# Name:     physicalDiskLabel
#
# Desc:     Label/Unlabel physical disk(s)
#
# In:       scalar $physicaldiskOrdinal
#
# Returns:  1 on success, 0 on error
##############################################################################
sub physicalDiskLabel
{
    my ($self,
        $pids,
        $type) = @_;

    logMsg("begin\n");

    my $rc = 0;

    # verify parameters
    my $args = [['i'],
                ['a'],
                ['d', 0, 0xFF],
                ["physicalDiskLabel"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my @disks = @$pids;
    my $pidcnt = scalar(@disks);
    my %rsp;

    for (my $i = 0; $i < $pidcnt; $i++)
    {
        my $pid = $disks[$i];

        %rsp = $self->physicalDiskLabelEx($pid, $type);

        if (%rsp)
        {
            if ($rsp{STATUS} != PI_GOOD)
            {
                return %rsp;
            }
        }
        else
        {
            $rsp{STATUS} = PI_ERROR;
            return %rsp;
        }
    }

    $rsp{STATUS} = PI_GOOD;
    return %rsp;
}

##############################################################################
# Name:     physicalDiskLabelEx
#
# Desc:     Label/Unlabel a physical disk and set the device name
#
# In:       scalar $physicaldiskOrdinal
#
# Returns:  1 on success, 0 on error
##############################################################################
sub physicalDiskLabelEx
{
    my ($self,
        $pid,
        $type) = @_;

    logMsg("begin\n");

    my $rc = 0;

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFF],
                ["physicalDiskLabelEx"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_LABEL_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SCC",
                    $pid,
                    $type,
                    0);

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
# Name:     physicalDiskList
#
# Desc:     Retrieves an array containing the identifiers of the physical disks.
#
# Input:    None
#
# Returns:  List of physical disk identifiers or UNDEF if an error occurred.
##############################################################################
sub physicalDiskList
{
    my ($self) = @_;
    return $self->getObjectList(PI_PDISK_LIST_CMD);
}

##############################################################################
# Name:     physicalDiskRestore
#
# Desc:     Restore a physical disk
#
# In:       ID of the physical disk
#
# Returns:
##############################################################################
sub physicalDiskRestore
{
    my ($self, $pid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["physicalDiskRestore"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_UNFAIL_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $pid,
                    0);

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
# Name:     physicalDiskDelete
#
# Desc:     deletes  a physical disk.
#
#           pid   -  Device ID
#
# Return:   Status Response Hash
##############################################################################
sub physicalDiskDelete
{
    my ($self, $pid) = @_;
    
    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF],
                ["physicalDiskDelete"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_DELETE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCS",
                    DELETE_DISK_DRIVE,
                    0,
                    $pid);

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
# Name:     physicalDiskBypass
#
# Desc:     Bypass a physical disk
#
# In:       pid - ID of the physical disk
#           setting - Bypass setting
#
# Returns:
##############################################################################
sub physicalDiskBypass
{
    my ($self, $ses, $slot, $setting) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFF],
                ['d', 0, 0xF],
                ["physicalDiskBypass"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_BYPASS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SCC",
                    $ses,
                    $slot,
                    $setting);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}

#SERVICEABILITY42
##############################################################################
# Name:     physicalDiskSpindown
#
# Desc:     Spin down a physical disk
#
# In:       pid - ID of the physical disk
#
# Returns:
##############################################################################
sub physicalDiskSpindown
{
    my ($self, $pid) = @_;

    logMsg("begin\n");

    my $cmd = PI_PDISK_SPINDOWN_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("S",
                    $pid);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}
#SERVICEABILITY42

##############################################################################
# Name: displayPhysicalDisks
#
# Desc: Print the physical disks
#
# In:   Physical Disks Information Hash
##############################################################################
sub displayPhysicalDisks
{
    my ($self, $dsptype, %info) = @_;
    my $wrong;
    my $ses;
    my $slot;
    my $tas_ne_las;

    my $msg = "";

    logMsg("begin\n");

    if (uc($dsptype) eq "STD")
    {
        $msg .= sprintf  "Physical Disks ($info{COUNT} disks):\n";
        $msg .= sprintf  "\n";

        $msg .=  "  PID  %REM    RBREMAIN          LAS            TAS        SES   SLOT    DNAME      HSDNAME     GEOLOCATION\n";
        $msg .=  "  ---  ----  -------------  -------------  -------------  -----  ----  ----------  ----------  -------------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $ses = ($info{PDISKS}[$i]{PD_DNAME} & 0x00FF0000) >> 16;
            $slot = ($info{PDISKS}[$i]{PD_DNAME} & 0xFF000000) >> 24;

            if ($ses != $info{PDISKS}[$i]{SES} or
                $slot != $info{PDISKS}[$i]{SLOT})
            {
                $wrong = "*";
            }
            else
            {
                $wrong = " ";
            }

            if($info{PDISKS}[$i]{LAS} == $info{PDISKS}[$i]{TAS})
            {
                $wrong .= " ";
            }
            else
            {
                $wrong .= "!";
            }

            $msg .= sprintf "%s%3hu  %4d  %13s  %13s  %13s  %5d  %4d  %10s  %10s      %3d\n",
                    $wrong,
                    $info{PDISKS}[$i]{PD_PID},
                    $info{PDISKS}[$i]{PCTREM},
                    $info{PDISKS}[$i]{RBREMAIN},
                    $info{PDISKS}[$i]{LAS},
                    $info{PDISKS}[$i]{TAS},
                    $info{PDISKS}[$i]{SES},
                    $info{PDISKS}[$i]{SLOT},
                    _getString_DNAME($info{PDISKS}[$i]{PD_DNAME}),
                    _getString_DNAME($info{PDISKS}[$i]{HSDNAME}),
#GR_GEORAID - Following is added for GEORaid
                    $info{PDISKS}[$i]{GL_ID},
        }
    }
    elsif (uc($dsptype) eq "LOOP")
    {
        my %portHash;
        my $portHashKey;

        my %sesHash;
        my $sesHashKey;

        my %slotHash;
        my $slotHashKey;

        my %lidHash;
        my $lidHashKey;

        my $debug = 1;

        ##
        # Make a new hash of hashes, using the PORT, SES, and SLOT as the keys
        ##
        for( my $i = 0; $i < $info{COUNT}; $i++ )
        {
            ##
            # Hash by PORT, SES, and SLOT
            ##
            my $portAChannelString = "?";
            my $portBChannelString = "?";
            my $portAString = "?";
            my $portBString = "?";
            my $portANumber = $info{PDISKS}[$i]{PD_CHANNEL} & ~1;
            my $portBNumber = $info{PDISKS}[$i]{PD_CHANNEL} |  1;

            if( $info{PDISKS}[$i]{PD_DEVTYPE} == PD_DT_UNKNOWN )
            {
                next;
            }
            elsif ($info{PDISKS}[$i]{PD_DEVTYPE} == PD_DT_FC_DISK ||
                   $info{PDISKS}[$i]{PD_DEVTYPE} == PD_DT_ECON_ENT)
            {
                if( $info{PDISKS}[$i]{CIP} =~ /N\/A/ )
                {
                    next;
                }
                elsif( $info{PDISKS}[$i]{CIP} == 0 )
                {
                    ##
                    # CIP is zero, meaning the current port number is 'channel A'
                    ##
                    $portAChannelString = "A";
                    $portBChannelString = "B";

                    $portANumber = $info{PDISKS}[$i]{PD_CHANNEL};
                    $portAString = sprintf( "*%01d", $info{PDISKS}[$i]{PD_CHANNEL} );

                    if( $info{PDISKS}[$i]{PD_CHANNEL} & 0x01 )
                    {
                        # Port number is odd, so this port is one less (even)
                        $portBNumber = $info{PDISKS}[$i]{PD_CHANNEL} - 1;
                    }
                    else
                    {
                        # Port number is even, so this port is one more (odd)
                        $portBNumber = $info{PDISKS}[$i]{PD_CHANNEL} + 1;
                    }
                }
                else
                {
                    ##
                    # CIP is one, meaning the current port number is 'channel B'
                    ##
                    $portAChannelString = "A";
                    $portBChannelString = "B";

                    $portBNumber = $info{PDISKS}[$i]{PD_CHANNEL};
                    $portBString = sprintf( "*%01d", $info{PDISKS}[$i]{PD_CHANNEL} );

                    if( $info{PDISKS}[$i]{PD_CHANNEL} & 0x01 )
                    {
                        # Port number is odd, so this port is one less (even)
                        $portANumber = $info{PDISKS}[$i]{PD_CHANNEL} - 1;
                    }
                    else
                    {
                        # Port number is even, so this port is one more (odd)
                        $portANumber = $info{PDISKS}[$i]{PD_CHANNEL} + 1;
                    }
                }
            }

            ##
            # Stringify the port numbers
            ##
            $portAString = sprintf( "%01d", $portANumber );
            $portBString = sprintf( "%01d", $portBNumber );

            ##
            # Add channel A to the hash
            ##
            $portHash{$portANumber}{$info{PDISKS}[$i]{SES}}{$info{PDISKS}[$i]{SLOT}}{$info{PDISKS}[$i]{PD_ID}} = {
                PD_PID   => $info{PDISKS}[$i]{PD_PID},
                PD_DNAME => $info{PDISKS}[$i]{PD_DNAME},
                SES      => $info{PDISKS}[$i]{SES},
                SLOT     => $info{PDISKS}[$i]{SLOT},
                LID      => $info{PDISKS}[$i]{PD_ID},
                LMAP     => $info{PDISKS}[$i]{PD_LOOPMAP},
                PORT     => $portAString,
                CHAN     => $portAChannelString,
                LFCNT    => $info{PDISKS}[$i]{LFCNT_A},
                LSCNT    => $info{PDISKS}[$i]{LSCNT_A},
                ITCNT    => $info{PDISKS}[$i]{ITCNT_A},
                ICCNT    => $info{PDISKS}[$i]{ICCNT_A},
                LIPF7I   => $info{PDISKS}[$i]{LIPF7I_A},
                LIPF7R   => $info{PDISKS}[$i]{LIPF7R_A},
                LIPF8I   => $info{PDISKS}[$i]{LIPF8I_A},
                LIPF8R   => $info{PDISKS}[$i]{LIPF8R_A},
                POM      => $info{PDISKS}[$i]{POM} };

            ##
            # Add channel B to the hash
            ##
            $portHash{$portBNumber}{$info{PDISKS}[$i]{SES}}{$info{PDISKS}[$i]{SLOT}}{$info{PDISKS}[$i]{PD_ID}} = {
                PD_PID   => $info{PDISKS}[$i]{PD_PID},
                PD_DNAME => $info{PDISKS}[$i]{PD_DNAME},
                SES      => $info{PDISKS}[$i]{SES},
                SLOT     => $info{PDISKS}[$i]{SLOT},
                LID      => $info{PDISKS}[$i]{PD_ID},
                LMAP     => $info{PDISKS}[$i]{PD_LOOPMAP},
                PORT     => $portBString,
                CHAN     => $portBChannelString,
                LFCNT    => $info{PDISKS}[$i]{LFCNT_B},
                LSCNT    => $info{PDISKS}[$i]{LSCNT_B},
                ITCNT    => $info{PDISKS}[$i]{ITCNT_B},
                ICCNT    => $info{PDISKS}[$i]{ICCNT_B},
                LIPF7I   => $info{PDISKS}[$i]{LIPF7I_B},
                LIPF7R   => $info{PDISKS}[$i]{LIPF7R_B},
                LIPF8I   => $info{PDISKS}[$i]{LIPF8I_B},
                LIPF8R   => $info{PDISKS}[$i]{LIPF8R_B},
                POM      => $info{PDISKS}[$i]{POM} };
        }

        ##
        # Display the data, sorted first on SES number, then on slot number
        ##
        $msg .= sprintf "Physical Disks ($info{COUNT} disks):\n";
        $msg .= sprintf "  PID   DNAME    SES   SLOT  LID   ALPA LMAP PORT LINK_FAIL  LOST_SYNC   INV_XMIT   INV_CRC   LIPF7_Init LIPF7_Recv LIPF8_Init LIPF8_Recv PowerOnMin\n";
        $msg .= sprintf "  --- ---------- ----- ---- ------ ---- ---- ---- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ----------\n";

        foreach $portHashKey( sort _sortNumerically keys %portHash )
        {
            foreach $sesHashKey( sort _sortNumerically keys %{$portHash{$portHashKey}} )
            {
                foreach $slotHashKey( sort _sortNumerically keys %{$portHash{$portHashKey}{$sesHashKey}} )
                {
                    foreach $lidHashKey( sort _sortNumerically keys %{$portHash{$portHashKey}{$sesHashKey}{$slotHashKey}} )
                    {
                        $msg .= sprintf "  %3hu %s %5d  %3d 0x%04x 0x%02x  %3d %2s-%1s %10s %10s %10s %10s %10s %10s %10s %10s %10s\n",
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{PD_PID},
                            _getString_DNAME($portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{PD_DNAME}),
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{SES},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{SLOT},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LID},
                            _getALPA($portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LID}),
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LMAP},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{PORT},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{CHAN},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LFCNT},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LSCNT},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{ITCNT},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{ICCNT},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LIPF7I},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LIPF7R},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LIPF8I},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LIPF8R},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{POM};
                    }
                }

                $msg .= sprintf "\n";
            }

            $msg .= sprintf "\n";
        }
    }
    elsif (uc($dsptype) eq "FWV")
    {
        $msg .= sprintf  "Physical Disks ($info{COUNT} disks):\n";
        $msg .= sprintf  "\n";

        $msg .= sprintf "  PID  VendorID    TYPE     REV   Product ID        Serial #      CAPACITY (blocks)     WWN\n";
        $msg .= sprintf "  ---  --------  --------  -----  ----------------  ------------  --------------------  ----------------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf "  %3hu  %8s  %-8s  %-5s  %16s  %12s  %-20s  %8.8x%8.8x\n", 
                    $info{PDISKS}[$i]{PD_PID},
                    $info{PDISKS}[$i]{VENDID},
                    _getString_PDDT($info{PDISKS}[$i]{PD_DEVTYPE}),
                    $info{PDISKS}[$i]{PD_REV},
                    $info{PDISKS}[$i]{PS_PRODID},
                    $info{PDISKS}[$i]{PS_SERIAL},
                    $info{PDISKS}[$i]{CAPACITY},
                    
                    $info{PDISKS}[$i]{WWN_LO}, $info{PDISKS}[$i]{WWN_HI};
                    
        }
    }
    elsif (uc($dsptype) eq "SES")
    {
        $msg .= sprintf  "Physical Disks ($info{COUNT} disks):\n";
        $msg .= sprintf  "\n";

        $msg .= sprintf "  PID   SES    SLOT      DNAME  \n";
        $msg .= sprintf "  ---  -----  ------  ----------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $ses = ($info{PDISKS}[$i]{PD_DNAME} & 0x00FF0000) >> 16;
            $slot = ($info{PDISKS}[$i]{PD_DNAME} & 0xFF000000) >> 24;

            if ($ses != $info{PDISKS}[$i]{SES} or
                $slot != $info{PDISKS}[$i]{SLOT})
            {
                $wrong = "*";
            }
            else
            {
                $wrong = " ";
            }

            $msg .= sprintf "%s %3hu  %5d  %6d  %s\n",
                    $wrong,
                    $info{PDISKS}[$i]{PD_PID},
                    $info{PDISKS}[$i]{SES},
                    $info{PDISKS}[$i]{SLOT},
                    _getString_DNAME($info{PDISKS}[$i]{PD_DNAME});
        }
    }
    elsif (uc($dsptype) eq "CMPL")
    {
        $msg .= sprintf  "Physical Disks ($info{COUNT} disks):\n";
        $msg .= sprintf  "\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf "Physical Disk Information:\n";
            $msg .= sprintf "  CLASS:                %s\n", _getString_CLASS($info{PDISKS}[$i]{PD_CLASS});
            $msg .= sprintf "  DEVTYPE:              %hu\n", $info{PDISKS}[$i]{PD_DEVTYPE};
            $msg .= sprintf "  MISCSTAT:             0x%02x\n", $info{PDISKS}[$i]{PD_MISCSTAT};
            $msg .= sprintf "  PORT:                 %hu\n", $info{PDISKS}[$i]{PD_CHANNEL};
            $msg .= sprintf "  LOOPMAP:              %hu\n",$info{PDISKS}[$i]{PD_LOOPMAP};
            $msg .= sprintf "  LUN:                  %hu\n", $info{PDISKS}[$i]{PD_LUN};
            $msg .= sprintf "  LID:                  0x%x\n", $info{PDISKS}[$i]{PD_ID};
            $msg .= sprintf "  ALPA:                 0x%x\n", _getALPA($info{PDISKS}[$i]{PD_ID});
            $msg .= sprintf "  DEV:                  0x%x\n", $info{PDISKS}[$i]{PD_DEV};
            $msg .= sprintf "  PID:                  %hu\n", $info{PDISKS}[$i]{PD_PID};
            $msg .= sprintf "  POSTSTAT:             0x%02x\n", $info{PDISKS}[$i]{PD_POSTSTAT};
            $msg .= sprintf "  DEVSTAT:              0x%02x\n", $info{PDISKS}[$i]{PD_DEVSTAT};
            $msg .= sprintf "  FLED:                 0x%x\n", $info{PDISKS}[$i]{PD_FLED};
            $msg .= sprintf "  PCTREM:               %hu\n", $info{PDISKS}[$i]{PCTREM};
            $msg .= sprintf "  CAPACITY:             $info{PDISKS}[$i]{CAPACITY}\n";
            $msg .= sprintf "  QD:                   %lu\n", $info{PDISKS}[$i]{PD_QD};
            $msg .= sprintf "  RPS:                  %lu\n", $info{PDISKS}[$i]{PD_RPS};
            $msg .= sprintf "  AVGSC:                %lu\n", $info{PDISKS}[$i]{PD_AVGSC};
            $msg .= sprintf "  SSERIAL:              0x%x\n", $info{PDISKS}[$i]{PD_SSERIAL};
            $msg .= sprintf "  RREQ:                 $info{PDISKS}[$i]{RREQ}\n";
            $msg .= sprintf "  WREQ:                 $info{PDISKS}[$i]{WREQ}\n";
            $msg .= sprintf "  VENDID:               $info{PDISKS}[$i]{VENDID}\n";
            $msg .= sprintf "  REV:                  $info{PDISKS}[$i]{PD_REV}\n";
            $msg .= sprintf "  ERR:                  %lu\n", $info{PDISKS}[$i]{PD_ERR};
            $msg .= sprintf "  PRODID:               $info{PDISKS}[$i]{PS_PRODID}\n";
            $msg .= sprintf "  SERIAL:               $info{PDISKS}[$i]{PS_SERIAL}\n";
            $msg .= sprintf "  DAML:                 0x%x\n", $info{PDISKS}[$i]{PD_DAML};
            $msg .= sprintf "  TAS:                  $info{PDISKS}[$i]{TAS}\n";
            $msg .= sprintf "  LAS:                  $info{PDISKS}[$i]{LAS}\n";
            $msg .= sprintf "  WWN:                  %8.8x%8.8x\n", $info{PDISKS}[$i]{WWN_LO}, $info{PDISKS}[$i]{WWN_HI};
            $msg .= sprintf "  R10_MISCOMP:          %lu\n", $info{PDISKS}[$i]{R10_MISCOMP};
            $msg .= sprintf "  DNAME:                0x%x\n", $info{PDISKS}[$i]{PD_DNAME};
            $msg .= sprintf "  LFCNT:                %lu\n", $info{PDISKS}[$i]{LFCNT};
            $msg .= sprintf "  LSCNT:                %lu\n", $info{PDISKS}[$i]{LSCNT};
            $msg .= sprintf "  LGCNT:                %lu\n", $info{PDISKS}[$i]{LGCNT};
            $msg .= sprintf "  PSCNT:                %lu\n", $info{PDISKS}[$i]{PSCNT};
            $msg .= sprintf "  ITCNT:                %lu\n", $info{PDISKS}[$i]{ITCNT};
            $msg .= sprintf "  ICCNT:                %lu\n", $info{PDISKS}[$i]{ICCNT};
            $msg .= sprintf "  MISCOMP:              %lu\n", $info{PDISKS}[$i]{MISCOMP};
            $msg .= sprintf "  DEVMISCOMP:           %lu\n", $info{PDISKS}[$i]{DEVMISCOMP};
            $msg .= sprintf "  RBTOTAL:              $info{PDISKS}[$i]{RBTOTAL}\n";
            $msg .= sprintf "  RBREMAIN:             $info{PDISKS}[$i]{RBREMAIN}\n";
            $msg .= sprintf "  SES:                  $info{PDISKS}[$i]{SES}\n";
            $msg .= sprintf "  SLOT:                 $info{PDISKS}[$i]{SLOT}\n";
            $msg .= sprintf "  HSDNAME:              0x%x\n", $info{PDISKS}[$i]{HSDNAME};
            $msg .= sprintf "  GEO-LOCATION:         %u\n", $info{PDISKS}[$i]{GL_ID};
            $msg .= sprintf "\n";
        }
    }
    elsif (uc($dsptype) eq "PORTS")
    {
        $msg .= sprintf  "Physical Disks ($info{COUNT} disks):\n";
        $msg .= sprintf  "\n";

        $msg .= sprintf "  PID  PORT  LOOPMAP  LUN    LID   ALPA\n";
        $msg .= sprintf "  ---  ----  -------  ----  -----  ----\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf "  %3hu  %4d  %7d  0x%2.2x  0x%3.3x  0x%2.2x\n",
                    $info{PDISKS}[$i]{PD_PID},
                    $info{PDISKS}[$i]{PD_CHANNEL},
                    $info{PDISKS}[$i]{PD_LOOPMAP},
                    $info{PDISKS}[$i]{PD_LUN},
                    $info{PDISKS}[$i]{PD_ID},
                    _getALPA($info{PDISKS}[$i]{PD_ID});
        }
    }
    elsif (uc($dsptype) eq "DEFRAG")
    {
        $msg .= sprintf  "Physical Disks defragging or requiring defrag:\n";
        $msg .= sprintf  "\n";

        $msg .=  "  PID  %REM    RBREMAIN          LAS            TAS     \n";
        $msg .=  "  ---  ----  -------------  -------------  -------------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            my $defragging = $info{PDISKS}[$i]{PD_MISCSTAT} & 0x4;

            if ($info{PDISKS}[$i]{LAS} != $info{PDISKS}[$i]{TAS} or
                $defragging > 0)
            {
                $msg .= sprintf "  %3hu  %4d  %13s  %13s  %13s\n",
                        $info{PDISKS}[$i]{PD_PID},
                        $info{PDISKS}[$i]{PCTREM},
                        $info{PDISKS}[$i]{RBREMAIN},
                        $info{PDISKS}[$i]{LAS},
                        $info{PDISKS}[$i]{TAS};
            }
        }
    }
    
    return $msg;
}


##############################################################################
# Name: displayPhysicalDiskInfo
#
# Desc: Print the physical disk information
#
# In:   Physical Disk Information Hash
##############################################################################
sub displayPhysicalDiskInfo
{
    my ($self, %info) = @_;

    my $msg = "";

    logMsg("begin\n");

    $msg .= sprintf "Physical Disk Information:\n";
    $msg .= sprintf "  CLASS:                %s\n", _getString_CLASS($info{PD_CLASS});
    $msg .= sprintf "  DEVTYPE:              0x%x - %s\n", $info{PD_DEVTYPE}, _getString_PDDT($info{PD_DEVTYPE});
    $msg .= sprintf "  MISCSTAT:             0x%02x\n", $info{PD_MISCSTAT};
    $msg .= sprintf "  PORT:                 %hu\n", $info{PD_CHANNEL};
    $msg .= sprintf "  LOOPMAP:              %hu\n",$info{PD_LOOPMAP};
    $msg .= sprintf "  LUN:                  %hu\n", $info{PD_LUN};
    $msg .= sprintf "  LID:                  0x%x\n", $info{PD_ID};
    $msg .= sprintf "  ALPA:                 0x%x\n", _getALPA($info{PD_ID});
    $msg .= sprintf "  DEV:                  0x%x\n", $info{PD_DEV};
    $msg .= sprintf "  PID:                  %hu\n", $info{PD_PID};
    $msg .= sprintf "  POSTSTAT:             0x%02x\n", $info{PD_POSTSTAT};
    $msg .= sprintf "  DEVSTAT:              0x%02x\n", $info{PD_DEVSTAT};
    $msg .= sprintf "  FLED:                 0x%x\n", $info{PD_FLED};
    $msg .= sprintf "  PCTREM:               %hu\n", $info{PCTREM};
    $msg .= sprintf "  CAPACITY:             $info{CAPACITY}\n";
    $msg .= sprintf "  QD:                   %lu\n", $info{PD_QD};
    $msg .= sprintf "  RPS:                  %lu\n", $info{PD_RPS};
    $msg .= sprintf "  AVGSC:                %lu\n", $info{PD_AVGSC};
    $msg .= sprintf "  SSERIAL:              0x%x\n", $info{PD_SSERIAL};
    $msg .= sprintf "  RREQ:                 $info{RREQ}\n";
    $msg .= sprintf "  WREQ:                 $info{WREQ}\n";
    $msg .= sprintf "  VENDID:               $info{VENDID}\n";
    $msg .= sprintf "  REV:                  $info{PD_REV}\n";
    $msg .= sprintf "  ERR:                  %lu\n", $info{PD_ERR};
    $msg .= sprintf "  PRODID:               $info{PS_PRODID}\n";
    $msg .= sprintf "  SERIAL:               $info{PS_SERIAL}\n";
    $msg .= sprintf "  DAML:                 0x%x\n", $info{PD_DAML};
    $msg .= sprintf "  TAS:                  $info{TAS}\n";
    $msg .= sprintf "  LAS:                  $info{LAS}\n";
    $msg .= sprintf "  WWN:                  %8.8x%8.8x\n", $info{WWN_LO}, $info{WWN_HI};
    $msg .= sprintf "  R10_MISCOMP:          %lu\n", $info{R10_MISCOMP};
    $msg .= sprintf "  DNAME:                0x%x\n", $info{PD_DNAME};
    $msg .= sprintf "  LFCNT:                %lu\n", $info{LFCNT};
    $msg .= sprintf "  LSCNT:                %lu\n", $info{LSCNT};
    $msg .= sprintf "  LGCNT:                %lu\n", $info{LGCNT};
    $msg .= sprintf "  PSCNT:                %lu\n", $info{PSCNT};
    $msg .= sprintf "  ITCNT:                %lu\n", $info{ITCNT};
    $msg .= sprintf "  ICCNT:                %lu\n", $info{ICCNT};
    $msg .= sprintf "  MISCOMP:              %lu\n", $info{MISCOMP};
    $msg .= sprintf "  DEVMISCOMP:           %lu\n", $info{DEVMISCOMP};
    $msg .= sprintf "  RBTOTAL:              $info{RBTOTAL}\n";
    $msg .= sprintf "  RBREMAIN:             $info{RBREMAIN}\n";
    $msg .= sprintf "  SES:                  $info{SES}\n";
    $msg .= sprintf "  SLOT:                 $info{SLOT}\n";
    $msg .= sprintf "  HSDNAME:              0x%x\n", $info{HSDNAME};
# GR_GEORAID - Following is added for GeoRaid
    $msg .= sprintf "  LOCATIONID:           %hu\n", $info{GL_ID};
    $msg .= sprintf "\n";

    return $msg;
}


##############################################################################
# Name: displayPDiskDefragStatus
#
# Desc: Print the physical disk information
#
# In:   Physical Disk Information Hash
##############################################################################
sub displayPDiskDefragStatus
{
    my ($self, %info) = @_;

    my $msg = "";

    logMsg("begin\n");

    if ($info{FLAGS} == 0)
    {
        $msg .= sprintf "Physical Disk Defrag Status:  INACTIVE\n";
    }
    else
    {
        $msg .= sprintf "Physical Disk Defrag Status:  ACTIVE ";

        if ($info{PDISKID} == 0xFFFF)
        {
            $msg .= sprintf "(ALL)\n";
        }
        else
        {
            $msg .= sprintf "(%u)\n", $info{PDISKID};
        }

        $msg .= sprintf "\n";
    }

    return $msg;
}

#SERVICEABILITY
##############################################################################
# Name: displayPDiskAutoFailback
#
# Desc: Print the physical disk autofailback response
#
# In:   Physical Disk Information Hash
##############################################################################
sub displayPDiskAutoFailback
{
    my ($self, %info) = @_;

    my $msg = "";

    logMsg("begin\n");

    if ($info{MODE} == 0)
    {
        $msg .= sprintf "PDisk Auto Failback Disabled\n";
    }
    else
    {
        $msg .= sprintf "PDisk Auto Failback Enabled\n";
    }

    return $msg;
}
#SERVICEABILITY

##############################################################################
#
#   All commands that start with a _ should not be used and are considered
#   private.
#
##############################################################################

##############################################################################
# Name:     _physicalDisksPacket
#
# Desc:     Parses the physical disks packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
#       DISK_ID                 Physical disk id
#       NAME                    Name of disk
#       CLASS                   Values
#                               0 = Unlabeled
#                               1 = Labeled device with redundancy
#                               2 = hotspare
#
#       SYSTEM_SERIAL_NUMBER    San box serial number   (perl BigInt)
#       CURRENT_POST_STATUS
#       DRIVE_STATUS            Values
#                               0 = Nonexistent device
#                               1 = Operational drive
#                               2 = Inoperative drive
#       CAPACITY                Capacity in bytes (perl BigInt)
#       PRODUCT_ID
#       VENDER_ID
#       REVISION
#       SERIAL_NUMBER
#       AVAILABLE_SPACE         In bytes (perl BigInt)
#       LARGEST_AVAIL_SEG       Largest available segment
#       WWN                     Disk WWN    (perl BigInt)
#       LUN
#       CHANNEL
#       LOOPID
#       CONTAINER_ID
##############################################################################
sub _physicalDisksPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_PDISKS_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $info{COUNT}
        ) = unpack("S", $parts{DATA});

        my @pdisks;

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            my $start = 4 + (216 * $i);

            my $rsvd;
            my %capacity;
            my %rreq;
            my %wreq;
            my %tas;
            my %las;
            my %wwn;
            my %rbtotal;
            my %rbremain;

            # Unpack the data
            (
            $rsvd,
            $pdisks[$i]{STATUS_MRP},
            $pdisks[$i]{LEN},
            $pdisks[$i]{PD_CLASS},
            $pdisks[$i]{PD_DEVTYPE},
            $pdisks[$i]{PD_MISCSTAT},
            $rsvd,
            $pdisks[$i]{PD_CHANNEL},
            $pdisks[$i]{PD_LOOPMAP},
            $pdisks[$i]{PD_LUN},
            $pdisks[$i]{PD_ID},

            $pdisks[$i]{PD_DEV},
            $pdisks[$i]{PD_PID},
            $rsvd,
            $pdisks[$i]{PD_POSTSTAT},
            $pdisks[$i]{PD_DEVSTAT},
            $pdisks[$i]{PD_FLED},
            $pdisks[$i]{PCTREM},

            $capacity{LO_LONG}, $capacity{HI_LONG},
            $pdisks[$i]{PD_QD},
            $pdisks[$i]{PD_RPS},

            $pdisks[$i]{PD_AVGSC},
            $pdisks[$i]{PD_SSERIAL},
            $rreq{LO_LONG}, $rreq{HI_LONG},

            $wreq{LO_LONG}, $wreq{HI_LONG},
            $pdisks[$i]{VENDID},

            $pdisks[$i]{PD_REV},
            $pdisks[$i]{PD_ERR},
            $pdisks[$i]{PS_PRODID},
            $pdisks[$i]{PS_SERIAL},
            $pdisks[$i]{PD_DAML},
            $tas{LO_LONG}, $tas{HI_LONG},

            $las{LO_LONG}, $las{HI_LONG},
            $wwn{LO_LONG}, $wwn{HI_LONG},

            $pdisks[$i]{R10_MISCOMP},
            $pdisks[$i]{PD_DNAME},
            $pdisks[$i]{LFCNT},
            $pdisks[$i]{LSCNT},

            $pdisks[$i]{LGCNT},
            $pdisks[$i]{PSCNT},
            $pdisks[$i]{ITCNT},
            $pdisks[$i]{ICCNT},

            $pdisks[$i]{MISCOMP},
            $pdisks[$i]{DEVMISCOMP},
            $rbtotal{LO_LONG}, $rbtotal{HI_LONG},
            $rbremain{LO_LONG}, $rbremain{HI_LONG},

            $pdisks[$i]{SES},
            $pdisks[$i]{SLOT},
            $rsvd,
            $pdisks[$i]{HSDNAME},
# GR_GEORAID - Following is added for GEORaid
            $pdisks[$i]{GL_ID},
            $rsvd
            ) = unpack("a3CLCCCCCCS LLSSCCCC LLLL LLLL LLa8 a4La16a12LLL 
                        LLNN LLLL LLLL LLLL LLSCCLCa8", substr($parts{DATA}, $start));

            # Now fixup all the 64 bit  numbers
            $pdisks[$i]{CAPACITY} = longsToBigInt(%capacity);
            $pdisks[$i]{RREQ} = longsToBigInt(%rreq);
            $pdisks[$i]{WREQ} = longsToBigInt(%wreq);
            $pdisks[$i]{TAS} = longsToBigInt(%tas);
            $pdisks[$i]{LAS} = longsToBigInt(%las);
            $pdisks[$i]{WWN} = longsToBigInt(%wwn);
            $pdisks[$i]{WWN_HI} = $wwn{HI_LONG};
            $pdisks[$i]{WWN_LO} = $wwn{LO_LONG};
            $pdisks[$i]{RBTOTAL} = longsToBigInt(%rbtotal);
            $pdisks[$i]{RBREMAIN} = longsToBigInt(%rbremain);
            
        }

        $info{PDISKS} = [@pdisks];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a physical disk info packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _physicalDiskInfoPacket
#
# Desc:     Parses the physical disk info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
#       DISK_ID                 Physical disk id
#       NAME                    Name of disk
#       CLASS                   Values
#                               0 = Unlabeled
#                               1 = Labeled device with redundancy
#                               2 = hotspare
#
#       SYSTEM_SERIAL_NUMBER    San box serial number   (perl BigInt)
#       CURRENT_POST_STATUS
#       DRIVE_STATUS            Values
#                               0 = Nonexistent device
#                               1 = Operational drive
#                               2 = Inoperative drive
#       CAPACITY                Capacity in bytes (perl BigInt)
#       PRODUCT_ID
#       VENDER_ID
#       REVISION
#       SERIAL_NUMBER
#       AVAILABLE_SPACE         In bytes (perl BigInt)
#       LARGEST_AVAIL_SEG       Largest available segment
#       WWN                     Disk WWN    (perl BigInt)
#       LUN
#       CHANNEL
#       LOOPID
#       CONTAINER_ID
##############################################################################
sub _physicalDiskInfoPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_PDISK_INFO_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        # Temp values for 64 values
        my %capacity;
        my %rreq;
        my %wreq;
        my %tas;
        my %las;
        my %wwn;
        my %rbtotal;
        my %rbremain;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{PD_CLASS},
        $info{PD_DEVTYPE},
        $info{PD_MISCSTAT},
        $rsvd,
        $info{PD_CHANNEL},
        $info{PD_LOOPMAP},
        $info{PD_LUN},

        $info{PD_ID},
        $info{PD_DEV},
        $info{PD_PID},
        $rsvd,
        $info{PD_POSTSTAT},
        $info{PD_DEVSTAT},
        $info{PD_FLED},
        $info{PCTREM},

        $capacity{LO_LONG}, $capacity{HI_LONG},
        $info{PD_QD},
        $info{PD_RPS},

        $info{PD_AVGSC},
        $info{PD_SSERIAL},
        $rreq{LO_LONG}, $rreq{HI_LONG},

        $wreq{LO_LONG}, $wreq{HI_LONG},
        $info{VENDID},

        $info{PD_REV},
        $info{PD_ERR},
        $info{PS_PRODID},
        $info{PS_SERIAL},
        $info{PD_DAML},
        $tas{LO_LONG}, $tas{HI_LONG},

        $las{LO_LONG}, $las{HI_LONG},
        $wwn{LO_LONG}, $wwn{HI_LONG},

        $info{R10_MISCOMP},
        $info{PD_DNAME},
        $info{LFCNT},
        $info{LSCNT},

        $info{LGCNT},
        $info{PSCNT},
        $info{ITCNT},
        $info{ICCNT},

        $info{MISCOMP},
        $info{DEVMISCOMP},
        $rbtotal{LO_LONG}, $rbtotal{HI_LONG},

        $rbremain{LO_LONG}, $rbremain{HI_LONG},
        $info{SES},
        $info{SLOT},
        $rsvd,
        $info{HSDNAME},
# GR_GEORAID - Following is added for GEORaid
        $info{GL_ID},
        $rsvd
        ) = unpack("a3CLCCCCCCS LLSSCCCC LLLL LLLL LLa8 a4La16a12LLL
                    LLNN LLLL LLLL LLLL LLSCCLCa8", $parts{DATA});

        # Now fixup all the 64 bit numbers
        $info{CAPACITY} = longsToBigInt(%capacity);
        $info{RREQ} = longsToBigInt(%rreq);
        $info{WREQ} = longsToBigInt(%wreq);
        $info{TAS} = longsToBigInt(%tas);
        $info{LAS} = longsToBigInt(%las);
        $info{WWN} = longsToBigInt(%wwn);
        $info{WWN_HI} = $wwn{HI_LONG};
        $info{WWN_LO} = $wwn{LO_LONG};
        $info{RBTOTAL} = longsToBigInt(%rbtotal);
        $info{RBREMAIN} = longsToBigInt(%rbremain);
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a physical disk info packet\n");
    }

    return %info;
}


##############################################################################
# Name:     _pDiskDefragStatusPacket
#
# Desc:     Parses the physical disk defrag status packet and places the 
#           information in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
##############################################################################
sub _pDiskDefragStatusPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_PDISK_DEFRAG_STATUS_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        # Temp values for 64 values
        my %capacity;
        my %rreq;
        my %wreq;
        my %tas;
        my %las;
        my %wwn;
        my %rbtotal;
        my %rbremain;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $info{PDISKID},
        $info{FLAGS},
        $rsvd
        ) = unpack("SCC", $parts{DATA});

    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a PDisk Defrag Status packet\n");
    }

    return %info;
}

#SERVICEABILITY42
##############################################################################
# Name:     _pDiskAutoFailbackPacket
#
# Desc:     Parses the physical disk autofailback response packet and places the 
#           information in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
##############################################################################
sub _pDiskAutoFailbackPacket 
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        # Temp values for 64 values

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $info{MODE},
        $rsvd
        ) = unpack("Ca3", $parts{DATA});

    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a PDisk AutoFailback Response packet\n");
    }

    return %info;
}
#SERVICEABILITY42


##############################################################################
# Name: _getString_CLASS
#
# Desc: Gets a string with detailed information given a CLASS number
#       (device class number).
#
# In:   Device Class
#
##############################################################################
sub _getString_CLASS
{
    my ($class) = @_;
    my $fmt;

    if ($class == 0)
    {
        $fmt = "0x%x - UNLABELED";
    }
    elsif ($class == 1)
    {
        $fmt = "0x%x - DATA";
    }
    elsif ($class == 2)
    {
        $fmt = "0x%x - HOTSPARE";
    }
    elsif ($class == 3)
    {
        $fmt = "0x%x - UNSAFE";
    }
    elsif ($class == 4)
    {
        $fmt = "0x%x - MAG LINKED DEVIVCE";
    }
    elsif ($class == 5)
    {
        $fmt = "0x%x - FOREIGN TARGET";
    }
    else
    {
        $fmt = "0x%x";
    }

    return sprintf $fmt, $class;
}

##############################################################################
# Name: _getString_PDDT
#
# Desc: Gets a string with detailed information given a CLASS number
#       (device class number).
#
# In:   Device Type
#
##############################################################################
sub _getString_PDDT
{
    my ($type) = @_;
    my $strType;

    #
    # Based on the device type, return the correct string value.
    # NOTE: The string must be less than 8 characters unless
    #       other code changes to support more than that limit.
    #
    if ($type == PD_DT_FC_DISK)
    {
        $strType = "FC";
    }
    elsif ($type == PD_DT_SATA)
    {
        $strType = "SATA";
    }
    elsif ($type == PD_DT_SSD)
    {
        $strType = "SSD";
    }
    elsif ($type == PD_DT_SAS)
    {
        $strType = "SAS";
    }
    elsif ($type == PD_DT_ECON_ENT)
    {
        $strType = "ECON ENT";
    }
    elsif ($type == PD_DT_FC_SES)
    {
        $strType = "FC SES";
    }
    elsif ($type == PD_DT_SATA_SES)
    {
        $strType = "SATA SES";
    }
    elsif ($type == PD_DT_SBOD_SES)
    {
        $strType = "SBOD SES";
    }
    elsif ($type == PD_DT_SBOD_SAS_EXP)
    {
        $strType = "SBOD SAS EXP";
    }
    else
    {
        $strType = "UNKNOWN";
    }

    return $strType;
}

##############################################################################
# Name: _getString_DNAME
#
# Desc: Gets a string with detailed information given a DNAME (device name).
#
# In:   Device Name
#
##############################################################################
sub _getString_DNAME
{
    my ($dname) = @_;
    my $fmt;
    my $bay;
    my $slot;

    # For names that have 'PD' output the bay and slot
    if (($dname & 0x0000FFFF) == 0x00004450)
    {

        $slot = ($dname & 0xFF000000) >> 24;
        $bay = (($dname & 0x00FF0000) >> 16);

        if ($bay >= 0 && $bay < (26*2))
        {
            $fmt = sprintf('  PD-%s%%02d  ', substr('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz', $bay, 1));
        }
        else
        {
            # Output the name in hex if the bay is invalid - shouldn't happen
            $fmt = "0x%8.8x";
            return sprintf $fmt, $dname;
        }

        return sprintf $fmt, $slot;
    } 

    # For names without a 'PD' output the name in hex
    else
    {
        $fmt = "0x%8.8x";
        return sprintf $fmt, $dname;
    } 

}

##############################################################################
# Name: _getALPA
#
# Desc: Gets an ALPA based upon the LID value.
#
# In:   LID value
#
##############################################################################
sub _getALPA
{
    my ($lid) = @_;
    my $alpa = 0;

    my @lid_to_alpa = (
        0xef,0xe8,0xe4,0xe2,0xe1,0xe0,0xdc,0xda,
        0xd9,0xd6,0xd5,0xd4,0xd3,0xd2,0xd1,0xce,
        0xcd,0xcc,0xcb,0xca,0xc9,0xc7,0xc6,0xc5,
        0xc3,0xbc,0xba,0xb9,0xb6,0xb5,0xb4,0xb3,
        0xb2,0xb1,0xae,0xad,0xac,0xab,0xaa,0xa9,
        0xa7,0xa6,0xa5,0xa3,0x9f,0x9e,0x9d,0x9b,
        0x98,0x97,0x90,0x8f,0x88,0x84,0x82,0x81,
        0x80,0x7c,0x7a,0x79,0x76,0x75,0x74,0x73,
        0x72,0x71,0x6e,0x6d,0x6c,0x6b,0x6a,0x69,
        0x67,0x66,0x65,0x63,0x5c,0x5a,0x59,0x56,
        0x55,0x54,0x53,0x52,0x51,0x4e,0x4d,0x4c,
        0x4b,0x4a,0x49,0x47,0x46,0x45,0x43,0x3c,
        0x3a,0x39,0x36,0x35,0x34,0x33,0x32,0x31,
        0x2e,0x2d,0x2c,0x2b,0x2a,0x29,0x27,0x26,
        0x25,0x23,0x1f,0x1e,0x1d,0x1b,0x18,0x17,
        0x10,0x0f,0x08,0x04,0x02,0x01,0x00,0xff);

    if ($lid < 0x80)
    {
        $alpa = $lid_to_alpa[$lid];
    }

    return $alpa;
}

1;

##############################################################################
# Change log:
# $Log$
# Revision 1.8  2006/07/17 20:38:32  RustadM
# TBolt00014770
# Move 750 branch onto main.
#
# Revision 1.6.2.4  2006/06/27 13:33:25  wirtzs
# TBOLT00000000
# removed devtype PD_DT_STP replaced with PD_DT_SATA
#
# Revision 1.6.2.3  2006/04/26 09:06:20  BharadwajS
# CCBE changes for PI Versioning
#
# Revision 1.6.2.2  2006/04/10 19:11:17  wirtzs
# updates for 750
#
# Revision 1.6.2.1  2006/02/24 14:17:23  MiddenM
#
# Merge from WOOKIEE_EGGS_GA_BR into MODEL750_BR
#
# Revision 1.7  2006/01/12 11:05:28  ChannappaS
# Adding ISCSI and GeoRaid data to SnapDump
#
# Revision 1.6  2005/12/23 08:13:48  BalemarthyS
# Merged ISCSI & GEORAID related changes
#
# Revision 1.5  2005/08/26 14:26:35  MiddenM
# TBolt00013154 - Delete carriage returns, return code for printing disk bay
# letter -- much smaller.
#
# Revision 1.4  2005/08/25 14:26:07  BalemarthyS
# pdiskautofailback status get functionality added
#
# Revision 1.3  2005/07/30 07:15:23  BharadwajS
# X1 Code for Serviceability
#
# Revision 1.2  2005/07/06 05:51:08  BalemarthyS
# Serviceability feature check-in
#
# Revision 1.1.1.1  2005/05/04 18:53:54  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.83  2005/04/12 12:52:30  NigburC
# TBolt00012694 - Updated the resyncmirrors command in the CCBCL and
# CCBE to send down the required request packet.  The command was out
# of data from the platform code.  Also added in the DEFRAG display option
# for the PDISKS request.
#
# Revision 1.82  2005/04/04 20:10:22  WilliamsJ
# TBolt00000000 - adding SBOD support.
#
# Revision 1.81  2005/03/17 16:21:43  NigburC
# TBolt00000000 - Added the "DEFRAG" display type for the "PDISKS"
# command.
#
# Revision 1.80.14.1  2005/03/28 17:31:37  WilliamsJ
# TBolt00000000 - Updated SBOD code.  Reviewed by Chris.
#
# Revision 1.80  2005/02/23 14:57:02  RysavyR
# TBolt00012347: Numerous fixes to SnapDump and underlying CCBE library to clean up errors seen on the snap server.
#
# Revision 1.79  2004/12/06 16:16:23  NigburC
# TBolt00000000 - Added the PORTS display option to PDISKS.
# Reviewed by Jeff Williams.
#
# Revision 1.78  2004/12/03 21:26:33  SchibillaM
# TBolt00011760: Add WWN to the PDisks FWV option.  It was the easiest place to
# put it.
#
# Revision 1.77  2004/08/27 18:29:59  WilliamsJ
# TBolt00000000 - Added LAS != TAS marker for easier defrag detection.
#
# Revision 1.76  2004/08/27 17:53:05  WilliamsJ
# TBolt00000000 - Added LAS != TAS marker for easier defrag detection.
#
# Revision 1.75  2004/08/26 13:16:13  NigburC
# TBolt00011120 - Added the forgotten line of code that gets the error string
# given the value of the status code which is then displayed in the message.
# Also modified the CCBCL/CCBE to allow the pdiskdefrag command to be
# used with no parameters (or STATUS) to display the status of the defrag
# operations.
# Reviewed by Jeff Williams.
#
# Revision 1.74  2004/08/19 20:42:47  SchibillaM
# TBolt00011069: Add support for Defrag Status command.
#
# Revision 1.73  2004/04/29 18:07:17  NigburC
# TBolt00010427 - Added code to support economy enterprise drives.
# Reviewed by Jeff Williams.
#
# Revision 1.72  2004/04/29 15:55:14  NigburC
# TBolt00010427 - Added code to support economy enterprise drives.
# Reviewed by Jeff Williams.
#
# Revision 1.71  2004/04/13 19:37:10  NigburC
# TBolt00000000 - Added the RBREMAIN into the display for PDISKS STD.
#
# Revision 1.70  2004/03/12 16:56:51  McmasterM
# TBolt00010235: Loop monitoring code should not issue log sense commands to SATA drives.
# Minor change to the counter update task to update counters for only FCAL drives.
# This should avoid problems with SATA and SSD devices as well.  I also made
# changes to the 'pdisks loop' CCBE command such that it behaves the same way.
#
# Revision 1.69  2004/02/26 15:44:41  SchibillaM
# TBolt00010148:  Changes to display text for DEV_TYPE.
#
# Revision 1.68  2004/02/24 19:43:49  NigburC
# TBolt00000000 - Added code to display the device type in the DEVSTAT PD
# and PDISKS FWV requests and to the DISKBAYS request.
# Reviewed by Jeff Williams.
#
# Revision 1.67  2004/02/24 19:34:19  NigburC
# TBolt00000000 - Added code to display the device type in the DEVSTAT PD
# and PDISKS FWV requests and to the DISKBAYS request.
# Reviewed by Jeff Williams.
#
# Revision 1.66  2004/02/24 19:19:23  NigburC
# TBolt00000000 - Added code to display the device type in the DEVSTAT PD
# and PDISKS FWV requests.
# Reviewed by Jeff Williams.
#
# Revision 1.65  2004/02/13 17:31:21  RysavyR
# TBolt00000000: Fixed handling of "N/A" fields in fid 291 processing.
#
# Revision 1.64  2003/12/30 19:35:56  SchibillaM
# TBolt00009808: Add the decode of the entire PDISKS hash (type==CMPL) following
# the existing summary data.
#
# Revision 1.63  2003/11/10 22:06:07  McmasterM
# TBolt00000000: Change 'pdisks loop' command to display port letter
#
# Revision 1.62  2003/10/14 14:33:25  McmasterM
# TBolt00009397: Add logic to CCB to gather FCAL counters in background
# Added logic to CCB to collect and process the FCAL counters.  The data is
# stored in several arrays in the CCB DRAM, and are retrievable through the CCBE
# using the command 'fidread 299'.  The snapshot tools and DDR decoder have
# also been modified so that they are able to process the new arrays.
# Portions reviewed by Brett Tollefson
#
# Revision 1.61  2003/09/11 15:28:15  WilliamsJ
# TBolt00009162 and TBolt00008793 - Corrected drives not disappearing in
# a one way configuration.  Also added hot spare labeling changes to allow a
# device that was hot spared to hot spare back when a drive is replaced.
# Reviewed by Chris.
#
# Revision 1.60  2003/09/09 21:02:06  NigburC
# TBolt00000000 - Added code to support a LID to ALPA conversion for LIDS
# greater than 0x7F.  These will return 0x00 for an ALPA value.
# Reviewed by Steve Howe.
#
# Revision 1.59  2003/06/03 19:46:18  MenningC
# TBOLT00000000: Changed many of the 'display' functions to fill a string rather than print to the screen. This supports use by the test scripts. Reviewed by Randy R.
#
# Revision 1.58  2003/05/13 17:48:17  McmasterM
# TBolt00000000: Added ALPA to 'pdisks loop' display
#
# Revision 1.57  2003/05/13 17:40:55  McmasterM
# TBolt00000000: Fixed 'pdisks loop' column formatting
#
# Revision 1.56  2003/05/06 14:47:29  WilliamsJ
# TBolt00000000 - Added ALPA to the display for devstat pd.  Reviewed by Chris.
#
# Revision 1.55  2003/04/29 22:02:31  McmasterM
# TBolt00008141: CSE 2 controller 1 faulted running normal load
# Yet more enhancements to 'pdisks loop' command - now sorted by port, SES, slot,
# and loop ID - accomodates bad SES data better.
#
# Revision 1.54  2003/04/29 14:55:30  McmasterM
# TBolt00008141: CSE 2 controller 1 faulted running normal load
# More enhancements to 'pdisks loop' command - now sorted by port, SES, SLOT
# to accomodate unlabeled drives better.  Had been sorted by DNAME.
#
# Revision 1.53  2003/04/25 19:39:12  TeskeJ
# tbolt00008122 - added the fcal loopmap index into pdisk info and displays
#
# Revision 1.52  2003/04/25 12:47:01  TeskeJ
# tbolt00008122 - format change for 'pdisks loop'
#
# Revision 1.51  2003/04/24 21:49:35  McmasterM
# TBolt00008141: CSE 2 controller 1 faulted running normal load
# More enhancements to 'pdisks loop' command - mainly formatting.
# Noticed issue with sorting by DNAME with unlabled drives, but not fixed.
#
# Revision 1.50  2003/04/24 17:22:29  McmasterM
# TBolt00008141: CSE 2 controller 1 faulted running normal load
# More enhancements to 'pdisks loop' command
#
# Revision 1.49  2003/04/24 14:28:08  McmasterM
# TBolt00008141: CSE 2 controller 1 faulted running normal load
# Changes to display "N/A" fields in LogSense structure.
#
# Revision 1.48  2003/04/23 19:27:33  McmasterM
# TBolt00008141: CSE 2 controller 1 faulted running normal load
# Changed 'pdisk loop' to fetch the loop counters through the log sense page.
# This allows us to get the counters for both loops at once.
#
# Revision 1.47  2003/04/22 13:39:31  TeskeJ
# tbolt00008122 - 'pdisks loop' updates and changed 'channel' to 'port'
# rev by Chris
#
# Revision 1.46  2003/04/15 16:43:27  TeskeJ
# tbolt00007720 - added rebuild % in raidinfo command and format for pdiskinfo
# tbolt00008060 - added SPS in 'raids' command
# rev by Tim
#
# Revision 1.45  2003/04/07 20:38:51  NigburC
# TBolt00007716 - Added optional parameter to pdiskinfo to enable the user
# to specify the OPTIONS value.  Modified the PDISKS command to enable
# the LOOP option to work correctly (actually gets the loop information).
# Reviewed by Craig Menning.
#
# Revision 1.44  2003/03/20 00:30:52  TeskeJ
# tbolt00000000 - pdisks and devstat pd display updates for DName.
# rev by Chris
#
# Revision 1.43  2003/02/28 17:34:48  NigburC
# TBolt00000000 - Added additional display information for DEVSTAT and
# PDISKS commands.
# Reviewed by Jeff Williams.
#
# Revision 1.42  2003/02/27 15:19:05  NigburC
# TBolt00000000 - Added additional display information for PDISKS command.
# Reviewed by Jeff Williams.
#
# Revision 1.41  2003/02/27 15:01:30  NigburC
# TBolt00000000 - Added additional display information for PDISKS command.
# Reviewed by Jeff Williams.
#
# Revision 1.40  2003/02/13 21:40:43  NigburC
# TBolt00007272 - Added the PDISKBYPASS command and it associated
# handler functions.
# Reviewed by Tim Swatosh and Jeff Williams.
#
# Revision 1.39  2003/02/06 16:44:55  TeskeJ
# tbolt00006870 - pdiskinfo display change.
# rev by Tim
#
# Revision 1.38  2002/12/03 22:32:07  NigburC
# TBolt00006359 - Modified the physical disk label code to support the MAG
# style label.  This removed the ability to gang label drives, changed the MRP
# input packet, added a new command handler for the label command, and
# generally made system configuration slower.
# Reviewed by Mark Schibilla.
#
# Revision 1.37  2002/11/19 21:14:55  NigburC
# TBolt00006343 - Modified the PDISK INFO response packet definition to
# match what the PROC is now returning.  The size of the structure did not
# change but the contents did.  The SES and SLOT information changed
# wherein two values became reserved and the names of the others changed.
# Reviewed by Jeff Williams.
#
# Revision 1.36  2002/07/25 21:35:47  NigburC
# TBolt00000000 - Added additional class descriptions for new pdisk class
# types.  Removed the check for matching CLASS types when validating
# physical disks.
# Reviewed by Tim Swatosh.
#
# Revision 1.35  2002/06/20 19:57:34  HoltyB
# Changed PD_ID in displayPhysicalDiskInfo to display in HEX
#
# Revision 1.34  2002/06/14 12:42:30  NigburC
# TBolt00000665 - Added additional command codes and log events that start
# the integration of the power-up and licensing changes.
# Added new option to PDISKS command in CCBE to display firmware/vendor
# information.
#
# Revision 1.33  2002/05/21 15:15:30  HoltyB
# Added ability for pdiskFail to spin down a drive
#
# Revision 1.32  2002/05/08 15:00:35  NigburC
# Tbolt00000000 - Added display type parameter to the displayPhysicalDisks
# function to allow different display information.
#
# Revision 1.31  2002/05/07 20:03:29  NigburC
# TBolt00004034, TBolt00002730 - Added ability for the UMC to connect to
# failed controllers.  They will show up in the controller list even if the connection
# cannot be established and the connection will be re-established as soon as
# possible when the controller is powered up.
# Added ability to fail and unfail controllers (all sides working just not final version
# of the code yet).
# Modified the label MRP to take an array of PIDS and a PIDCNT.
# Removed VCGADDSLAVE and VCGPREPSLAVE from the CCBCL.
#
# Revision 1.30  2002/04/29 21:59:27  HoltyB
# TBolt00004068: Separated DEVDELETE into two seperate commands
# PDISKDELETE and DISKBAYDELETE and removed DEVDELETE
#
# Revision 1.29  2002/04/15 19:20:06  NigburC
# TBolt00003771 - Modified the display routines for PDISKS and DISKBAYS to
# provide useful information in a readable format.
# Reviewed by Mark Schibilla.
#
# Revision 1.28  2002/03/27 13:11:13  SchibillaM
# TBolt00003487: Add support for Get BE Port List and Break VLink lock.
# Reviewed by Randy.
#
# Revision 1.27  2002/03/25 16:59:16  HoltyB
# TBolt00003442: Added support for SNMP configuration on the ccb to allow
# ip addresses to be sent to the ccb to be used for generating traps
#
# Revision 1.26  2002/03/20 20:47:15  NigburC
# TBolt00000000 - The pdiskrestore command was using the wrong packet
# handler.  It was using the _statusResponsePacket instead of the
# _genericResponsePacket.
#
# Revision 1.25  2002/03/11 13:22:59  HoltyB
# Added an extra parameter for physicalDiskFail
# bit 2 = 1 = Write a fail label on the drive
#
# Revision 1.24  2002/02/26 21:16:29  NigburC
# TBolt00000000 - Added check in the PDISKLABEL command to default
# the value of serial number if it is not passed into the function.  This will make
# the code backward compatible with scripts that do not know about the
# new parameter.  Also removed the serial number from the checked
# parameters.
#
# Revision 1.23  2002/02/26 20:52:11  NigburC
# TBolt00000159 - Added additional code to start retrieving statistics information
# in bulk.
#
# Also added in the new PI_PDISKS command to retrieve information for all
# pdisks in one PI request (multiple MRPs).
#
# Revision 1.22  2002/02/11 19:08:03  HoltyB
# moved command codes to PI_CommandCodes.pm
#
# Revision 1.21  2002/02/05 23:20:09  NigburC
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
# Revision 1.20  2001/12/20 22:33:53  NigburC
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
# Revision 1.19  2001/12/11 19:41:43  NigburC
# Added the duration parameter to PDISKBEACON.
#
# Revision 1.18  2001/12/10 22:11:42  NigburC
# Added the UNSAFE type to the function that returns the string text for the
# class of physical disk.
#
# Revision 1.17  2001/12/05 16:21:53  NigburC
# Fixed the display of the WWN values.
#
# Revision 1.16  2001/12/03 22:05:24  NigburC
# Fixed the retrieval and display of the VENDID and PD_REV.  They were
# supposed to be string values.  We need to change the MRP.H structure
# to show them as being strings.
#
# Revision 1.15  2001/12/03 15:10:32  NigburC
# Added PDISKFAIL and PDISKRESTORE commands.
#
# Revision 1.14  2001/11/28 16:12:21  NigburC
# Added many additional command handlers...
# TARGET commands
# SERVER commands
# Many others...
# Replaced LOGIN and LOGOUT with CONNECT and DISCONNECT
#
# Revision 1.13  2001/11/14 12:57:37  NigburC
# Modified verify parameters to return a hash instead of just the message.
# This will allow the code to return it to the command line where it will be
# processed like any other error, except there is a MESSAGE in the
# hash which will be displayed describing the parameter error that occurred.
#
# Revision 1.12  2001/11/13 23:02:12  NigburC
# Modified the command line interface to always expect a response packet
# returned from the commands called in the command manager.  This will
# enable the command line to check the STATUS to determine if the command
# was good or bad and then interrogate the ERROR_CODE to determine what
# the underlying error really was.
#
# Revision 1.11  2001/11/13 15:40:49  NigburC
# Switched the usage of LO_LONG and HI_LONG when dealing with
# 64 bit integers.  I had them reversed.
#
# Revision 1.10  2001/11/12 22:07:11  NigburC
# Finished pdisklable.
# Fixed other problems in pdisklist and pdiskcount.
#
# Revision 1.9  2001/11/09 21:56:55  NigburC
# Fixed the unpack of the physical disk info packet, I was missing a "V".
#
# Revision 1.8  2001/11/08 14:03:34  NigburC
# Changed to use the generic count and list response packet handlers.
# Changed code for pdiskinfo to handle new packet interface (not working yet).
#
# Revision 1.7  2001/11/07 22:19:06  NigburC
# Removed the code for the 3007 port.
#
# Revision 1.6  2001/11/07 13:51:13  NigburC
# Change the usage of the ID tag to HEADER tag.
#
# Revision 1.5  2001/11/05 20:53:07  NigburC
# More cleanup work.
# - Moved environmental statistics code to cmStats.pm.
# - Added encrypt/decrypt functionality (stubs).
#
# Revision 1.4  2001/11/02 20:35:35  NigburC
# Code cleanup time, lots of changes.
#
# Added code handling for the 3100 port and new packet interface.  BigfootCL
# is now able to connect to a machine running the new packet interface and
# send a pdiskCount command.
#
# Revision 1.3  2001/10/31 19:51:36  NigburC
# Modified the display routine for a physical disk information to display the
# information cleaner.
#
# Revision 1.2  2001/10/31 15:42:02  NigburC
# Updated the command line to include the "logInfo" command to display
# the last N log messages.
#
# Revision 1.1.1.1  2001/10/31 12:51:30  NigburC
# Initial integration of Bigfoot command line.
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
