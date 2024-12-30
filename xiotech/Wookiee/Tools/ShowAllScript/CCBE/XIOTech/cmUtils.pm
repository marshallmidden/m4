# $Id: cmUtils.pm 161676 2013-09-18 14:06:02Z marshall_midden $
##############################################################################
# Copyright (c) 2001-2008  Xiotech Corporation. All rights reserved.
# ======================================================================
# Author: Chris Nigbur
#
# Purpose:
#   Wrapper for all the different XIOTech commands that can be sent
#   to the Xiotech SAN system
##############################################################################
package XIOTech::cmdMgr;

use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;

use XIOTech::bigNums;
use XIOTech::error;

use XIOTech::logMgr;

use XIOTech::decodeSupport;
use XIOTech::fmtFIDs;

use strict;

my @msgStatus;
# $msgStatus[PI_SOCKET_ERROR]         = "Socket communication error.";                                # -1
$msgStatus[PI_GOOD]                 = "Status is good";                                             # 0
$msgStatus[PI_ERROR]                = "Error occurred";                                             # 1
$msgStatus[PI_IN_PROGRESS]          = "Command in progress when completion expected";               # 2
$msgStatus[PI_TIMEOUT]              = "Timeout limit exceeded";                                     # 4
$msgStatus[PI_INVALID_CMD_CODE]     = "Invalid command code specified";                             # 5
$msgStatus[PI_MALLOC_ERROR]         = "Malloc Error";                                               # 6
$msgStatus[PI_PARAMETER_ERROR]      = "Invalid Parameter error";                                    # 7
$msgStatus[PI_MASTER_CNT_ERROR]     = "You are not the master error";                               # 8
$msgStatus[PI_POWER_UP_REQ_ERROR]   = "Power-up sequencing has not completed and is required.";     # 9
$msgStatus[PI_ELECTION_ERROR]       = "Election in progress, command cannot be completed.";         # 10
$msgStatus[PI_TUNNEL_ERROR]         = "Tunnel error.";                                              # 11
$msgStatus[PI_R5_STRIPE_RESYNC_ERROR] = "Raid 5 Stripe Resync error.";                              # 12
$msgStatus[PI_LOCAL_RAID_RESYNC_ERROR] = "Local Raid Resync error.";                                # 13
$msgStatus[PI_INVALID_PACKETVERSION_ERROR] = "Invalid Packet Version error.";                       # 14
$msgStatus[PI_COMPAT_INDEX_NOT_SUPPORTED] = "Compatibility Index not supported error.";             # 15



##############################################################################
# Name:     getStatusMsg
#
# Desc:     Returns an error string based on the status value
#
# Input:    STATUS - status value
#
# Returns:  String with the status message
##############################################################################
sub getStatusMsg
{
    my ($self, $status) = @_;

    my $msg;
    if ($status == PI_SOCKET_ERROR)
    {
        $msg = "Socket communication error.";
    }
    elsif (defined($msgStatus[$status]))
    {
        $msg = $msgStatus[$status];
    }
    else
    {
        $msg = "Unknown status.";
    }

    return $msg;
}

##############################################################################
# Name:     getObjectCount
#
# Desc:     Retrieves the count of a specific object.
#
# Input:    COMMAND CODE - Command code for which count of objects
#
# Returns:  Hash with the count of the objects.
##############################################################################
sub getObjectCount
{
    my ($self, $cmd) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["getObjectCount"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_countResponsePacket);
}

##############################################################################
# Name:     getObjectList
#
# Desc:     Retrieves an array containing the identifiers of an object.
#
# Input:    COMMAND CODE - Command code for which list of objects
#
# Returns:  List of object identifiers or UNDEF if an error occurred.
##############################################################################
sub getObjectList
{
    my ($self, $cmd) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["getObjectList"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_listResponsePacket);
}

##############################################################################
# Name:     getPortList
#
# Desc:     Retrieves an array containing the identifiers of an port.
#
# Input:    COMMAND CODE - Command code for which list of objects
#
# Returns:  List of object identifiers or UNDEF if an error occurred.
##############################################################################
sub getPortList
{
    my ($self, $cmd, $type) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ["getPortList"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data = pack("Sa2",
                    $type,
                    0
                    );

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_portListResponsePacket);
}

##############################################################################
# Name:     getSos
#
# Desc:     Retrieves an array containing the identifiers of an port.
#
# Input:    COMMAND CODE - Command code for which list of objects
#
# Returns:  List of object identifiers or UNDEF if an error occurred.
##############################################################################
sub getSos
{
    my ($self, @parms) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['i'],
                ["getSos"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmdCode = PI_GENERIC_GET_SOS_STRUCTURE;

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LLL64L",
                    $cmdCode,
                    0,
                    0,
                    @parms);

    my $packet = assembleXiotechPacket(PI_GENERIC_CMD,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_sosResponsePacket);
}

##############################################################################
# Name:     batteryHealthSet
#
# Desc:     Sets the battery health for a given board.
#
# In:       Battery Board and Health State
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub batteryHealthSet
{
    my ($self, $board, $state) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF],
                ['d', 0, 0xFF],
                ["batteryHealth"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_BATTERY_HEALTH_SET_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $board,
                    0,
                    $state,
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
# Name:     deviceCount
#
# Desc:     Get the device count for the given serial number.
#
# In:       serial_number
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub deviceCount
{
    my ($self, $sn) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ["deviceCount"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_MISC_GET_DEVICE_COUNT_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("L", $sn);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_deviceCountResponsePacket);
}

##############################################################################
# Name:     deviceName
#
# Desc:     Set or retrieve a name of a device.
#
# In:       ID - Identifier of a device
#           Option - Device name option
#           Name - Name for the device
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub deviceName
{
    my ($self, $id, $option, $name) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0x0003],
                ['s'],
                ["deviceName"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PROC_NAME_DEVICE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SSLLLa16",
                    $option,
                    $id,
                    0,0,0,
                    $name);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_deviceNameResponsePacket);
}

##############################################################################
# Name:     deviceConfigGet
#
# Desc:     Retrieves the device configuration information.
#
# In:
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
##############################################################################
sub deviceConfigGet
{
    my ($self) = @_;
    my @devices;
    my $i;
    my $iFlags;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["deviceConfigGet"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_MISC_GETDEVCONFIG_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_deviceConfigGetResponsePacket);
}

##############################################################################
# Name:     deviceConfigSet
#
# Desc:     Updates the device configuration information.
#
# In:
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
##############################################################################
sub deviceConfigSet
{
    my ($self, $inputDevices) = @_;
    my @devices;
    my $i;
    my $iFlags;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['a'],
                ["deviceConfigSet"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_MISC_PUTDEVCONFIG_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    @devices = @$inputDevices;

    my $data = pack("SCC", scalar(@devices), 0, 0);

    for ($i = 0; $i < scalar(@devices); $i++)
    {
        $data .= pack("a8a16",
                        $devices[$i]{DEVVENDOR},
                        $devices[$i]{DEVPRODID});

        for ($iFlags = 0; $iFlags < 8; $iFlags++)
        {
            $data .= pack("C", $devices[$i]{DEVFLAGS}[$iFlags]);
        }
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
# Name:     deviceStatus
#
# Desc:     Retrieves the device status information
#
# Input:    DEV - Device information type
#
# Return:   _deviceStatus hash
##############################################################################
sub deviceStatus
{
    my ($self, $dev) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['s'],
                ["deviceStatus"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my %info;
    my $i;

    $info{STATUS} = PI_GOOD;
    $info{ERROR_CODE} = 0;

    if (uc($dev) eq "PD")
    {
        my %pdlist = $self->physicalDisks();

        if (%pdlist && $pdlist{STATUS} == PI_GOOD)
        {
            for ($i = 0; $i < $pdlist{COUNT}; $i++)
            {
                $info{LIST}[$i]{STATUS_MRP}     = $pdlist{PDISKS}[$i]{STATUS_MRP};
                $info{LIST}[$i]{LEN}            = $pdlist{PDISKS}[$i]{LEN};
                $info{LIST}[$i]{PD_CLASS}       = $pdlist{PDISKS}[$i]{PD_CLASS};
                $info{LIST}[$i]{PD_DEVTYPE}     = $pdlist{PDISKS}[$i]{PD_DEVTYPE};
                $info{LIST}[$i]{PD_MISCSTAT}    = $pdlist{PDISKS}[$i]{PD_MISCSTAT};
                $info{LIST}[$i]{PD_CHANNEL}     = $pdlist{PDISKS}[$i]{PD_CHANNEL};
                $info{LIST}[$i]{PD_LOOPMAP}     = $pdlist{PDISKS}[$i]{PD_LOOPMAP};
                $info{LIST}[$i]{PD_LUN}         = $pdlist{PDISKS}[$i]{PD_LUN};
                $info{LIST}[$i]{PD_ID}          = $pdlist{PDISKS}[$i]{PD_ID};
                $info{LIST}[$i]{PD_DEV}         = $pdlist{PDISKS}[$i]{PD_DEV};
                $info{LIST}[$i]{PD_PID}         = $pdlist{PDISKS}[$i]{PD_PID};
                $info{LIST}[$i]{PD_POSTSTAT}    = $pdlist{PDISKS}[$i]{PD_POSTSTAT};
                $info{LIST}[$i]{PD_DEVSTAT}     = $pdlist{PDISKS}[$i]{PD_DEVSTAT};
                $info{LIST}[$i]{PD_FLED}        = $pdlist{PDISKS}[$i]{PD_FLED};
                $info{LIST}[$i]{PCTREM}         = $pdlist{PDISKS}[$i]{PCTREM};
                $info{LIST}[$i]{CAPACITY}       = $pdlist{PDISKS}[$i]{CAPACITY};
                $info{LIST}[$i]{PD_QD}          = $pdlist{PDISKS}[$i]{PD_QD};
                $info{LIST}[$i]{PD_RPS}         = $pdlist{PDISKS}[$i]{PD_RPS};
                $info{LIST}[$i]{PD_AVGSC}       = $pdlist{PDISKS}[$i]{PD_AVGSC};
                $info{LIST}[$i]{PD_SSERIAL}     = $pdlist{PDISKS}[$i]{PD_SSERIAL};
                $info{LIST}[$i]{RREQ}           = $pdlist{PDISKS}[$i]{RREQ};
                $info{LIST}[$i]{WREQ}           = $pdlist{PDISKS}[$i]{WREQ};
                $info{LIST}[$i]{VENDID}         = $pdlist{PDISKS}[$i]{VENDID};
                $info{LIST}[$i]{PD_REV}         = $pdlist{PDISKS}[$i]{PD_REV};
                $info{LIST}[$i]{PD_ERR}         = $pdlist{PDISKS}[$i]{PD_ERR};
                $info{LIST}[$i]{PS_PRODID}      = $pdlist{PDISKS}[$i]{PS_PRODID};
                $info{LIST}[$i]{PS_SERIAL}      = $pdlist{PDISKS}[$i]{PS_SERIAL};
                $info{LIST}[$i]{PD_DAML}        = $pdlist{PDISKS}[$i]{PD_DAML};
                $info{LIST}[$i]{TAS}            = $pdlist{PDISKS}[$i]{TAS};
                $info{LIST}[$i]{LAS}            = $pdlist{PDISKS}[$i]{LAS};
                $info{LIST}[$i]{WWN_HI}         = $pdlist{PDISKS}[$i]{WWN_HI};
                $info{LIST}[$i]{WWN_LO}         = $pdlist{PDISKS}[$i]{WWN_LO};
                $info{LIST}[$i]{R10_MISCOMP}    = $pdlist{PDISKS}[$i]{R10_MISCOMP};
                $info{LIST}[$i]{PD_DNAME}       = $pdlist{PDISKS}[$i]{PD_DNAME};
                $info{LIST}[$i]{LFCNT}          = $pdlist{PDISKS}[$i]{LFCNT};
                $info{LIST}[$i]{LSCNT}          = $pdlist{PDISKS}[$i]{LSCNT};
                $info{LIST}[$i]{LGCNT}          = $pdlist{PDISKS}[$i]{LGCNT};
                $info{LIST}[$i]{PSCNT}          = $pdlist{PDISKS}[$i]{PSCNT};
                $info{LIST}[$i]{ITCNT}          = $pdlist{PDISKS}[$i]{ITCNT};
                $info{LIST}[$i]{ICCNT}          = $pdlist{PDISKS}[$i]{ICCNT};
                $info{LIST}[$i]{MISCOMP}        = $pdlist{PDISKS}[$i]{MISCOMP};
                $info{LIST}[$i]{DEVMISCOMP}     = $pdlist{PDISKS}[$i]{DEVMISCOMP};
                $info{LIST}[$i]{RBTOTAL}        = $pdlist{PDISKS}[$i]{RBTOTAL};
                $info{LIST}[$i]{RBREMAIN}       = $pdlist{PDISKS}[$i]{RBREMAIN};

                $info{LIST}[$i]{SES}            = $pdlist{PDISKS}[$i]{SES};
                $info{LIST}[$i]{SLOT}           = $pdlist{PDISKS}[$i]{SLOT};
                $info{LIST}[$i]{GL_ID}          = $pdlist{PDISKS}[$i]{GL_ID};
            }
        }
        else
        {
            if (%pdlist)
            {
                $info{STATUS} = $pdlist{STATUS};
                $info{ERROR_CODE} = $pdlist{ERROR_CODE};
                $info{MESSAGE} = "Unable to retrieve physical disks.";
            }
            else
            {
                return;
                $info{STATUS} = PI_ERROR;
                $info{ERROR_CODE} = 0;
                $info{MESSAGE} = "Failed to receive response packet.";
            }
        }
    }
    elsif (uc($dev) eq "VD")
    {
        my $ri;

        my %vdlist = $self->virtualDisks();

        if (%vdlist && $vdlist{STATUS} == PI_GOOD)
        {
            for ($i = 0; $i < $vdlist{COUNT}; $i++)
            {
                $info{LIST}[$i]{STATUS_MRP}     = $vdlist{VDISKS}[$i]{STATUS_MRP};
                $info{LIST}[$i]{LEN}            = $vdlist{VDISKS}[$i]{LEN};
                $info{LIST}[$i]{VID}            = $vdlist{VDISKS}[$i]{VID};
                $info{LIST}[$i]{ATTR}           = $vdlist{VDISKS}[$i]{ATTR};
                $info{LIST}[$i]{DEVSTAT}        = $vdlist{VDISKS}[$i]{DEVSTAT};
                $info{LIST}[$i]{SCORVID}        = $vdlist{VDISKS}[$i]{SCORVID};
                $info{LIST}[$i]{SCPCOMP}        = $vdlist{VDISKS}[$i]{SCPCOMP};
                $info{LIST}[$i]{RAIDCNT}        = $vdlist{VDISKS}[$i]{RAIDCNT};
                $info{LIST}[$i]{CAPACITY}       = $vdlist{VDISKS}[$i]{CAPACITY};
                $info{LIST}[$i]{ERROR}          = $vdlist{VDISKS}[$i]{ERROR};
                $info{LIST}[$i]{QD}             = $vdlist{VDISKS}[$i]{QD};
                $info{LIST}[$i]{RPS}            = $vdlist{VDISKS}[$i]{RPS};
                $info{LIST}[$i]{AVGSC}          = $vdlist{VDISKS}[$i]{AVGSC};
                $info{LIST}[$i]{RREQ}           = $vdlist{VDISKS}[$i]{RREQ};
                $info{LIST}[$i]{WREQ}           = $vdlist{VDISKS}[$i]{WREQ};
                $info{LIST}[$i]{CACHEEN}        = $vdlist{VDISKS}[$i]{CACHEEN};
                $info{LIST}[$i]{MIRROR}         = $vdlist{VDISKS}[$i]{MIRROR};
                $info{LIST}[$i]{DRAIDCNT}       = $vdlist{VDISKS}[$i]{DRAIDCNT};
                $info{LIST}[$i]{SPRC}           = $vdlist{VDISKS}[$i]{SPRC};
                $info{LIST}[$i]{SPSC}           = $vdlist{VDISKS}[$i]{SPSC};
                $info{LIST}[$i]{SCHEAD}         = $vdlist{VDISKS}[$i]{SCHEAD};
                $info{LIST}[$i]{SCTAIL}         = $vdlist{VDISKS}[$i]{SCTAIL};
                $info{LIST}[$i]{CPSCMT}         = $vdlist{VDISKS}[$i]{CPSCMT};
                $info{LIST}[$i]{VLINKS}         = $vdlist{VDISKS}[$i]{VLINKS};
                $info{LIST}[$i]{NAME}           = $vdlist{VDISKS}[$i]{NAME};

                for ($ri = 0; $ri < $vdlist{VDISKS}[$i]{RAIDCNT}; $ri++)
                {
                    $info{LIST}[$i]{RIDS}[$ri] = $vdlist{VDISKS}[$i]{RIDS}[$ri];
                }
            }
        }
        else
        {
            if (%vdlist)
            {
                $info{STATUS} = $vdlist{STATUS};
                $info{ERROR_CODE} = $vdlist{ERROR_CODE};
                $info{MESSAGE} = "Unable to retrieve virtual disk list.";
            }
            else
            {
                return;
                $info{STATUS} = PI_ERROR;
                $info{ERROR_CODE} = 0;
                $info{MESSAGE} = "Failed to receive response packet.";
            }
        }
    }
    elsif (uc($dev) eq "RD")
    {
        my $pi;

        my %rdlist = $self->raids();

        if (%rdlist && $rdlist{STATUS} == PI_GOOD)
        {
            for ($i = 0; $i < $rdlist{COUNT}; $i++)
            {
                my %rsp = $self->virtualDiskInfo($rdlist{RAIDS}[$i]{VID});

                if (%rsp)
                {
                    if ($rsp{STATUS} == PI_GOOD)
                    {
                        $info{LIST}[$i]{VID_STATUS} = $rsp{DEVSTAT};
                    }
                    else
                    {
                        $info{LIST}[$i]{VID_STATUS} = "NONE";
                    }
                }
                else
                {
                    $info{LIST}[$i]{VID_STATUS} = "NONE";
                }

                $info{LIST}[$i]{STATUS_MRP}     = $rdlist{RAIDS}[$i]{STATUS_MRP};
                $info{LIST}[$i]{LEN}            = $rdlist{RAIDS}[$i]{LEN};
                $info{LIST}[$i]{RID}            = $rdlist{RAIDS}[$i]{RID};
                $info{LIST}[$i]{TYPE}           = $rdlist{RAIDS}[$i]{TYPE};
                $info{LIST}[$i]{DEVSTAT}        = $rdlist{RAIDS}[$i]{DEVSTAT};
                $info{LIST}[$i]{DEPTH}          = $rdlist{RAIDS}[$i]{DEPTH};
                $info{LIST}[$i]{PCTREM}         = $rdlist{RAIDS}[$i]{PCTREM};
                $info{LIST}[$i]{PSDCNT}         = $rdlist{RAIDS}[$i]{PSDCNT};
                $info{LIST}[$i]{SPS}            = $rdlist{RAIDS}[$i]{SPS};
                $info{LIST}[$i]{SPU}            = $rdlist{RAIDS}[$i]{SPU};
                $info{LIST}[$i]{CAPACITY}       = $rdlist{RAIDS}[$i]{CAPACITY};
                $info{LIST}[$i]{NVRDD}          = $rdlist{RAIDS}[$i]{NVRDD};
                $info{LIST}[$i]{VID}            = $rdlist{RAIDS}[$i]{VID};
                $info{LIST}[$i]{FRCNT}          = $rdlist{RAIDS}[$i]{FRCNT};
                $info{LIST}[$i]{ERROR}          = $rdlist{RAIDS}[$i]{ERROR};
                $info{LIST}[$i]{QD}             = $rdlist{RAIDS}[$i]{QD};
                $info{LIST}[$i]{RPS}            = $rdlist{RAIDS}[$i]{RPS};
                $info{LIST}[$i]{AVGSC}          = $rdlist{RAIDS}[$i]{AVGSC};
                $info{LIST}[$i]{RREQ}           = $rdlist{RAIDS}[$i]{RREQ};
                $info{LIST}[$i]{WREQ}           = $rdlist{RAIDS}[$i]{WREQ};
                $info{LIST}[$i]{LLSDA}          = $rdlist{RAIDS}[$i]{LLSDA};
                $info{LIST}[$i]{LLEDA}          = $rdlist{RAIDS}[$i]{LLEDA};
                $info{LIST}[$i]{IPROCS}         = $rdlist{RAIDS}[$i]{IPROCS};
                $info{LIST}[$i]{IERRORS}        = $rdlist{RAIDS}[$i]{IERRORS};
                $info{LIST}[$i]{ISECTORS}       = $rdlist{RAIDS}[$i]{ISECTORS};
                $info{LIST}[$i]{MISCOMP}        = $rdlist{RAIDS}[$i]{MISCOMP};
                $info{LIST}[$i]{PARDRV}         = $rdlist{RAIDS}[$i]{PARDRV};
                $info{LIST}[$i]{DEFLOCK}        = $rdlist{RAIDS}[$i]{DEFLOCK};
                $info{LIST}[$i]{ASTATUS}        = $rdlist{RAIDS}[$i]{ASTATUS};
                $info{LIST}[$i]{SPRC}           = $rdlist{RAIDS}[$i]{SPRC};
                $info{LIST}[$i]{SPSC}           = $rdlist{RAIDS}[$i]{SPSC};
                $info{LIST}[$i]{RPNHEAD}        = $rdlist{RAIDS}[$i]{RPNHEAD};


                for ($pi = 0; $pi < $rdlist{RAIDS}[$i]{PSDCNT}; $pi++)
                {
                    $info{LIST}[$i]{PIDS}[$pi]{PID} = $rdlist{RAIDS}[$i]{PIDS}[$pi]{PID};
                    $info{LIST}[$i]{PIDS}[$pi]{PSD_STATUS} = $rdlist{RAIDS}[$i]{PIDS}[$pi]{PSD_STATUS};
                    $info{LIST}[$i]{PIDS}[$pi]{PSD_ASTATUS} = $rdlist{RAIDS}[$i]{PIDS}[$pi]{PSD_ASTATUS};
                }
            }
        }
        else
        {
            if (%rdlist)
            {
                $info{STATUS} = $rdlist{STATUS};
                $info{ERROR_CODE} = $rdlist{ERROR_CODE};
                $info{MESSAGE} = "Unable to retrieve raid device list.";
            }
            else
            {
                return;
                $info{STATUS} = PI_ERROR;
                $info{ERROR_CODE} = 0;
                $info{MESSAGE} = "Failed to receive response packet.";
            }
        }
    }

    return %info;
}

##############################################################################
# Name:     failureStateSet
#
# Desc:     Set the failure state of a controller.
#
# Input:    serialNumber - Serial number of the controller
#           state - failure state for the controller
#
# Return:   Status Response Hash
##############################################################################
sub failureStateSet
{
    my ($self, $serialNumber, $state) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 11],
                ["failureStateSet"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_MISC_FAILURE_STATE_SET_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LL",
                    $serialNumber,
                    $state);

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
# Name:     fwVersion
#
# Desc:     Retrieves the firmware version
#
# Input:    NONE
#
# Return:   _fwHeaderPacket hash
##############################################################################
sub fwVersion
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["fwVersion"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my @fwtypes;
    my %info;
    $info{STATUS} = PI_GOOD;
    $info{ERROR_CODE} = 0;


    if ($self->{CONTROLLER_TYPE} != CTRL_TYPE_BIGFOOT)
    {
        @fwtypes = ( FW_VER_CCB_RUNTIME,
                     FW_VER_BE_RUNTIME,
                     FW_VER_FE_RUNTIME );
    }
    else # CTRL_TYPE_BIGFOOT
    {
        @fwtypes = ( FW_VER_CCB_RUNTIME,
                     FW_VER_CCB_BOOT,
                     FW_VER_BE_RUNTIME,
                     FW_VER_BE_BOOT,
                     FW_VER_BE_DIAG,
                     FW_VER_FE_RUNTIME,
                     FW_VER_FE_BOOT,
                     FW_VER_FE_DIAG);
    }

    my $cmd = PI_ADMIN_FW_VERSIONS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    for (my $i = 0; $i < scalar(@fwtypes); ++$i)
    {
        my $type = $fwtypes[$i];
        my $data = pack("S", $type);
        my $packet = assembleXiotechPacket($cmd,
                                            $seq,
                                            $ts,
                                            $data,
                                            $self->{PORT}, VERSION_1);

        my %rsp = $self->_handleSyncResponse($seq,
                                                $packet,
                                                \&_fwHeaderPacket);

        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                $info{$type}{PRODUCT_ID} = $rsp{PRODUCT_ID};
                $info{$type}{TARGET} = $rsp{TARGET};
                $info{$type}{REVISION} = $rsp{REVISION};
                $info{$type}{REV_COUNT} = $rsp{REV_COUNT};
                $info{$type}{BUILD_ID} = $rsp{BUILD_ID};
                $info{$type}{SYSTEM_RLS} = $rsp{SYSTEM_RLS};
                $info{$type}{TS_YEAR} = $rsp{TS_YEAR};
                $info{$type}{TS_MONTH} = $rsp{TS_MONTH};
                $info{$type}{TS_DATE} = $rsp{TS_DATE};
                $info{$type}{TS_DAY} = $rsp{TS_DAY};
                $info{$type}{TS_HOURS} = $rsp{TS_HOURS};
                $info{$type}{TS_MINUTES} = $rsp{TS_MINUTES};
                $info{$type}{TS_SECONDS} = $rsp{TS_SECONDS};
            }
            else
            {
                $info{STATUS} = $rsp{STATUS};
                $info{ERROR_CODE} = $rsp{ERROR_CODE};
                $info{MESSAGE} = "Unable to retrieve FW version information ($type).";
                last;
            }
        }
        else
        {
            $info{STATUS} = PI_ERROR;
            $info{ERROR_CODE} = 0;
            $info{MESSAGE} = "Failed to receive response packet ($type).";
            last;
        }
    }

    return %info;
}


##############################################################################
# Name:     getfwVersion
#
# Desc:     Retrieves and stores the firmware version
#
# Input:    NONE
#
# Return:   NONE
##############################################################################
sub storefwVersion($)
{
    my ($self) = @_;

    logMsg("begin\n");
    my $version = 0;
    my $cmd = PI_ADMIN_FW_VERSIONS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

        my $type = FW_VER_CCB_RUNTIME;
        my $data = pack("S", $type);
        my $packet = assembleXiotechPacket($cmd,
                                            $seq,
                                            $ts,
                                            $data,
                                            $self->{PORT}, VERSION_1);

        my %rsp = $self->_handleSyncResponse($seq,
                                                $packet,
                                                \&_fwHeaderPacket);

        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                $version = $rsp{SYSTEM_RLS};
            }
        }


    $self->{Controller_FW_Version} = $version
}



##############################################################################
# Name:     FWSystemRelease
#
# Desc:     Retrieves the firmware system release version
#
# Input:    NONE
#
# Return:   _fwHeaderPacket hash
##############################################################################
sub FWSystemRelease
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["FWSystemRelease"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_ADMIN_FW_SYS_REL_LEVEL_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                       $seq,
                                       $ts,
                                       undef,
                                       $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                      $packet,
                                      \&_fwSysRelPacket);
}

##############################################################################
# Name:     RefreshCCBCahe
#
# Desc:     Retrieves the firmware system release version
#
# Input:    NONE
#
# Return:   _fwHeaderPacket hash
##############################################################################
sub RefreshCCBCahe
{
    my ($self, $cacheMask, $waitForCompletion) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 1],
                ["RefreshCCBCahe"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_CACHE_REFRESH_CCB_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LCCCC",
                    $cacheMask,
                    $waitForCompletion, 0, 0, 0);

    my $packet = assembleXiotechPacket($cmd,
                                       $seq,
                                       $ts,
                                       $data,
                                       $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                      $packet,
                                      \&_genericResponsePacket);
}

#ifdef ENGINEERING
##############################################################################
# Name:     timeSet
#
# Desc:     sync controller clock to local PC.
#
# Input:    systime - system time in Epoch seconds
#
# Return:   Status Response Hash
##############################################################################
sub timeSet
{
    my ($self, $systime) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ["timeSet"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_ADMIN_SETTIME_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LL",
                    $systime,
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
#endif

##############################################################################
# Name:     GetTime
#
# Desc:     get controller time.
#
# Input:    None
#
# Return:   systime - system time in Epoch seconds
##############################################################################
sub GetTime
{
    my ($self) = @_;

    logMsg("begin\n");

    my $cmd = PI_ADMIN_GETTIME_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_systimePacket);
}

#ifdef ENGINEERING
##############################################################################
# Name:     ipSet
#
# Desc:     set new ip, subnet and gateway.
#
# Input:    ipAdr       - long ip address
#           subNetAdr   - long subnet mask ip address
#           gateWayAdr  - long gateway ip address
#
# Return:   generic Response Hash
##############################################################################
sub ipSet
{
    my ($self, $serNum, $ipAdr, $subNetAdr, $gateWayAdr) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFFFFFFFF],
                ["ipSet"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_ADMIN_SET_IP_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LLLL",
                    $serNum,
                    $ipAdr,
                    $subNetAdr,
                    $gateWayAdr);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}
#endif

##############################################################################
# Name:     ipGet
#
# Desc:     Retrieve IP address/network information.
#
# Input:    none
#
# Returns:
##############################################################################
sub ipGet
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["ipGet"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_ADMIN_GET_IP_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_ipInfoPacket);
}

##############################################################################
# Name:     ledSet
#
# Desc:     set LED on controller.
#
# Input:    led - led to set
#           value  - value to set led (0 or 1)
#
# Return:   Status Response Hash
##############################################################################
sub ledSet
{
    my ($self, $led, $value, $blink) = @_;
    my $args;
    
    logMsg("begin\n");

    if (!defined($blink))
    {
        # verify parameters
        $args = [['i'],
                 ['d', 0, 0],
                 ['d', 0, 1],
                 ["ledSet"]];
        $blink = 0;
    }
    else
    {
        # verify parameters
        $args = [['i'],
                 ['d', 0, 0xFF],
                 ['d', 0, 3],
                 ['i'],
                 ["ledSet"]];
    }

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_ADMIN_LEDCNTL_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $led, $value, $blink,
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
# Name: _getString_POWERUP
#
# Desc: Gets a string with detailed information given a power up state.
#
# In:   Power-Up State
#
##############################################################################
sub _getString_POWERUP
{
    my ($state) = @_;
    my $strState = "";

    if ($state == POWER_UP_UNKNOWN)
    {
        $strState = "POWER_UP_UNKNOWN";
    }
    elsif ($state == POWER_UP_START)
    {
        $strState = "POWER_UP_START";
    }
    elsif ($state == POWER_UP_WAIT_FWV_INCOMPATIBLE)
    {
        $strState = "POWER_UP_WAIT_FWV_INCOMPATIBLE";
    }
    elsif ($state == POWER_UP_WAIT_PROC_COMM)
    {
        $strState = "POWER_UP_WAIT_PROC_COMM";
    }
    elsif ($state == POWER_UP_WAIT_CONFIGURATION)
    {
        $strState = "POWER_UP_WAIT_CONFIGURATION";
    }
    elsif ($state == POWER_UP_WAIT_LICENSE)
    {
        $strState = "POWER_UP_WAIT_LICENSE";
    }
    elsif ($state == POWER_UP_WAIT_DRIVES)
    {
        $strState = "POWER_UP_WAIT_DRIVES";
    }
    elsif ($state == POWER_UP_DISCOVER_CONTROLLERS)
    {
        $strState = "POWER_UP_DISCOVER_CONTROLLERS";
    }
    elsif ($state == POWER_UP_WAIT_CONTROLLERS)
    {
        $strState = "POWER_UP_WAIT_CONTROLLERS";
    }
    elsif ($state == POWER_UP_WAIT_DISASTER)
    {
        $strState = "POWER_UP_WAIT_DISASTER";
    }
    elsif ($state == POWER_UP_PROCESS_BE_INIT)
    {
        $strState = "PROCESS_BE_INIT";
    }
    elsif ($state == POWER_UP_PROCESS_DISCOVERY)
    {
        $strState = "PROCESS_DISCOVERY";
    }
    elsif ($state == POWER_UP_WAIT_DISK_BAY)
    {
        $strState = "POWER_UP_WAIT_DISK_BAY";
    }
    elsif ($state == POWER_UP_WAIT_CORRUPT_BE_NVRAM)
    {
        $strState = "POWER_UP_WAIT_CORRUPT_BE_NVRAM";
    }
    elsif ($state == POWER_UP_ALL_CTRL_BE_READY)
    {
        $strState = "POWER_UP_ALL_CTRL_BE_READY";
    }
    elsif ($state == POWER_UP_PROCESS_R5_RIP)
    {
        $strState = "POWER_UP_PROCESS_R5_RIP";
    }
    elsif ($state == POWER_UP_SIGNAL_SLAVES_RUN_FE)
    {
        $strState = "POWER_UP_SIGNAL_SLAVES_RUN_FE";
    }
    elsif ($state == POWER_UP_PROCESS_CACHE_INIT)
    {
        $strState = "POWER_UP_PROCESS_CACHE_INIT";
    }
    elsif ($state == POWER_UP_WAIT_CACHE_ERROR)
    {
        $strState = "POWER_UP_WAIT_CACHE_ERROR";
    }
    elsif ($state == POWER_UP_INACTIVE)
    {
        $strState = "POWER_UP_INACTIVE";
    }
    elsif ($state == POWER_UP_FAILED)
    {
        $strState = "POWER_UP_FAILED";
    }
    elsif ($state == POWER_UP_WRONG_SLOT)
    {
        $strState = "POWER_UP_WRONG_SLOT";
    }
    elsif ($state == POWER_UP_COMPLETE)
    {
        $strState = "POWER_UP_COMPLETE";
    }
    else
    {
        $strState = "UNKNOWN";
    }

    return sprintf $strState;
}

##############################################################################
# Name: _getString_AdditionalPowerUp
#
# Desc: Gets a string with detailed information given a power up state.
#
# In:   Power-Up State
#
##############################################################################
sub _getString_AdditionalPowerUp
{
    my ($state) = @_;
    my $strState = "";

    if ($state == POWER_UP_ASTATUS_UNKNOWN)
    {
        $strState = "No additional state available";
    }
    elsif ($state == POWER_UP_ASTATUS_WC_SEQNO_BAD)
    {
        $strState = "Write Cache, Bad Sequence Number";
    }
    elsif ($state == POWER_UP_ASTATUS_WC_SN_VCG_BAD)
    {
        $strState = "Write Cache, Bad VCG Serial Number";
    }
    elsif ($state == POWER_UP_ASTATUS_WC_SN_BAD)
    {
        $strState = "Write Cache, Bad Serial Number";
    }
    elsif ($state == POWER_UP_ASTATUS_WC_NVMEM_BAD)
    {
        $strState = "Write Cache, Bad NVMEM";
    }
    else
    {
        $strState = "UNKNOWN";
    }

    return sprintf $strState;
}

##############################################################################
# Name:     powerUpState
#
# Desc:     Gets the current power-up state of the controller.
#
# Inputs:   NONE
##############################################################################
sub powerUpState
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["powerUpState"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_POWER_UP_STATE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_powerUpStateResponsePacket);
}

##############################################################################
# Name:     powerUpResponse
#
# Desc:     Sends a response to a power-up "wait" state.
#
# Inputs:   serial number of controller
##############################################################################
sub powerUpResponse
{
    my ($self, $state, $astatus, $response) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFF],
                ["powerUpResponse"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_POWER_UP_RESPONSE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SSCCCC",
                    $state,
                    $astatus,
                    $response,
                    0, 0, 0);


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
# Name:     rmState
#
# Desc:     Gets the current resource manager state for the controller.
#
# Inputs:   NONE
##############################################################################
sub rmState
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["rmState"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_DEBUG_GET_STATE_RM_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_stateResponsePacket);
}

##############################################################################
# Name:     mfgClean
#
# Desc:     Cleans the controller.
#
# Input:    Option for how to clean the controller.
#
# Return:   Generic Response Hash
##############################################################################
sub mfgClean
{
    my ($self, $option) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF],
                ["mfgClean"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my %rsp;
    $rsp{STATUS} = PI_GOOD;
    $rsp{ERROR_CODE} = 0;

    my $cmd = PI_MFG_CTRL_CLEAN_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    0,
                    0,
                    0,
                    $option);

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
# Name:     resetProcessor
#
# Desc:     Resets the specified processor(s).
#
# Input:    Which processor (one of: CCB, FE, BE or ALL)
#
# Return:   Generic Response Hash
##############################################################################
sub resetProcessor
{
    my ($self, $processor, $type) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFF],
                ["resetProcessor"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc = 0;

    my %rsp;
    $rsp{STATUS} = PI_GOOD;
    $rsp{ERROR_CODE} = 0;

    my $cmd = PI_RESET_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LCCCC",
                    $processor,
                    $type,
                    0,
                    0,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    %rsp = $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);

    if (%rsp && $rsp{STATUS} == PI_GOOD && $type == 2)
    {
        $self->logout();

        # Sleep for 60 second(s) before attempting to reconnect.
        sleep(60);

        my $host = $self->{HOST};
        my $port = $self->{PORT};

        while ($rc == 0)
        {
            # Sleep for 5 second(s) before attempting to connect.
            sleep(5);

            # Attempt to reconnect the socket
            $rc = $self->login($host, $port);
        }
    }

    return %rsp;
}

##############################################################################
# Name:     stopIO
#
# Desc:     stop IO on this controller
#
# Input:    none
#
# Return:   Status Response Hash
##############################################################################
sub stopIO
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["stopIO"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PROC_STOP_IO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    0x05,
                    0,
                    0x42,
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
# Name:     startIO
#
# Desc:     start IO on this controller
#
# Input:    none
#
# Return:   Status Response Hash
##############################################################################
sub startIO
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["startIO"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PROC_START_IO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    0,
                    0,
                    0x42,
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
# Name:     assignMirrorPartner
#
# Desc:     resets Qlogic.
#
# Input:    chan  -  Channel to reset
#
# Return:   Status Response Hash
##############################################################################
sub assignMirrorPartner
{
    my ($self, $serial_number) = @_;

    my $cmd;
    my $seq;
    my $ts;
    my $data;
    my $packet;
    my %info;
    my %rspIO;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ["assignMirrorPartner"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    %rspIO = $self->stopIO();

    $cmd = PI_PROC_ASSIGN_MIRROR_PARTNER_CMD;
    $seq = $self->{SEQ}->nextId();
    $ts = $self->{SEQ}->nextTimeStamp();
    $data = pack("L", $serial_number);

    $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    %info = $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_assignMirrorPartnerResponsePacket);

    %rspIO = $self->startIO();

    return %info;
}

##############################################################################
# Name:     resetQlogicFE
#
# Desc:     resets Qlogic.
#
# Input:    chan  -  Channel to reset
#
# Return:   Status Response Hash
##############################################################################
sub resetQlogicFE
{
    my ($self, $chan, $option) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF],
                ['d', 0, 0xFFFF],
                ["resetQlogicFE"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PROC_RESET_FE_QLOGIC_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $chan,
                    0,
                    $option,
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
# Name:     loopPrimitiveBE
#
# Desc:     Back end loop primitive command.
#
# Input:    option      - loop primitive option
#                           LP_RESET_LOOP           0x0000
#                           LP_RESET_LID_PORT       0x0001
#                           LP_SID_PID_RESET        0x0002
#                           LP_LOGIN_LID            0x0011
#                           LP_LOGIN_PID            0x0012
#                           LP_LOGOUT_LID           0x0021
#                           LP_LOGOUT_PID           0x0022
#
#           id          - pid
#
#           port
#
#           lid
#
# Return:   Status Response Hash
##############################################################################
sub loopPrimitiveBE
{
    my ($self, $option, $id, $port, $lid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0x0004],
                ['d', 0, 0xFFFF],
                ["loopPrimitiveBE"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_DEBUG_BE_LOOP_PRIMITIVE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data = pack("SSCCCCL",
                    $option,
                    $id,
                    $port,
                    0, 0, 0,
                    $lid);

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
# Name:     loopPrimitiveFE
#
# Desc:     Front end loop primitive command.
#
# Input:    option      - loop primitive option
#                           LP_RESET_LOOP           0x0000
#                           LP_RESET_LID_PORT       0x0001
#                           LP_SID_PID_RESET        0x0002
#                           LP_LOGIN_LID            0x0011
#                           LP_LOGIN_PID            0x0012
#                           LP_LOGOUT_LID           0x0021
#                           LP_LOGOUT_PID           0x0022
#
#           id          - pid
#
#           port
#
#           lid
#
# Return:   Status Response Hash
##############################################################################
sub loopPrimitiveFE
{
    my ($self, $option, $id, $port, $lid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0x0004],
                ['d', 0, 0xFFFF],
                ["loopPrimitiveFE"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_DEBUG_FE_LOOP_PRIMITIVE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data = pack("SSCCCCL",
                    $option,
                    $id,
                    $port,
                    0, 0, 0,
                    $lid);

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
# Name:     rescanDevice
#
# Desc:     Rescans the devices.
#
# Input:    NONE
#
# Return:   Status Response Hash
##############################################################################
sub rescanDevice
{
    my ($self, $type) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['s'],
                ["rescanDevice"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    if (uc($type) eq "LIST")
    {
        $type = RESCAN_EXISTING;
    }
    elsif (uc($type) eq "LUNS")
    {
        $type = RESCAN_LUNS;
    }
    else
    {
        $type = RESCAN_REDISCOVER;
    }

    my $cmd = PI_MISC_RESCAN_DEVICE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $type,
                    0,
                    0,
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
# Name:     resyncCtl
#
# Desc:     Submit the resync control operation.
#
# Input:    NONE
#
# Return:   Status Response Hash
##############################################################################
sub resyncCtl
{
    my ($self, $fc, $rid, $csn, $gid, $name) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFFFFFFFF],
                ['s'],
                ["resyncCtl"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_MISC_RESYNCCTL_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCCLLLa16",
                    $fc,
                    0,0,0,
                    $rid,
                    $csn,
                    $gid,
                    $name);

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
# Name:     resyncData
#
# Desc:     Retrieve the resync data.
#
# Input:    NONE
#
# Return:   Status Response Hash
##############################################################################
sub resyncData
{
    my ($self, $format) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ["resyncData"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_MISC_RESYNCDATA_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCCCCCC",
                    $format,
                    0,0,0,0,0,0,0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_resyncDataResponsePacket);
}

##############################################################################
# Name:     resyncMirrors
#
# Desc:     Resync the mirrors.
#
# Input:    NONE
#
# Return:   Status Response Hash
##############################################################################
sub resyncMirrors
{
    my ($self, $type, $rid) = @_;

    logMsg("begin\n");

    # verify parameters
    #
    # NOTE: The type of "5" is not supported through the CCBE requests.
    my $args = [['i'],
                ['d', 1, 4],
                ['d', 0, 0xFFFF],
                ["resyncMirrors"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_MISC_RESYNC_MIRROR_RECORDS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCS",
                    $type,
                    0,
                    $rid);

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
# Name:     resetQlogicBE
#
# Desc:     resets Qlogic.
#
# Input:    chan  -  Channel to reset
#
# Return:   Status Response Hash
##############################################################################
sub resetQlogicBE
{
    my ($self, $chan, $option) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF],
                ['d', 0, 0xFFFF],
                ["resetQlogicBE"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PROC_RESET_BE_QLOGIC_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $chan,
                    0,
                    $option,
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
# Name:     beDevicePaths
#
# Desc:     retrieves backend device paths.
#
# Input:    type   -  type of device
#           format -  format of return
#
# Return:   Status Response Hash
##############################################################################
sub beDevicePaths
{
    my ($self, $type, $format) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xF],
                ['d', 0, 0xF],
                ["beDevicePaths"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PROC_BE_DEVICE_PATH_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $type,
                    $format
                    );

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_devicePathResponsePacket);
}

##############################################################################
# Name:     deviceList
#
# Desc:     retrieves device list.
#
# Input:    type  -  Proceesor to get list from
#           chan  -  Channel to retrieve list
#
# Return:   Status Response Hash
##############################################################################
sub deviceList
{
    my ($self, $type, $chan) = @_;
    my $cmd;
    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['s'],
                ['d', 0, 0xFF],
                ["deviceList"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    if (uc($type) eq "BE")
    {
        $cmd = PI_MISC_GET_BE_DEVICE_LIST_CMD;
    }
    else
    {
        $cmd = PI_MISC_GET_FE_DEVICE_LIST_CMD;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $chan,
                    0,
                    0,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_deviceListResponsePacket);
}

##############################################################################
# Name:     initProcNVRAM
#
# Desc:     Initialzie PROC NVRAM
#
# Input:    type    type of init to do
#                       init all nvram                      0x00
#                       init FE NVA Records ONLY            0x01
#                       init NMI counts ONLY                0x02
#                       init BE NVA Records ONLY            0x04
#
# Return:   _statusResponsePacket hash
##############################################################################
sub initProcNVRAM
{
    my ($self, $type) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0x04],
                ["initProcNVRAM"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_DEBUG_INIT_PROC_NVRAM_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $type,
                    0,
                    0,
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
# Name:     initCCBNVRAM
#
# Desc:     Initialzie CCB NVRAM
#
# Input:    type    type of init to do
#                       init all nvram                      0x00
#                       init FE NVA Records ONLY            0x01
#                       init NMI counts ONLY                0x02
#
# Return:   _statusResponsePacket hash
##############################################################################
sub initCCBNVRAM
{
    my ($self, $type) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0x1],
                ["initCCBNVRAM"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_DEBUG_INIT_CCB_NVRAM_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $type,
                    0,
                    0,
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
# Name:     ReadMemory()
#
# Desc:     Read processor memory
#
# Input:    address
#           data
#           processor
#
# Returns:  hash with STATUS, LENGTH, RD_DATA
##############################################################################
sub ReadMemory
{
    my ($self, $address, $length, $proc) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ['d', 1, 0xFFFFFFFF],
                ['s'],
                ["ReadMemory"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $procType;
    if ($proc =~ /CCB/i) {
        $procType = 0;
    }
    elsif ($proc =~ /FE/i) {
        $procType = 1;
    }
    else { # BE
        $procType = 2;
    }

    my $allData;
    my $orgLength = $length & 0x7fffffff;
    my %retData;

    while ($length & 0x7ffffff) {
        my $cmd = PI_DEBUG_MEM_RDWR_CMD;
        my $seq = $self->{SEQ}->nextId();
        my $ts = $self->{SEQ}->nextTimeStamp();
        my $reqData = pack("LLSS", $address, $length, $procType, 1);

        my $packet = assembleXiotechPacket($cmd,
                $seq,
                $ts,
                $reqData,
                $self->{PORT}, VERSION_1);

        %retData = $self->_handleSyncResponse($seq,
                $packet,
                \&_readMemResponsePacket);

        if($retData{STATUS} == 0) {
            $allData .= $retData{RD_DATA};
        }
        else {
            return %retData;
        }

        $length -= $retData{LENGTH};
        $address += $retData{LENGTH};
    }

    $retData{LENGTH} = $orgLength;
    $retData{RD_DATA} = $allData;
    return %retData;
}


##############################################################################
# Name:     MPXReadMemory()
#
# Desc:     Read processor memory
#
# Input:    address
#           data
#           processor
#
# Returns:  hash with STATUS, LENGTH, RD_DATA
##############################################################################
sub MPXReadMemory
{
    my ($self, $address, $length, $proc) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ['d', 1, 0xFFFFFFFF],
                ['s'],
                ["ReadMemory"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $procType;
    if ($proc =~ /CCB/i) {
        $procType = 0;
    }
    elsif ($proc =~ /FE/i) {
        $procType = 1;
    }
    else { # BE
        $procType = 2;
    }
    $procType <<= 1;

    my $x;
    my $n = 999;
    my %retData;
    my $cmd = PI_MULTI_PART_XFER_CMD;
    my $fileData;

    for($x = 1; $x <= $n; $x++) {

        my $data = pack("CCCCLL", MPX_MEMIO_SCMD,     # subCmdCode;
                                  $x,                 # partX
                                  $n == 999 ? 0 : $n, # ofN
                                  $procType,          # flags
                                  $address,           # parm1
                                  $length);           # parm2

        my $seq = $self->{SEQ}->nextId();
        my $ts = $self->{SEQ}->nextTimeStamp();

        my $packet = assembleXiotechPacket($cmd,
                $seq,
                $ts,
                $data,
                $self->{PORT}, VERSION_1);

        %retData = $self->_handleSyncResponse($seq,
                                          $packet,
                                          \&_readMpxResponsePacket);

        if ($retData{STATUS} == PI_GOOD) {
            $n = $retData{OFN};
            $fileData .= $retData{RD_DATA};
            $retData{RD_DATA} = $fileData;
            printf "%2u/$n) OK\r", $x;
        }
        else {
            printf "%2u/$n) BAD (MPXReadMemory)\r", $x;
            undef $retData{RD_DATA};
            last;
        }
    }
    print "\n\n";

    return %retData;
}

##############################################################################
# Name:     WriteMemory()
#
# Desc:     Write processor memory
#
# Input:    address
#           data
#           processor
#
# Returns:  hash with STATUS field
##############################################################################
sub WriteMemory
{
    my ($self, $address, $data, $proc) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ['i'],
                ['s'],
                ["WriteMemory"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $procType;
    if ($proc =~ /CCB/i) {
        $procType = 0;
    }
    elsif ($proc =~ /FE/i) {
        $procType = 1;
    }
    else { # BE
        $procType = 2;
    }

    my $cmd = PI_DEBUG_MEM_RDWR_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $reqData = pack("LLSS", $address, length $data, $procType, 2);
    $reqData .= $data; # tack on the data payload.

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $reqData,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}

##############################################################################
# Name:     MPXWriteMemory()
#
# Desc:     Write File ID
#
# Input:    fid
#           binary data
#
# Returns:  hash with STATUS
##############################################################################
sub MPXWriteMemory
{
    my ($self, $address, $buffer, $proc) = @_;

    my $rc = 1;

    logMsg("MPXWriteMemory...begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ['i'],
                ['s'],
    ["MPXWriteMemory"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $procType;
    if ($proc =~ /CCB/i) {
        $procType = 0;
    }
    elsif ($proc =~ /FE/i) {
        $procType = 1;
    }
    else { # BE
        $procType = 2;
    }
    $procType <<= 1;

    my $fLen = length($buffer);
    my $x;
    my $n = int(($fLen + MPX_MAX_TX_DATA_SIZE - 1) / MPX_MAX_TX_DATA_SIZE);
    my %hash;

    for($x = 1; $x <= $n; $x++) {

        my $data = pack("CCCCLL", MPX_MEMIO_SCMD,       # subCmdCode;
                                  $x,                   # partX
                                  $n,                   # ofN
                                  MPX_WRITE+$procType,  # flags
                                  $address,             # parm1
                                  $fLen);               # parm2

        my $seq = $self->{SEQ}->nextId();
        my $ts = $self->{SEQ}->nextTimeStamp();
        my $cmd = PI_MULTI_PART_XFER_CMD;
        my $sendData = $data . substr($buffer, 0, MPX_MAX_TX_DATA_SIZE);

        if(length($buffer) > MPX_MAX_TX_DATA_SIZE) {
            $buffer = substr($buffer, MPX_MAX_TX_DATA_SIZE);
        }

        my $packet = assembleXiotechPacket($cmd,
                $seq,
                $ts,
                $sendData,
                $self->{PORT}, VERSION_1);

        %hash = $self->_handleSyncResponse($seq,
                $packet,
                \&_genericResponsePacket);

        if ($hash{STATUS} == PI_GOOD) {
            printf "%2u/$n) OK \r", $x;
        }
        else {
            printf "%2u/$n) BAD (MPXWriteMemory)\r", $x;
            last;
        }
    }
    print "\n\n";
    return %hash;
}


##############################################################################
# Name:     ReadFID()
#
# Desc:     Read File ID
#
# Input:    fid
#           length
#
# Returns:  hash with STATUS, LENGTH, RD_DATA
##############################################################################
sub ReadFID
{
    my ($self, $fid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 23],
                ["ReadFID"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my %retData;

    my $cmd = PI_MISC_FILE_SYSTEM_READ_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $reqData = pack("L", $fid);

    my $packet = assembleXiotechPacket($cmd,
                                       $seq,
                                       $ts,
                                       $reqData,
                                       $self->{PORT}, VERSION_1);

    %retData = $self->_handleSyncResponse($seq,
                                          $packet,
                                          \&_readFIDResponsePacket);

    return %retData;
}

##############################################################################
# Name:     MPXReadFID()
#
# Desc:     Read File ID
#
# Input:    fid
#           length
#
# Returns:  hash with STATUS, LENGTH, RD_DATA
##############################################################################
sub MPXReadFID
{
    my ($self, $fid) = @_;
    my $fh;
    my %retData;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0x400 + 6],
                ["MPXReadFID"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    # Linux FileRead rsvd fids
    if ( ($fid >= 0x100) && ($fid < 0x200) &&
         (($fid & 0xFF) >= 0x31) &&
         (($fid & 0xFF) <= 0x60) )
    {
        if ( !open($fh, ">ccbLinFileBfr.dat") )
        {
            $retData{STATUS} = PI_ERROR;
            return;
        }
        else
        {
            binmode $fh, ":raw";
            $fh->autoflush(1);
        }
    }
    else
    {
        undef $fh;
    }


    my $x;
    my $n = 999;
    my $fLength = 0;
    my $cmd = PI_MULTI_PART_XFER_CMD;
    my $fileData;

    for($x = 1; $x <= $n; $x++) {

        my $data = pack("CCCCLL", MPX_FILEIO_SCMD,    # subCmdCode;
                                  $x,                 # partX
                                  $n == 999 ? 0 : $n, # ofN
                                  0,                  # flags
                                  $fid,               # parm1
                                  0);                 # parm2

        my $seq = $self->{SEQ}->nextId();
        my $ts = $self->{SEQ}->nextTimeStamp();

        my $packet = assembleXiotechPacket($cmd,
                $seq,
                $ts,
                $data,
                $self->{PORT}, VERSION_1);

        %retData = $self->_handleSyncResponse($seq,
                                          $packet,
                                          \&_readMpxResponsePacket);

        if ($retData{STATUS} == PI_GOOD) {
            $x = $retData{PARTX};
            $n = $retData{OFN};
            if (defined($fh))
            {
                print $fh $retData{RD_DATA};
            }
            else
            {
                $fileData .= $retData{RD_DATA};
                $retData{RD_DATA} = $fileData;
            }
#            printf "%2u/$n) OK\r", $x;
        }
        else {
            printf "%2u/$n) BAD (MPXReadFID)\r", $x;
            undef $retData{RD_DATA};
            last;
        }
    }

    if (defined $fh)
    {
        close $fh;
        if ($retData{STATUS} == PI_GOOD)
        {
            undef $retData{RD_DATA};
            $retData{RD_DATA} = "ccbLinFileBfr.dat";
        }
    }

#    print "\n\n";

    return %retData;
}


##############################################################################
# Name:     MPXWriteFID()
#
# Desc:     Write File ID
#
# Input:    fid
#           binary data
#
# Returns:  hash with STATUS
##############################################################################
sub MPXWriteFID
{
    my ($self, $fid, $buffer, $noHdrFlag) = @_;

    my $rc = 1;

    logMsg("MPXWriteFID...begin\n");

    # verify parameters
    my $args = [['i'],
#                ['d', 0, 0xFF],
                ['i'],
    ["MPXWriteFID"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $fLen = length($buffer);
    my $x;
    my $n = int(($fLen + MPX_MAX_TX_DATA_SIZE - 1) / MPX_MAX_TX_DATA_SIZE);
    my %hash;
    my $flags = MPX_WRITE;

    if (defined($noHdrFlag) and $noHdrFlag)
    {
        $flags |= MPX_WRITE_NO_HDR;
    }

    for($x = 1; $x <= $n; $x++) {

        my $data = pack("CCCCLL", MPX_FILEIO_SCMD, # subCmdCode;
                                  $x,           # partX
                                  $n,           # ofN
                                  $flags,       # flags
                                  $fid,         # parm1
                                  $fLen);       # parm2

        my $seq = $self->{SEQ}->nextId();
        my $ts = $self->{SEQ}->nextTimeStamp();
        my $cmd = PI_MULTI_PART_XFER_CMD;
        my $sendData = $data . substr($buffer, 0, MPX_MAX_TX_DATA_SIZE);

        if(length($buffer) > MPX_MAX_TX_DATA_SIZE) {
            $buffer = substr($buffer, MPX_MAX_TX_DATA_SIZE);
        }

        my $packet = assembleXiotechPacket($cmd,
                $seq,
                $ts,
                $sendData,
                $self->{PORT}, VERSION_1);

        %hash = $self->_handleSyncResponse($seq,
                $packet,
                \&_genericResponsePacket);

        if ($hash{STATUS} == PI_GOOD) {
            printf "%2u/$n) OK \r", $x;
        }
        else {
            printf "%2u/$n) BAD (MPXWriteFID)\r", $x;
            last;
        }
    }
    print "\n\n";
    return %hash;
}


##############################################################################
# Name:     UnfailInterface()
#
# Desc:     Unfail an interface
#
# Input:    controllerSN
#           interface
#
# Returns:  hash with STATUS, LENGTH, RD_DATA
##############################################################################
sub UnfailInterface
{
    my ($self, $controllerSN, $interface) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 3],
                ["UnfailInterface"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $reqData = pack("LCCCC", $controllerSN, $interface, 0, 0, 0);

    my $packet = assembleXiotechPacket(PI_MISC_UNFAIL_INTERFACE_CMD,
                                       $seq,
                                       $ts,
                                       $reqData,
                                       $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                      $packet,
                                      \&_genericResponsePacket);
}


##############################################################################
# Name:     FailInterface()
#
# Desc:     Fail an interface
#
# Input:    controllerSN
#           interface
#
# Returns:  hash with STATUS, LENGTH, RD_DATA
##############################################################################
sub FailInterface
{
    my ($self, $controllerSN, $interface) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 3],
                ["UnfailInterface"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $reqData = pack("LCCCC", $controllerSN, $interface, 0, 0, 0);

    my $packet = assembleXiotechPacket(PI_MISC_FAIL_INTERFACE_CMD,
                                       $seq,
                                       $ts,
                                       $reqData,
                                       $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                      $packet,
                                      \&_genericResponsePacket);
}

##############################################################################
# Name:     serialNumGet
#
# Desc:     Retreive a serial number for this controller.
#
# Input:    NONE
#
# Returns:
##############################################################################
sub serialNumGet
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["serialNumGet"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my %info;
    $info{STATUS} = PI_GOOD;
    $info{ERROR_CODE} = 0;

    my $cmd = PI_DEBUG_GET_SER_NUM_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    for (my $i = 1; $i <= 2; ++$i)
    {
        my $data = pack("L", $i);
        my $packet = assembleXiotechPacket($cmd,
                                            $seq,
                                            $ts,
                                            $data,
                                            $self->{PORT}, VERSION_1);

        my %rsp = $self->_handleSyncResponse($seq,
                                                $packet,
                                                \&_serialNumGetPacket);

        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                $info{$i}{SERIAL_NUM} = $rsp{SERIAL_NUM};
            }
            else
            {
                $info{STATUS} = $rsp{STATUS};
                $info{ERROR_CODE} = $rsp{ERROR_CODE};
                $info{MESSAGE} = "Unable to serial numbers ($i).";
                last;
            }
        }
        else
        {
            $info{STATUS} = PI_ERROR;
            $info{ERROR_CODE} = 0;
            $info{MESSAGE} = "Failed to receive response packet ($i).";
            last;
        }
    }

    return %info;
}

##############################################################################
# Name:     serialNumSet
#
# Desc:     Sets the system serial number for this controller.
#
# Input:    serial number
#
# Returns:
##############################################################################
sub serialNumSet
{
    my ($self, $serialNum, $which) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ['d', 1, 2],
                ["serialNumSet"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_MISC_SERIAL_NUMBER_SET_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCCL", $which, 0, 0, 0, $serialNum);

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
# Name:     scrubInfo
#
# Desc:     Retrieves the scrubbing information.
#
# Input:    NONE
#
# Return:   Scrubbing Information Packet Hash
##############################################################################
sub scrubInfo
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["scrubInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_RAID_CONTROL_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data = pack("LLSS",
                    SCRUB_POLL,
                    0,
                    0,
                    0);

#    my $data = pack("CCCCLSS",
#                    SCRUB_POLL,
#                    0,
#                    0,
#                    0,
#                    0,
#                    0,
#                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_scrubInfoPacket);
}

##############################################################################
# Name:     scrubSet
#
# Desc:     Set the scrubbing options.
#
# Input:    NONE
#
# Return:   Scrubbing Information Packet Hash
##############################################################################
sub scrubSet
{
    my ($self, $scrubcontrol, $paritycontrol, $raidid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFFFF],
                ["scrubSet"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_RAID_CONTROL_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data = pack("LLSS",
                    $scrubcontrol,
                    $paritycontrol,
                    $raidid,
                    0);

#    my $data = pack("CCCCLSS",
#                    $scrubcontrol,
#                    0,
#                    0,
#                    0,
#                    $paritycontrol,
#                    $raidid,
#                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_scrubInfoPacket);
}

##############################################################################
# Name:     timeoutMRP
#
# Desc:     Set the MRP global timeout.
#
# Input:    DURATION - Time in seconds for the timeout
#
# Return:   Generic Response Packet Hash
##############################################################################
sub timeoutMRP
{
    my ($self, $type, $duration) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['s'],
                ['d', 0, 0xFFFFFFFF],
                ["timeoutMRP"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    if (uc($type) eq "MRP")
    {
        $type = PI_GENERIC_GLOBAL_MRP_TIMEOUT;
    }
    elsif (uc($type) eq "CCB")
    {
        $type = PI_GENERIC_GLOBAL_PI_SELECT_TIMEOUT;
    }
    elsif (uc($type) eq "IPC")
    {
        $type = PI_GENERIC_GLOBAL_IPC_TIMEOUT;
    }
    else
    {
        return undef;
    }

    my $cmd = PI_GENERIC_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LLLL63L",
                    $type,
                    0,
                    0,
                    $duration);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                      $packet,
                                      \&_genericResponsePacket);
}
#ifdef ENGINEERING
##############################################################################
# Name:     targetTest
#
# Desc:     Run the Target Test.
#
# Input:    TARGET -        target
#           OLDIF -         old interface slot
#           NEWIF -         new interface slot
#           TESTPASSES -    number of times to test
#           TIMEINTERVAL -  time interval between passes
#           TIMEOLDNEW -    time interval between old and new interface change
#           VERBOSE -       verbose (serial console output)  0 = off,  1 = on
#
# Return:   Generic Response Packet Hash
##############################################################################
sub targetTest
{
    my ($self,
        $target,
        $oldIF,
        $newIF,
        $testPasses,
        $timeInterval,
        $timeOldNew,
        $verbose) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF],
                ['d', 0, 0xFF],
                ['d', 0, 0xFF],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFFFFFFFF],
                ["targetTest"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_GENERIC_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LLLLLLLLLL57L",
                    PI_GENERIC_TEST_TARGET,
                    0,
                    0,
                    $target,
                    $oldIF,
                    $newIF,
                    $testPasses,
                    $timeInterval,
                    $timeOldNew,
                    $verbose);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}
#endif
##############################################################################
# Name:     globalCacheInfo
#
# Desc:     Get global caching information.
#
# INPUT:    NONE
#
# OUTPUT:   Global Caching Information Packet Hash
##############################################################################
sub globalCacheInfo
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["globalCacheInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_STATS_GLOBAL_CACHE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_globalCachePacket);
}

##############################################################################
# Name:     globalCacheSet
#
# Desc:     Set global caching mode.
#
# INPUT:    MODE    = Caching Mode
#                       0x80 = Caching disabled
#                       0x81 = Caching enabled
#
# OUTPUT:   Status Response Packet Hash
##############################################################################
sub globalCacheSet
{
    my ($self, $mode) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0x80, 0x81],
                ["globalCacheSet"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VCG_SET_CACHE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    0,0,0,
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
# Name:     modeDataSet
#
# Desc:     sets new modes
#
# Input:    hash of bits including values and masks
#
# Return:   _statusResponsePacket hash
##############################################################################
use constant MODEDATA_BITS_T => "
        L   # ccb bits
        L   # ccb bits DPrintf
        L   # ccb bits rsvd1
        L   # ccb bits rsvd2

        L   # proc bits[0]
        L   # proc bits[1]
        L   # proc bits[2]
        L   # proc bits[3]
        ";

# MODEDATA_T is the bits + mask which is simply two MODEDATA_BITS_T
# concatenated together.
use constant MODEDATA_T => MODEDATA_BITS_T.MODEDATA_BITS_T;

sub modeDataSet
{
    my ($self, %type) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['i'],
                ["structureGet"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_MISC_SET_MODE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack(MODEDATA_T,

        defined($type{CCB_BITS1}) ? $type{CCB_BITS1} : 0,               # ccb bits
        defined($type{CCB_BITS_DPRINTF}) ? $type{CCB_BITS_DPRINTF} : 0, # ccb bits DPrintf
        defined($type{CCB_BITS_RSVD1}) ? $type{CCB_BITS_RSVD1} : 0,     # ccb bits rsvd1
        defined($type{CCB_BITS_RSVD2}) ? $type{CCB_BITS_RSVD2} : 0,     # ccb bits rsvd2

        defined($type{PROC_BITS1}) ? $type{PROC_BITS1} : 0,             # proc bits[0]
        defined($type{PROC_BITS2}) ? $type{PROC_BITS2} : 0,             # proc bits[1]
        defined($type{PROC_BITS3}) ? $type{PROC_BITS3} : 0,             # proc bits[2]
        defined($type{PROC_BITS4}) ? $type{PROC_BITS4} : 0,             # proc bits[3]

        defined($type{CCB_BITS_MASK1}) ? $type{CCB_BITS_MASK1} : 0,     # ccb mask
        defined($type{CCB_BITS_DPRINTF_MASK}) ?
                                    $type{CCB_BITS_DPRINTF_MASK} : 0,   # ccb mask DPrintf
        defined($type{CCB_BITS_RSVD1_MASK}) ?
                                    $type{CCB_BITS_RSVD1_MASK} : 0,     # ccb bits rsvd1
        defined($type{CCB_BITS_RSVD2_MASK}) ?
                                    $type{CCB_BITS_RSVD2_MASK} : 0,     # ccb bits rsvd2

        defined($type{PROC_BITS_MASK1}) ? $type{PROC_BITS_MASK1} : 0,   # proc mask[0]
        defined($type{PROC_BITS_MASK2}) ? $type{PROC_BITS_MASK2} : 0,   # proc mask[1]
        defined($type{PROC_BITS_MASK3}) ? $type{PROC_BITS_MASK3} : 0,   # proc mask[2]
        defined($type{PROC_BITS_MASK4}) ? $type{PROC_BITS_MASK4} : 0,   # proc mask[3]
        );

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
# Name:     modeDataGet
#
# Desc:     retrieves mode bit data from CCB
#
# Input:    none
#
# Return:   _modeDataResponsePacket hash
##############################################################################
sub modeDataGet
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["modeDataGet"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_MISC_GET_MODE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_modeDataResponsePacket);
}

##############################################################################
# Name:     cfgoption
#
# Desc:     Configure controller options
#
# Input:    bits and mask
#
# Return:   hash
##############################################################################
sub cfgoption
{
    my ($self, $bits, $mask) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFFFFFFFF],
                ["cfgoption"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_MISC_CFGOPTION_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LL", $bits, $mask);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_cfgoptionResponsePacket);
}

##############################################################################
# Name:     structureInfo
#
# Desc:     retrieves a strucures info
#
# Input:    type of structure to retreive
#
# Return:   _readMemResponsePacket hash
##############################################################################
sub structureInfo
{
    my ($self, $type) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ["structureGet"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_DEBUG_STRUCT_DISPLAY_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("L", $type);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_structureResponsePacket);
}

##############################################################################
# Name:     writeBuffer
#
# Desc:     writes count images to the list of wwn's with luns in arr
#
# In:       count   -   number of images to be burned
#           arr     -   list of wwn and lun
#
# Returns:  _statusResponsePacket
##############################################################################
sub writeBuffer
{
    my ($self, $count, $filename, @arr) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['s'],
                ['i'],
                ["writeBuffer"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    # read up the file in binary mode
    my $buffer;
    my $rc = open FW, "$filename";
    binmode FW;
    read FW, $buffer, -s $filename;
    close FW;

    my $cmd = PI_WRITE_BUFFER_MODE5_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("L", $count);
    my $i;

    for ($i = 0; $i < $count; ++$i)
    {
        $data .=  pack("NNLL",
                    $arr[$i]{WWN_LO},
                    $arr[$i]{WWN_HI},
                    $arr[$i]{PD_LUN},
                    0);
    }

    if($rc)
    {
        $data .= $buffer;
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
# Name:  _powerUpStateResponsePacket
#
# Desc: Handles a power up state response packet
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
#       STATUS                  status of the command
##############################################################################

sub _regClientTypeResponse
{
    my ($self,
        $seq,
        $recvPacket) = @_;
    
    logMsg("begin\n");

    my %info;
    my $i;
    my $rsvd;
    
    my %parts = disassembleXiotechPacket($recvPacket);
    
    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};
    
    (
     $info{TYPE},
     $info{NCONN},
     $rsvd
     ) = unpack("CCS", $parts{DATA});
    
    return %info;
}

##############################################################################
# Name:     RegisterClientType
#
# Desc:     Register the Client type of the connection.
#
# In:       type    -   type of the client to be registered
#
# Returns:  
##############################################################################

sub RegisterClientType
{

    my ($self, $type) = @_;
    
    logMsg ("begin\n");
    
    my $cmd = PI_REGISTER_CLIENT_TYPE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $reqData = pack("C", $type);
    
    my $packet = assembleXiotechPacket($cmd,
                                       $seq,
                                       $ts,
                                       $reqData,
                                       $self->{PORT});
                
    return $self->_handleSyncResponse($seq,
                                      $packet,
                                      \&_regClientTypeResponse);
}

##############################################################################
# Name:     scsiCmd
#
# Desc:     issues count scsi commands to the list of wwn's with luns in arr
#
# In:       count   -   number of images to be burned
#           arr     -   list of wwn and lun
#           cbd     -   the SCSI cmd cdb
#           inpData -   the input data (if any)
#
# Returns:  _statusResponsePacket
##############################################################################
sub scsiCmd
{
    my ($self, $cdb, $inpData, @deviceID) = @_;

    logMsg("begin scsiCmd\n");

    # verify parameters
    my $args = [['i'],
                ["scsiCmd"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_DEBUG_SCSI_COMMAND_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data =  pack("NNLL",
            $deviceID[0]{WWN_LO},
            $deviceID[0]{WWN_HI},
            $deviceID[0]{PD_LUN},
            0);

    $data .= pack("L", length($cdb));

    my $pad = pack "a16", "";
    $cdb .= $pad;
    $cdb = substr $cdb, 0, 16;

    $data .= $cdb;

    if(defined($inpData)) {
        $data .= pack("L", length($inpData));
        $data .= $inpData;
    }
    else {
        $data .= pack("L", 0);
    }

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_scsiioResponsePacket);
}

##############################################################################
# Name:     diskReadWrite
#
# Desc:     issues count disk read/write commands
#
# In:       count   -   number of images to be burned
#           arr     -   list of wwn and lun
#           cbd     -   the SCSI cmd cdb
#           inpData -   the input data (if any)
#
# Returns:  _statusResponsePacket
##############################################################################
sub diskReadWrite
{
    my ($self, $type, $rw, $handle, $id, $block, $numberblocks) = @_;

    logMsg("begin diskReadWrite\n");

    my %rsp;
    my $cmd = PI_DEBUG_READWRITE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $buf = pack "a512", "";
    my $lth;
    if ($rw eq 'w')
    {
        $lth = 512;
    } else {
        $lth = 0;
    }

    for (my $i = 0; $i < $numberblocks; $i++)
    {
        my $block_lo = $block & 0xffffffff;
        my $block_hi = ($block / 4294967296) & 0xffffffff;
        my $Reqdata = pack("CCSVVV",
                            ord($type),
                            ord($rw),
                            $id,
                            $block_lo,
                            $block_hi,
                            $lth);
        if ($rw eq 'w')
        {
            if (!eof($handle))
            {
                read $handle, $buf, 512;
            }
            my $svl = length($Reqdata);
            $Reqdata .= $buf;
#-- my($t,$r,$i,$bh,$bl,$l,$st);
#-- ($t,$r,$i,$bl,$bh,$l,$st) = unpack("CCSVVVa512", $Reqdata);
#-- my $bk = $bl + ($bh << 32);
#-- printf STDERR "orig: $type $rw $id 0x%8.8x%8.8x ($block) $lth  next: $t $r $i 0x%8.8x%8.8x ($bk) $l $st\n", $block_lo, $block_hi, $bl, $bh;
#--             printf STDERR "lth(buf)=%d  svl=%d  reqdata=%d\n", length($buf), $svl, length($Reqdata);
        }
        my $packet = assembleXiotechPacket($cmd, $seq, $ts, $Reqdata, $self->{PORT});
        %rsp = $self->_handleSyncResponse($seq, $packet, \&_diskREADWRITEResponsePacket);
        if (%rsp)
        {
            # If successful, format the output data
            if ($rsp{STATUS} == PI_GOOD)
            {
                if ($rw eq 'r')
                {
                    print $handle $rsp{DATA};
                }
#-- printf "$rw block=$block (%u of $numberblocks) -- length=$rsp{DATALENGTH}\n", $i + 1;
                $block++;
            }
            else                # If failure, display sense data
            {
                my $msg = "cmd failed, block=$block";
                $rsp{BLOCK} = $block;
                return(%rsp);
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet, block=$block\n";
            return(%rsp);
        }
    }
    return(%rsp);
}   # diskReadWrite

##############################################################################
# Name: displaycfgoption
#
# Desc: prints configuration option information
#
# In:   hash
##############################################################################
sub displaycfgoption
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf "Configuration Option: 0x%8.8x", $info{OPTION};

    return $msg;
}

##############################################################################
# Name: displayStructureInfo
#
# Desc: prints structure information
#
# In:   structure hash
##############################################################################
sub displayStrucureInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    if (defined($info{RD_DATA}))
    {
        print $info{RD_DATA};
    }
    else
    {
        print "No data returned.";
    }
}

##############################################################################
# Name: displayTargetResList
#
# Desc: Print the appropriate object list depending on the listType
#
# In:   List Hash
##############################################################################
sub displayTargetResList
{
    my ($self, $listType, %info) = @_;
    logMsg("begin\n");

    my $msg = "";

    if (($listType < 12) || ($listType == 16))
    {
        $msg = displayObjectList(0, %info);
    }
    elsif ($listType < 32)
    {
        $msg = displayLunMapList(%info);
    }
    else
    {
        $msg = displaySidWwnList(%info);
    }

    return $msg;
}


##############################################################################
# Name: displayNClients
#
# Desc: Display the number of clients of client type.
#
# In:   List Hash
##############################################################################

sub displayNClients
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf "The Client type %d number of clients exist %d\n", $info{TYPE}, $info{NCONN};
    return $msg;
}

##############################################################################
# Name: displayObjectList
#
# Desc: Print the object list
#
# In:   List Hash
##############################################################################
sub displayObjectList
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $i;
    my $msg = "";

    for $i (0..$#{$info{LIST}})
    {
        $msg .= sprintf "  " . $info{LIST}[$i] . "\n";
    }

    return $msg;
}

##############################################################################
# Name: displayLunMapList
#
# Desc: Print the LUN Map list from Get Target Resource List
#
# In:   Scrubbing Information Hash
##############################################################################
sub displayLunMapList
{
    my (%info) = @_;

    logMsg("begin\n");

    my $i;
    my $j;
    my $msg = "";

    $msg .= sprintf "  Target    Server      LUN     VDisk \n";

    for $i (0..$#{$info{LIST}} / 4)
    {
        for ($j = 0; $j < 4; $j++)
        {
            $msg .= sprintf "   %3d    ", $info{LIST}[($i * 4) + $j];
        }
        $msg .= "\n";
    }

    return $msg;
}

##############################################################################
# Name: displaySidWwnList
#
# Desc: Print the Sid to WWN list from Get Target Resource List
#
# In:   Scrubbing Information Hash
##############################################################################
sub displaySidWwnList
{
    my (%info) = @_;

    logMsg("begin\n");

    my $i;
    my $j;
    my $msg = "";

    $msg .= sprintf "  SID    TID          WWN\n";
    $msg .= sprintf "  ---    ---    ----------------\n";

    for $i (0..$#{$info{LIST}})
    {
        $msg .= sprintf "  %3d    %3d    %8.8X%8.8X\n", $info{LIST}[$i]{SID},
                        $info{LIST}[$i]{TID}, $info{LIST}[$i]{WWN_LO},
                        $info{LIST}[$i]{WWN_HI};
    }

    return $msg;
}

##############################################################################
# Name: displayDeviceCount
#
# Desc: Display the device count response information.
#
# In:   Device Count Information Hash
##############################################################################
sub displayDeviceCount
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    print "Device Count:\n";
    printf "  STATUS:                %hu\n", $info{STATUS_MRP};
    printf "  LEN:                   %lu\n", $info{LEN};
    printf "  COUNT:                 %lu\n", $info{COUNT};
    print "\n";
}

##############################################################################
# Name: displayDeviceName
#
# Desc: Display the device name response information.
#
# In:   Device Name Information Hash
##############################################################################
sub displayDeviceName
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    print "Device Name:\n";
    printf "  STATUS:                %hu\n", $info{STATUS_MRP};
    printf "  LEN:                   %lu\n", $info{LEN};
    print  "  NAME:                  $info{NAME}\n";
    print "\n";
}

##############################################################################
# Name: displayDeviceConfigGet
#
# Desc: Display the device config get response information.
#
# In:   Device Config Get Information Hash
##############################################################################
sub displayDeviceConfigGet
{
    my ($self, %info) = @_;
    my $i;
    my $msg;

    logMsg("begin\n");

    $msg .= sprintf "Device Configuration Information (count: %d):\n", $info{COUNT};
    $msg .= sprintf  "\n";
    $msg .= sprintf  "  Vendor    Product ID        Device Flags                           \n";
    $msg .= sprintf  "  --------  ----------------  ---------------------------------------\n";

    for ($i = 0; $i < $info{COUNT}; $i++)
    {
        $msg .= sprintf "\"%8s\", \"%16s\", 0x%2.2X, 0x%2.2X, 0x%2.2X, 0x%2.2X, 0x%2.2X, 0x%2.2X, 0x%2.2X, 0x%2.2X\n",
                        $info{DEVICES}[$i]{DEVVENDOR},
                        $info{DEVICES}[$i]{DEVPRODID},
                        $info{DEVICES}[$i]{DEVFLAGS}[0],
                        $info{DEVICES}[$i]{DEVFLAGS}[1],
                        $info{DEVICES}[$i]{DEVFLAGS}[2],
                        $info{DEVICES}[$i]{DEVFLAGS}[3],
                        $info{DEVICES}[$i]{DEVFLAGS}[4],
                        $info{DEVICES}[$i]{DEVFLAGS}[5],
                        $info{DEVICES}[$i]{DEVFLAGS}[6],
                        $info{DEVICES}[$i]{DEVFLAGS}[7];
    }

    $msg .= sprintf "\n";

    return $msg;
}

##############################################################################
# Name: displayDeviceStatus
#
# Desc: Print the device status information
#
# In:   Firmware Version Information Hash
##############################################################################
sub displayDeviceStatus
{
    my ($self, $dev, $swtch, %info) = @_;

    my $controller_type = CTRL_TYPE_BIGFOOT;
    if (defined($self) && $self != 0)
    {
      $controller_type = $self->{CONTROLLER_TYPE};
    }

    logMsg("begin\n");

    my $i;

    my $msg = "";

    if(!defined($swtch))
    {
         $swtch = "N";
    }

    #added to allow for more than one print method, this print is the default print
    # S switch for results per second
    if(uc($swtch) eq "S")
    {
        if (uc($dev) eq "PD")
        {
            $msg .= sprintf "            PHYSICAL DEVICES PER SECOND STATISTICS          \n";
            $msg .= sprintf "       Queue                  Read     Write   Trnsfr       \n";
            $msg .= sprintf "  PID  Depth  MB  Commands  Commands  Commands  Size  Errors  Class\n";
            $msg .= sprintf "  ---  -----  --  --------  --------  --------  ----  ------  ------------\n";

            for $i (0..$#{$info{LIST}})
            {
                $msg .= sprintf "  %3hu  %5hu  %2hu  %8hu  %8hu  %8hu  %4hu  %6hu  %s\n",
                    $info{LIST}[$i]{PD_PID},
                    $info{LIST}[$i]{PD_QD},
                    ($info{LIST}[$i]{PD_RPS} * $info{LIST}[$i]{PD_AVGSC} * 512 / 1048576),
                    $info{LIST}[$i]{PD_RPS},
                    $info{LIST}[$i]{RREQ},
                    $info{LIST}[$i]{WREQ},
                    $info{LIST}[$i]{PD_AVGSC},
                    $info{LIST}[$i]{PD_ERR},
                    _getString_CLASS($info{LIST}[$i]{PD_CLASS});
            }
        }
        elsif (uc($dev) eq "VD")
        {
            my $ri;

            $msg .= sprintf "             LOGICAL DEVICES PER SECOND STATISTICS           \n";
            $msg .= sprintf "        Queue                  Read     Write   Trnsfr       \n";
            $msg .= sprintf "  VID   Depth  MB  Commands  Commands  Commands  Size  Errors   RID\n";
            $msg .= sprintf "  ----  -----  --  --------  --------  --------  ----  ------  ----\n";

            for $i (0..$#{$info{LIST}})
            {
                $msg .= sprintf "  %4hu  %5hu  %2hu  %8hu  %8hu  %8hu  %4hu  %6hu  ",
                    $info{LIST}[$i]{VID},
                    $info{LIST}[$i]{QD},
                    ($info{LIST}[$i]{RPS} * $info{LIST}[$i]{AVGSC} * 512 / 1048576),
                    $info{LIST}[$i]{RPS},
                    $info{LIST}[$i]{RREQ},
                    $info{LIST}[$i]{WREQ},
                    $info{LIST}[$i]{AVGSC},
                    $info{LIST}[$i]{ERROR};

                for ($ri = 0; $ri < $info{LIST}[$i]{RAIDCNT}; $ri++)
                {
                    if ($ri > 0)
                    {
                        $msg .= sprintf ",";
                    }

                    $msg .= sprintf "%4hu", $info{LIST}[$i]{RIDS}[$ri];
                }

                $msg .= sprintf "\n";
            }
        }
        elsif (uc($dev) eq "RD")
        {
            my $pi;

            $msg .= sprintf "              RAID DEVICES PER SECOND STATISTICS           \n";
            $msg .= sprintf "        Queue                  Read     Write   Trnsfr       \n";
            $msg .= sprintf "  RID   Depth  MB  Commands  Commands  Commands  Size  Errors   VID    PID\n";
            $msg .= sprintf "  ----  -----  --  --------  --------  --------  ----  ------  -----  -----\n";

            for $i (0..$#{$info{LIST}})
            {
                $msg .= sprintf "  %4hu  %5hu  %2hu  %8hu  %8hu  %8hu  %4hu  %6hu  %5hu  ",
                    $info{LIST}[$i]{RID},
                    $info{LIST}[$i]{QD},
                    ($info{LIST}[$i]{RPS} * $info{LIST}[$i]{AVGSC} * 512 / 1048576),
                    $info{LIST}[$i]{RPS},
                    $info{LIST}[$i]{RREQ},
                    $info{LIST}[$i]{WREQ},
                    $info{LIST}[$i]{AVGSC},
                    $info{LIST}[$i]{ERROR},
                    $info{LIST}[$i]{VID};

                for ($pi = 0; $pi < $info{LIST}[$i]{PSDCNT}; $pi++)
                {
                    if ($pi > 0)
                    {
                        $msg .= sprintf "\n";
                        $msg .= sprintf "                                                                     ";
                    }

                    $msg .= sprintf "%5hu",
                            $info{LIST}[$i]{PIDS}[$pi]{PID};
                }

                $msg .= sprintf "\n";
            }
        }
    }

    #add other swtch prints here
    #elsif(uc($swtch) eq "F")
    #{
    #    #new $msg .= sprintf functions for fd pd and rd
    #}

    else
    {
        if (uc($dev) eq "PD")
        {
            $msg .= sprintf "  PID   Dev  Misc  Post   LID    ALPA    TYPE          WWN          Serial#       Name          Class     \n";
            $msg .= sprintf "  ---  ----  ----  ----  ------  ----  --------  ----------------  ----------  ----------  ---------------\n";

            for $i (0..$#{$info{LIST}})
            {
                my $bad;

                if ($info{LIST}[$i]{PD_DEVSTAT} != 0x10 ||
                    $info{LIST}[$i]{PD_MISCSTAT} != 0x00 ||
                    $info{LIST}[$i]{PD_POSTSTAT} != 0x10)
                {
                    $bad = "*";
                }
                else
                {
                    $bad = " ";
                }
                if (uc($swtch) ne "B" || $bad eq '*')
                {
                    $msg .= sprintf "%s %3hu  0x%02x  0x%02x  0x%02x  0x%04x  0x%02x  %-8s  %8.8x%8.8x  0x%8.8x  %s  %s\n",
                        $bad,
                        $info{LIST}[$i]{PD_PID},
                        $info{LIST}[$i]{PD_DEVSTAT},
                        $info{LIST}[$i]{PD_MISCSTAT},
                        $info{LIST}[$i]{PD_POSTSTAT},
                        $info{LIST}[$i]{PD_ID},
                        _getALPA($info{LIST}[$i]{PD_ID}),
                        _getString_PDDT($info{LIST}[$i]{PD_DEVTYPE}),
                        $info{LIST}[$i]{WWN_LO},
                        $info{LIST}[$i]{WWN_HI},
                        $info{LIST}[$i]{PD_SSERIAL},
                        _getString_DNAME($controller_type, $info{LIST}[$i]{PD_DNAME}),
                        _getString_CLASS($info{LIST}[$i]{PD_CLASS});
                }
            }
        }
        elsif (uc($dev) eq "VD")
        {
            my $ri;

            $msg .= sprintf "  VID   Status        NAME         RID\n";
            $msg .= sprintf "  ----  ------  ----------------   ----\n";

            for $i (0..$#{$info{LIST}})
            {
                if (uc($swtch) ne "B" || $info{LIST}[$i]{DEVSTAT} != 0x10)
                {
                    $msg .= sprintf "  %4hu   0x%02x   %16s   ",
                            $info{LIST}[$i]{VID},
                            $info{LIST}[$i]{DEVSTAT},
                            $info{LIST}[$i]{NAME};
                    for ($ri = 0; $ri < $info{LIST}[$i]{RAIDCNT}; $ri++)
                    {
                        if ($ri > 0)
                        {
                            $msg .= sprintf ",";
                        }
                        $msg .= sprintf "%hu", $info{LIST}[$i]{RIDS}[$ri];
                    }
                    $msg .= sprintf "\n";
                }
            }
        }
        elsif (uc($dev) eq "RD")
        {
            my $pi;

            $msg .= sprintf "  RID   TYPE  STATUS  ASTATUS  VID   VID_STATUS  PSD  PID  PSD_STATUS  PSD_ASTATUS\n";
            $msg .= sprintf "  ----  ----  ------  -------  ----  ----------  ---  ---  ----------  -----------\n";

            for $i (0..$#{$info{LIST}})
            {
                if (uc($swtch) ne "B" || $info{LIST}[$i]{DEVSTAT} != 0x10 || $info{LIST}[$i]{ASTATUS} != 0x00)
                {
                    $msg .= sprintf "  %4hu    %2hu   0x%02x     0x%02x   %4hu     0x%02x     ",
                            $info{LIST}[$i]{RID},
                            $info{LIST}[$i]{TYPE},
                            $info{LIST}[$i]{DEVSTAT},
                            $info{LIST}[$i]{ASTATUS},
                            $info{LIST}[$i]{VID},
                            $info{LIST}[$i]{VID_STATUS};
                    for ($pi = 0; $pi < $info{LIST}[$i]{PSDCNT}; $pi++)
                    {
                        if ($pi > 0)
                        {
                            $msg .= sprintf "\n";
                            $msg .= sprintf "                                               ";
                        }
                        $msg .= sprintf "%3hu  %3hu     0x%02x        0x%02x",
                                $pi,
                                $info{LIST}[$i]{PIDS}[$pi]{PID},
                                $info{LIST}[$i]{PIDS}[$pi]{PSD_STATUS},
                                $info{LIST}[$i]{PIDS}[$pi]{PSD_ASTATUS};
                    }
                    $msg .= sprintf "\n";
                }
            }
        }
    }

    $msg .= sprintf "\n";

    return $msg;
}

##############################################################################
# Name: displayFirmwareDiskInfo
#
# Desc: Print the firmware version information
#
# In:   Firmware Version Information Hash
##############################################################################
sub displayFirmwareVersion
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    my @fwtypes = ( FW_VER_CCB_RUNTIME,
                    FW_VER_BE_RUNTIME,
                    FW_VER_FE_RUNTIME,
                    FW_VER_CCB_BOOT,
                    FW_VER_BE_BOOT,
                    FW_VER_FE_BOOT,
                    FW_VER_BE_DIAG,
                    FW_VER_FE_DIAG);

    my @fwnames = ( "CCB_RUN",
                    "BE_RUN",
                    "FE_RUN",
                    "CCB_BOOT",
                    "BE_BOOT",
                    "FE_BOOT",
                    "BE_DIAG",
                    "FE_DIAG");

    my $numDashes = 63;
    $msg .= sprintf "Firmware    Vers    Count   BldID  SysRls   Timestamp (GMT)\n";
    $msg .= sprintf "-" x $numDashes . "\n";

    for (my $i = 0; $i < scalar(@fwtypes); ++$i)
    {
        my $type = $fwtypes[$i];
        my $name = $fwnames[$i];

        if ($info{$type})
        {
            $msg .= sprintf "%-8s    %-4s    %-4s    %-4s    %-4s    ".
                                                "%02x/%02x/%04x %02x:%02x:%02x\n",
            $name,
            $info{$type}{REVISION},
            $info{$type}{REV_COUNT},
            $info{$type}{BUILD_ID},
            $info{$type}{SYSTEM_RLS},
            $info{$type}{TS_MONTH},
            $info{$type}{TS_DATE},
            $info{$type}{TS_YEAR},
            $info{$type}{TS_HOURS},
            $info{$type}{TS_MINUTES},
            $info{$type}{TS_SECONDS};

            # Add a little hack for Wookiee (look forward to see if the next
            # fw header is defined. If not, skip the extra seperator).
            $msg .= sprintf "-" x $numDashes . "\n"
                            if (($type == FW_VER_FE_RUNTIME) && $info{$fwtypes[$i+1]} );
            $msg .= sprintf "-" x $numDashes . "\n" if $type == FW_VER_CCB_BOOT;
            $msg .= sprintf "-" x $numDashes . "\n" if $type == FW_VER_FE_BOOT;
        }
    }

    $msg .= sprintf "-" x $numDashes . "\n";

    return $msg;
}

##############################################################################
# Name: displayFWSysRel
#
# Desc: Display the Firmware System Release information
#
# In:   sos Information Hash
##############################################################################
sub displayFWSysRel
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf("System Release: 0x%08X  ", $info{SYSREL});
    $msg .= sprintf("Tag: %s\n\n", $info{TAG});

    return $msg;
}

##############################################################################
# Name: displayGlobalCacheInfo
#
# Desc: Print the global cache information
#
# In:   Global Cache Information Hash
##############################################################################
sub displayGlobalCacheInfo
{
    my ($self, %info) = @_;
    my $msg = "";

    logMsg("begin\n");

    $msg .= sprintf "Global Information:\n";
    $msg .= sprintf "  STATUS:                %hu\n", $info{STATUS_MRP};
    $msg .= sprintf "  LEN:                   %lu\n", $info{LEN};
    $msg .= sprintf "  CA_STATUS:             0x%x", $info{CA_STATUS};
    if ($info{CA_STATUS} & 1) {
      $msg .= sprintf " GlobalCacheOn";
    } else {
      $msg .= sprintf " GlobalCacheOff";
    }
    if ($info{CA_STATUS} & 2) {
      $msg .= sprintf " DisableInProg";
    }
    if ($info{CA_STATUS} & 4) {
      $msg .= sprintf " EnablePending";
    }
    if ($info{CA_STATUS} & 8) {
      $msg .= sprintf " MirrorPartner";
    }
    if ($info{CA_STATUS} & 16) {
      $msg .= sprintf " MirrorPartnerBroken";
    }
    if ($info{CA_STATUS} & 32) {
      $msg .= sprintf " CacheErrors";
    }
    if ($info{CA_STATUS} & 64) {
      $msg .= sprintf " CacheShutdown";
    }
    if ($info{CA_STATUS} & 128) {
      $msg .= sprintf " NoBackgroundFlushAllowed";
    }
    $msg .= sprintf "\n";
    $msg .= sprintf "  CA_BATTERY:            0x%x\n", $info{CA_BATTERY};
    $msg .= sprintf "  CA_STOPCNT:            %lu\n", $info{CA_STOPCNT};
    $msg .= sprintf "  CA_SIZE                %lu\n", $info{CA_SIZE};
    $msg .= sprintf "  CA_MAXCWR              %lu\n", $info{CA_MAXCWR};
    $msg .= sprintf "  CA_MAXSGL              %lu\n", $info{CA_MAXSGL};
    $msg .= sprintf "  CA_NUMTAGS             %lu\n", $info{CA_NUMTAGS};
    $msg .= sprintf "  CA_TAGSDIRTY           %lu\n", $info{CA_TAGSDIRTY};
    $msg .= sprintf "  CA_TAGSRESIDENT        %lu\n", $info{CA_TAGSRESIDENT};
    $msg .= sprintf "  CA_TAGSFREE            %lu\n", $info{CA_TAGSFREE};
    $msg .= sprintf "  CA_TAGSFLUSHIP         %lu\n", $info{CA_TAGSFLUSHIP};
    $msg .= sprintf "  CA_NUMBLKS             %lu\n", $info{CA_NUMBLKS};
    $msg .= sprintf "  CA_BLOCKSDIRTY         %lu\n", $info{CA_BLOCKSDIRTY};
    $msg .= sprintf "  CA_BLOCKRESIDENT       %lu\n", $info{CA_BLOCKRESIDENT};
    $msg .= sprintf "  CA_BLOCKFREE           %lu\n", $info{CA_BLOCKFREE};
    $msg .= sprintf "  CA_BLKSFLUSHIP         %lu\n", $info{CA_BLKSFLUSHIP};

    $msg .= sprintf "\n";

    return $msg;

}

##############################################################################
# Name: displayResyncData
#
# Desc: Print the resync data.
#
# In:   Resync Data Hash
##############################################################################
sub displayResyncData
{
    my ($self, $fmt, $cnt, %info) = @_;

    my $i;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf "Resync Data:\n";
    $msg .= sprintf "  STATUS:          %hu\n", $info{STATUS_MRP};
    $msg .= sprintf "  LEN:             %lu\n", $info{LEN};
    $msg .= sprintf "  COUNT:           %lu\n", $info{COUNT};
    $msg .= sprintf "  FORMAT:          %d\n", $info{FORMAT};
    $msg .= sprintf "  STRCTSIZE:       %d\n", $info{STRCTSIZE};
    $msg .= sprintf "\n";

    if ($info{FORMAT} == 0)
    {
        if (uc($fmt) eq "STD")
        {
            $msg .= sprintf "  RID   GID  CSTATE  FLAGS  CRSTATE     RCSN       POWNER      SOWNER          LABEL     \n";
            $msg .= sprintf "  ----  ---  ------  -----  -------  ----------  ----------  ----------  ----------------\n";

            for ($i = 0; $i < $info{COUNT}; $i++)
            {
                $msg .= sprintf "  %4lu  %3lu    0x%2.2x   0x%2.2x     0x%2.2x  0x%8.8x  0x%8.8x  0x%8.8x  %16s\n",
                                $info{CORS}[$i]{RID},
                                $info{CORS}[$i]{GID},
                                $info{CORS}[$i]{COPYSTATE},
                                $info{CORS}[$i]{FLAGS},
                                $info{CORS}[$i]{CRSTATE},
                                $info{CORS}[$i]{RCSN},
                                $info{CORS}[$i]{POWNER},
                                $info{CORS}[$i]{SOWNER},
                                $info{CORS}[$i]{LABEL};
            }
        }
        elsif (uc($fmt) eq "VIDS")
        {

            $msg .= sprintf "  RID      RCSN     RCS VID (CL-VD)  RCD VID (CL-VD)     RSSN     RS VID (CL-VD)     RDSN     RD VID (CL-VD)\n";
            $msg .= sprintf "  ----  ----------  ---------------  ---------------  ----------  --------------  ----------  --------------\n";

            for ($i = 0; $i < $info{COUNT}; $i++)
            {
                my $clvecal = "";
                my $rcd = "";
                my $rcs = "";
                my $rs = "";
                my $rd = "";

                if ($info{CORS}[$i]{RCSN} >= 10000)
                {
                    $clvecal = ($info{CORS}[$i]{RCSCL} * 256) + $info{CORS}[$i]{RCSVD};

                    $rcs  =  sprintf "%lu %9s", $clvecal,
                             sprintf "(%lu-%2.2lu)", $clvecal / 32, $clvecal % 32;


                    $clvecal = ($info{CORS}[$i]{RCDCL} * 256) + $info{CORS}[$i]{RCDVD};

                    $rcd  = sprintf "%lu %9s", $clvecal,
                            sprintf "(%lu-%2.2lu)", $clvecal / 32, $clvecal % 32;
                }
                else
                {
                    $clvecal = ($info{CORS}[$i]{RCSCL} * 256) + $info{CORS}[$i]{RCSVD};

                    $rcs  =  sprintf "  --- %9s",
                             sprintf "(%lu-%2.2lu)", $info{CORS}[$i]{RCSCL}, $info{CORS}[$i]{RCSVD};

                    $clvecal = ($info{CORS}[$i]{RCDCL} * 256) + $info{CORS}[$i]{RCDVD};

                    $rcd  = sprintf "  --- %9s",
                            sprintf "(%lu-%2.2lu)", $info{CORS}[$i]{RCDCL}, $info{CORS}[$i]{RCDVD};
                }

                if ($info{CORS}[$i]{RSSN} >= 10000)
                {
                    $clvecal = ($info{CORS}[$i]{RSCL} * 256) + $info{CORS}[$i]{RSVD};

                    $rs  =  sprintf "%lu %9s", $clvecal,
                            sprintf "(%lu-%2.2lu)", $clvecal / 32, $clvecal % 32;
                }
                else
                {
                    $rs  =  sprintf " --- %9s",
                            sprintf "(%lu-%2.2lu)", $info{CORS}[$i]{RSCL}, $info{CORS}[$i]{RSVD};
                }


                if ($info{CORS}[$i]{RDSN} >= 10000)
                {
                    $clvecal = ($info{CORS}[$i]{RDCL} * 256) + $info{CORS}[$i]{RDVD};

                    $rd  =  sprintf "%lu %9s", $clvecal,
                            sprintf "(%lu-%2.2lu)", $clvecal / 32, $clvecal % 32;
                }
                else
                {
                    $rd  =  sprintf "  --- %9s",
                            sprintf "(%lu-%lu)", $info{CORS}[$i]{RDCL}, $info{CORS}[$i]{RDVD};
                }

                $msg .= sprintf "  %4lu  0x%8.8x  %15s  %15s  0x%8.8x  %14s  0x%8.8x  %14s\n",
                                $info{CORS}[$i]{RID},
                                $info{CORS}[$i]{RCSN},
                                $rcs,
                                $rcd,
                                $info{CORS}[$i]{RSSN},
                                $rs,
                                $info{CORS}[$i]{RDSN},
                                $rd;

            }
        }
        elsif (uc($fmt) eq "OCSE")
        {
            $msg .= sprintf "  RID      RCSN      CC TBL    CC   CC   CC   CC    OC TBL    OC   OC   OC   OC\n";
            $msg .= sprintf "                      PTR      CST  CEV  LST  LEV    PTR      CST  CEV  LST  LEV\n";
            $msg .= sprintf "  ----  ---------  ---------   ---  ---  ---  ---  ---------  ---  ---  ---  ---\n";

            for ($i = 0; $i < $info{COUNT}; $i++)
            {
                $msg .= sprintf "  %4lu  %8.8xh  %8.8xh   %2.2xh  %2.2xh  %2.2xh  %2.2xh  %8.8xh  %2.2xh  %2.2xh  %2.2xh  %2.2xh",
                                $info{CORS}[$i]{RID},
                                $info{CORS}[$i]{RCSN},
                                $info{CORS}[$i]{CCSEPTR},
                                $info{CORS}[$i]{CCSECST},
                                $info{CORS}[$i]{CCSECEV},
                                $info{CORS}[$i]{CCSELST},
                                $info{CORS}[$i]{CCSELEV},
                                $info{CORS}[$i]{OCSEPTR},
                                $info{CORS}[$i]{OCSECST},
                                $info{CORS}[$i]{OCSECEV},
                                $info{CORS}[$i]{OCSELST},
                                $info{CORS}[$i]{OCSELEV};
                $msg .= sprintf "\n";

            }
        }
    }
    elsif ($info{FORMAT} == 1)
    {
        $msg .= sprintf "  RID      RCSN    Owner  SVID   DVID   COR C  COR R  COR M  CM C   S P2   STYPE  D P2   DTYPE  Mirror  VDD\n";
        $msg .= sprintf "                                        State  State  State  State  HDLR          HDLR          State   ATTR   TYPE\n";
        $msg .= sprintf "  ----  ---------  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  ------  -----  ----\n";

        for ($i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf "  %4lu  %8.8xh    %1lu    %2.2x%2.2xh  %2.2x%2.2xh   %2.2xh    %2.2xh    %2.2xh    %2.2xh    %2.2xh    %2.2xh    %2.2xh    %2.2xh    %2.2xh     %2.2xh   %u\n",
                            $info{DTLCPY}[$i]{RID},
                            $info{DTLCPY}[$i]{RCSN},
                            $info{DTLCPY}[$i]{OWNER},
                            $info{DTLCPY}[$i]{RCSCL},
                            $info{DTLCPY}[$i]{RCSDV},
                            $info{DTLCPY}[$i]{RCDCL},
                            $info{DTLCPY}[$i]{RCDDV},
                            $info{DTLCPY}[$i]{CORCSTATE},
                            $info{DTLCPY}[$i]{CORRSTATE},
                            $info{DTLCPY}[$i]{CORMSTATE},
                            $info{DTLCPY}[$i]{CMCSTATE},
                            $info{DTLCPY}[$i]{SP2HDLR},
                            $info{DTLCPY}[$i]{STYPE},
                            $info{DTLCPY}[$i]{DP2HDLR},
                            $info{DTLCPY}[$i]{DTYPE},
                            $info{DTLCPY}[$i]{VMIRROR},
                            $info{DTLCPY}[$i]{VATTR},
                            $info{DTLCPY}[$i]{CTYPE},
        }
    }
    elsif ($info{FORMAT} == 2)
    {
        if ($cnt > $info{COUNT})
        {
            $cnt = $info{COUNT};
        }

        $msg .= sprintf "  CCSM Traces\n\n";

        for ($i = $info{COUNT} - $cnt; $i < $info{COUNT}; $i++)
        {

            $msg .= FormatDataString($self, $info{RDATA}[$i]{TRECORD}, ($info{COUNT} - $i) - 1, "byte", undef, undef);
        }
    }
    elsif ($info{FORMAT} == 3)
    {
        if ($cnt > $info{COUNT})
        {
            $cnt = $info{COUNT};
        }

        $msg .= sprintf "  COPIES NOT PAUSED MAP\n\n";

        for ($i = 0; $i < $info{COUNT}; $i++)
        {

            $msg .= FormatDataString($self, $info{RDATA}[$i]{TRECORD},  $i, "byte", undef, undef);
        }

        my @vDisks = ParseBitmap($info{MIRROR_IO_STATUS_MAP});

        $msg .= sprintf "\n  Copies not paused (list format): @vDisks\n";

    }

    $msg .= sprintf "\n";

    return $msg;
}

##############################################################################
# Name: displayScrubInfo
#
# Desc: Print the scrubbing information
#
# In:   Scrubbing Information Hash
##############################################################################
sub displayScrubInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf "Scrubbing Information:\n";
    $msg .= sprintf "  STATUS:             %hu\n", $info{STATUS_MRP};
    $msg .= sprintf "  LEN:                %lu\n", $info{LEN};
    $msg .= sprintf "  SSTATE:             0x%x\n", $info{SSTATE};
    $msg .= sprintf "  PSTATE:             0x%x\n", $info{PSTATE};

    $msg .= sprintf "\n";

    $msg .= sprintf "  Classic Scrubbing:  %s\n", $info{SCRUBBING};
    $msg .= sprintf "\n";
    $msg .= sprintf "  Marked:             %s\n", $info{PC_MARKED};
    $msg .= sprintf "  Corruption:         %s\n", $info{PC_CORRUPT};
    $msg .= sprintf "  Raid Devices:       %s\n", $info{PC_SPECIFIC};
    $msg .= sprintf "  Clear Logs:         %s\n", $info{PC_CLEARLOGS};
    $msg .= sprintf "  Number of Passes:   %s\n", $info{PC_1PASS};
    $msg .= sprintf "  Correction:         %s\n", $info{PC_CORRECT};
    $msg .= sprintf "  Parity Scan:        %s\n", $info{PC_ENABLE};
    $msg .= sprintf "  Passes:             %s\n", $info{PASSES};

    $msg .= sprintf "\n";

    $msg .= sprintf "  PID Scrubbed:       0x%x\n", $info{SCRUBP};
    $msg .= sprintf "  Block Scrubbed:     0x%x\n", $info{SCRUBB};
    $msg .= sprintf "  RID Scanned:        0x%x\n", $info{SCANR};
    $msg .= sprintf "  Block Scanned:      0x%x\n", $info{SCANB};

    $msg .= sprintf "\n";

    return $msg;
}

##############################################################################
# Name: displaySerialNumbers
#
# Desc: Print the serial numbers for this controller.
#
# In:   Serial Number Hash
##############################################################################
sub displaySerialNumbers
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    for (my $i = 1; $i <= 2; ++$i)
    {
        if ($info{$i})
        {
            if ($i == 1)
            {
                $msg .= sprintf "Controller Serial Number: %d (0x%x)\n",
                        $info{$i}{SERIAL_NUM},
                        $info{$i}{SERIAL_NUM};
            }
            elsif ($i == 2)
            {
                $msg .= sprintf "Virtual Control Group ID: %d (0x%x)\n",
                        $info{$i}{SERIAL_NUM},
                        $info{$i}{SERIAL_NUM};
            }
        }
    }

    $msg .= sprintf "\n";

    return $msg;
}

##############################################################################
# Name: displayModeDataInfo
#
# Desc: Print the Mode bits information
#
##############################################################################
sub displayModeDataInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf "Mode bits:\n";
    $msg .= sprintf "  CCB bits:              0x%08x\n", $info{CCB_MODE_BITS1};
    $msg .= sprintf "  CCB DPRINTF bits:      0x%08x\n", $info{CCB_MODE_DPRINTF_BITS};
    $msg .= sprintf "  CCB rsvd1 bits:        0x%08x\n", $info{CCB_MODE_RSVD1_BITS};
    $msg .= sprintf "  CCB rsvd2 bits:        0x%08x\n", $info{CCB_MODE_RSVD2_BITS};
    $msg .= sprintf "  PROC bits 1:           0x%08x\n", $info{PROC_MODE_BITS1};
    $msg .= sprintf "  PROC bits 2:           0x%08x\n", $info{PROC_MODE_BITS2};
    $msg .= sprintf "  PROC bits 3:           0x%08x\n", $info{PROC_MODE_BITS3};
    $msg .= sprintf "  PROC bits 4:           0x%08x\n", $info{PROC_MODE_BITS4};
    $msg .= sprintf "\n";

    return $msg;

}

##############################################################################
# Name: displayDeviceList
#
# Desc: print the device list information
#
# In:   device list Information Hash
##############################################################################
sub displayDeviceList
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf "\n";
    $msg .= sprintf "Device List Count: $info{NDEVS}\n\n";

    my $i;
    $msg .= sprintf "   LID   MST  SST   PORT_ID      PORT_WWN          NODE_WWN    \n";
    $msg .= sprintf "  -----  ---  ---  --------  ----------------  ----------------\n";
    for ($i = 0; $i < $info{NDEVS}; ++$i)
    {
        $msg .= sprintf "  %5X  %3X  %3X  %8X  %8.8x%8.8x  %8.8x%8.8x\n",
            $info{LIST}[$i]{LID},
            $info{LIST}[$i]{MST},
            $info{LIST}[$i]{SST},
            $info{LIST}[$i]{PORT_ID},
            $info{LIST}[$i]{PORT_WWN_LO},
            $info{LIST}[$i]{PORT_WWN_HI},
            $info{LIST}[$i]{NODE_WWN_LO},
            $info{LIST}[$i]{NODE_WWN_HI};
    }

    return $msg;
}

##############################################################################
# Name: displayDevicePath
#
# Desc: print the device path information
#
# In:   device path Information Hash
##############################################################################
sub displayDevicePath
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    print "\n";

    my $i;
    my $msg = "";

    $msg .= sprintf "Number of devices: %d\n\n", $info{NDEVS};

    if ($info{SIZE} == 8 || $info{SIZE} == 1)
    {
        $msg .= sprintf "   PID   COUNT  PATH[0]  PATH[1]  PATH[2]  PATH[3]\n";
        $msg .= sprintf " ------  -----  -------  -------  -------  --------\n";
        for ($i = 0; $i < $info{NDEVS}; ++$i)
        {
            if ($info{SIZE} != 1)
            {
                if (($info{LIST}[$i]{PATH_COUNT} != 0))
                {
                    $msg .= sprintf " %3d %02X   %4d     %02X       %02X       %02X       %02X\n",
                                    $info{LIST}[$i]{PID}, $info{LIST}[$i]{PID},
                                    $info{LIST}[$i]{PATH_COUNT},
                                    $info{LIST}[$i]{PATH1},
                                    $info{LIST}[$i]{PATH2},
                                    $info{LIST}[$i]{PATH3},
                                    $info{LIST}[$i]{PATH4};
                }
            }
            else
            {
                if (($info{LIST}[$i]{PATH_COUNT} != 2))
                {
                    $msg .= sprintf " %3d %02X   %4d     %02X       %02X       %02X       %02X\n",
                                    $info{LIST}[$i]{PID}, $info{LIST}[$i]{PID},
                                    $info{LIST}[$i]{PATH_COUNT},
                                    $info{LIST}[$i]{PATH1},
                                    $info{LIST}[$i]{PATH2},
                                    $info{LIST}[$i]{PATH3},
                                    $info{LIST}[$i]{PATH4};
                }
            }
        }
    }
    else
    {
        $msg .= sprintf "  PID   BIT_PATH \n";
        $msg .= sprintf " -----  -------- \n";
        for ($i = 0; $i < $info{NDEVS}; ++$i)
        {
            $msg .= sprintf " %04X     %04X\n",
                            $info{LIST}[$i]{PID},
                            $info{LIST}[$i]{BIT_PATH};
        }
    }

    return $msg;
}

##############################################################################
# Name: displayPowerUpState
#
# Desc: print the device list information
#
# In:   device list Information Hash
##############################################################################
sub displayPowerUpState
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    print "\n";
    printf "Power-up State:   0x%04X - ", $info{STATE};
    print _getString_POWERUP($info{STATE});
    print "\n";

    printf "Additional State: 0x%04X - ", $info{ADDITIONALSTATE};
    print _getString_AdditionalPowerUp($info{ADDITIONALSTATE});
    print "\n";

}

##############################################################################
# Name: displayRMState
#
# Desc: print the device list information
#
# In:   device list Information Hash
##############################################################################
sub displayRMState
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    print "\n";
    printf "Resource Manager State: 0x%x - ", $info{STATE};

    if ($info{STATE} == RM_NONE)
    {
        print "RM_NONE";
    }
    elsif ($info{STATE} == RM_INIT)
    {
        print "RM_INIT";
    }
    elsif ($info{STATE} == RM_SHUTDOWN)
    {
        print "RM_SHUTDOWN";
    }
    elsif ($info{STATE} == RM_RUNNING)
    {
        print "RM_RUNNING";
    }
    elsif ($info{STATE} == RM_BUSY)
    {
        print "RM_BUSY";
    }
    elsif ($info{STATE} == RM_DOWN)
    {
        print "RM_DOWN";
    }
    else
    {
        print "UNKNOWN";
    }

    print "\n\n";
}

##############################################################################
# Name: displaySos
#
# Desc: print the sos information
#
# In:   sos Information Hash
##############################################################################
sub displaySos
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf "SOS TABLE:\n\n";
    $msg .= sprintf("NEXT:       0x%08X\n",      $info{NEXT});
    $msg .= sprintf("PID:        %d\n",          $info{PID});
    $msg .= sprintf("FLAGS:      0x%04X\n",      $info{FLAGS});
    $msg .= sprintf("REMAIN:     0x%08X\n",      $info{REMAIN});
    $msg .= sprintf("TOTAL:      0x%08X\n",      $info{TOTAL});
    $msg .= sprintf("PDD:        0x%08X\n",      $info{PDD});
    $msg .= sprintf("OWNER:      0x%08X\n",      $info{OWNER});
    $msg .= sprintf("COUNT:      %d\n",          $info{COUNT});
    $msg .= sprintf("CURRENT:    %d\n",          $info{CURRENT});
    $msg .= sprintf("PCB:        0x%08X\n",      $info{PCB});

    $msg .= sprintf "\n";

    my $i;
    $msg .= sprintf "  GAP_SIZE      SDA         LEN         RID\n";
    $msg .= sprintf " ----------  ----------  ----------  ----------\n";
    for ($i = 0; $i < $info{COUNT}; ++$i)
    {
        $msg .= sprintf " 0x%08X  0x%08X  0x%08X  0x%08X\n",
            $info{LIST}[$i]{GAP_SIZE},
            $info{LIST}[$i]{SDA},
            $info{LIST}[$i]{LEN},
            $info{LIST}[$i]{RID};
    }

    return $msg;
}

##############################################################################
# Name:     FormatData()
#
# Desc:     Format binary data in various formats to STDOUT or a file
#
# Input:    data
#           address (that is came from)
#           format (byte/short/word/binary)
#           filename (if output to go to a file; undef otherwise)
#           reqLength - requested data length (0 = all available data)
#
##############################################################################
sub FormatData
{
    my ($self, $buffer, $address, $format, $filen, $reqLength) = @_;

    my $useFile = 0;

    # open the output file if one was requested
    if (defined($filen))
    {
        if (!open(FH, ">$filen"))
        {
            print "\nCouldn't open $filen for output\n";
        }
        else
        {
            $useFile = 1;
        }
    }

    # see if alternate format chosen
    if ($format =~ /^binary$/i) {
        if(!$useFile) {
            # explicitly use STDOUT here in case it is redirected
            print STDOUT "\n'binary' format only supported when writing to a file.\n";
        }
        else {
            binmode FH;
            print FH $buffer;
            close FH;
        }
        return;
    }

    my $str =  FormatDataString($self, $buffer, $address, $format, $filen, $reqLength);

    if (!$useFile)
    {
        print $str;
    }
    else
    {
        print FH $str;
        close FH;
    }
}


##############################################################################
# Name:     FormatDataString()
#
# Desc:     Format binary data in various formats to a string
#
# Input:    data
#           address (that is came from)
#           format (byte/short/word/binary)
#           filename (if output to go to a file; undef otherwise)
#           reqLength - requested data length (0 = all available data)
#
##############################################################################
sub FormatDataString
{
    my ($self, $buffer, $address, $format, $filen, $reqLength) = @_;

    # String to store output
    my $str = "";

    # set up the byte count and templates based upon output format requested
    my $addrTpl = "%08X  ";
    my $asciiTpl = "  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n";
    my $byteTpl = "CCCC CCCC CCCC CCCC";
    my $unpackTpl = "L L L L";
    my $printfTpl = "%08X %08X  %08X %08X";

    if ($format =~ /^byte$/i) {
        $unpackTpl = $byteTpl;
         $printfTpl = "%02X %02X %02X %02X %02X %02X %02X %02X  " .
             "%02X %02X %02X %02X %02X %02X %02X %02X";
    }
    elsif ($format =~ /^short$/i) {
        $unpackTpl = "SS SS SS SS";
        $printfTpl = "%04X %04X %04X %04X  %04X %04X %04X %04X";
    }


    # get the overall length of the data buffer
    my $length = length $buffer;


    # if the requested length is defined return only the amount requested.
    if (defined $reqLength)
    {
        $length = $reqLength;
    }

    my $i;
    my @rowData;
    my @asciiData;
    my $padLen = 0;

    for ($i=0; $i<$length; $i+=16) {

        @rowData = ();
        @asciiData = ();

        # Getting the final line to display properly when not an even multiple
        # of 16 bytes is not easy given the way that this routine has been
        # structured. So as a kludge, we pad the data out to the proper length
        # with 0xAA's, and then warn the caller that we did this.
        if ($i+16 > $length) {
            $padLen = ($i+16) - $length;
            $buffer .= pack "C$padLen",
            0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
            0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA;
        }

        push @rowData, unpack $unpackTpl, $buffer;

        my $tmp;
        foreach $tmp (unpack $byteTpl, $buffer) {
            if ($tmp < 0x20 or $tmp >= 0x7f) {
                $tmp = 0x2e; # '.'
            }
            push @asciiData, $tmp;
        }

        $str .= sprintf  $addrTpl.$printfTpl.$asciiTpl, $address, @rowData, @asciiData;

        if ($padLen) {
            $str .= sprintf
            "\nWarning: The final $padLen bytes (AA's) ARE NOT YOUR DATA!\n";
        }

        if (length($buffer) > 16) {
            $buffer = substr $buffer, 16;
        }
        $address += 16;
    }

    return $str;

}


##############################################################################
# Name:     FormatHeapData
#
# Desc:     Parses the heap report data retrieved from the CCB
#
# Input:    Which command (for a report, one of: HEAP, TRACE, PCB, PROFILE)
##############################################################################
sub FormatHeapData()
{
    my ($self, $data, $filen) = @_;
    my $msg;

    # Call the new decoder, it will call the old decoder if necessary.
    FmtCCBHeapStatsFID2(\$msg, 0, \$data, 0, 0, 0, 0, 0);

    # open the output file if one was requested
    if (defined $filen)
    {
        my $rc = open FH, ">$filen";
        if ($rc == 0)
        {
            print "Couldn't open \"$filen\".\n";
        }
        else
        {
            print "Writing output to \"$filen\".\n";
            print FH $msg;
            close FH;
            undef $msg;  # don't return anything if writing to a file
        }
    }

    return $msg;
}

##############################################################################
# Name:     ip2long()
#
# Desc:     Convert an IP address of the form "X.X.X.X" to a long in
#           network byte order.  VERY similar to the BSD inet_addr()
#           call.
#
# Input:    IP address string
#
# Return:   Packed long in network (big-endian) byte order
##############################################################################
sub ip2long
{
    my ($self, $ip) = @_;
    my $long = 0;

    if ((defined $ip) and ($ip =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?$/))
    {
        my @parts = split /\./, $ip;

        $long = ($parts[0] << 24) +
                ($parts[1] << 16) +
                ($parts[2] << 8) +
                $parts[3];
    }

    return pack "N", $long;
}

##############################################################################
# Name:     long2ip
#
# Desc:     Convert a long in network byte order to an IP address of
#           the form "X.X.X.X"
#
# Input:    long to be converted
#
# Return:   String IP address
##############################################################################
sub long2ip
{
    my ($self, $long) = @_;
    my $ip = "";

    if ((defined $long))
    {
        $ip = "" . (($long & 0xFF000000) >> 24) . "\." .
                   (($long & 0x00FF0000) >> 16) . "\." .
                   (($long & 0x0000FF00) >> 8) . "\." .
                   ($long & 0x000000FF) . "";
    }

    return $ip;
}

##############################################################################
# Name:     net2ip
#
# Desc:     Convert a long in reverse network byte order to an IP address of
#           the form "X.X.X.X"
#
# Input:    long to be converted
#
# Return:   String IP address
##############################################################################
sub net2ip
{
    my ($self, $long) = @_;
    my $ip = "";

    if ((defined $long))
    {
        $ip = "" . ($long & 0x000000FF) . "\." .
                   (($long & 0x00FF0000) >> 16) . "\." .
                   (($long & 0x0000FF00) >> 8) . "\." .
                   (($long & 0xFF000000) >> 24) . "";
    }

    return $ip;
}

##############################################################################
# Name:     BuildBitmap
#
# Desc:     Build up a bitmap string from an array of ints
#
# Input:    totalBits - total number of bits in the bitmap (must be mult. of 8)
#           bitsToSet - an array of bits to set
#
# Returns:  A packed bitmap on success, undef otherwise.
#
##############################################################################
sub BuildBitmap
{
    my ($totalBits,       # total number of bits in the bitmap (must be mult. of 8)
        $bitsToSet) = @_; # an array of bits to set
    my $bitStr;

    if ($totalBits % 8) {
        print "Total Bits must be an even multiple of 8!\n";
        return undef;
    }

    # Get a sorted list of bits to set
    my @bits = sort {$a <=> $b} @$bitsToSet;
    my $theBit = shift @bits;
    my $i;

    for ($i = 0; $i < $totalBits; $i++) {
        if($theBit == $i) {
            $bitStr .= "1";
            if (scalar(@bits)) {
                $theBit = shift @bits;
                if($theBit > ($totalBits-1)) {
                    print "A specified bit ($theBit) in the input array is too big!\n";
                    return undef;
                }
                if($theBit == $i) {
                    print "A specified bit ($theBit) was duplicated in the input array!\n";
                    return undef;
                }
            }
            else {
                $theBit = -1;
            }
        }
        else {
            $bitStr .= "0";
        }
    }

    return pack("b$totalBits", $bitStr);
}

##############################################################################
# Name:     ParseBitmap
#
# Desc:     Parse a packed bitmap into an array of ints
#
# Input:    bitMap - a packed bitmap
#
# Returns:  An array of bitnums.
#
##############################################################################
sub ParseBitmap
{
    my ($bitMap) = @_;    # packed bitmap

    my $totalBits = length($bitMap) * 8;
    my $bitStr = unpack "b$totalBits", $bitMap;
    my @bitArray;
    my $i;
    my $theBit;

    for ($i = 0; $i < $totalBits; $i++) {
        $theBit = substr($bitStr, 0, 1);
        $bitStr = substr($bitStr, 1);

        if($theBit eq "1") {
            push @bitArray, $i;
        }
    }

    return @bitArray;
}

##############################################################################
#
#   All commands that start with a _ should not be used and are considered
#   private.
#
##############################################################################

##############################################################################
# Name:  _assignMirrorPartnerResponsePacket
#
# Desc: Handles a assign mirror partner response packet
#
##############################################################################
sub _assignMirrorPartnerResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    my $data = $parts{DATA};
    my $rsvd;

    if ($self->{CONTROLLER_TYPE} != CTRL_TYPE_BIGFOOT)
    {
        (
            $info{MIRROR_PARTNER}
        ) = unpack("L", $data);
    }
    else
    {
        (
            $rsvd,
            $info{STATUS_MRP},
            $info{LEN},
            $info{MIRROR_PARTNER}
        ) = unpack("a3CLL", $data);
    }

    return %info;
}


##############################################################################
# Name:  _fwHeaderPacket
#
# Desc: Handles a firmware header packet
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns:
##############################################################################
sub _fwHeaderPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    my $data = $parts{DATA};
    my $rsvd;

    (
    $rsvd,
    $info{STATUS_MRP},
    $info{LEN},
    ) = unpack("a3CL", $data);

    $data = substr($parts{DATA}, 8);

    my $rsvd0;
    my $rsvd1;

    (
    $rsvd0,
    $info{MAGIC_NUMBER},
    $rsvd1,
    $info{PRODUCT_ID},
    $info{TARGET},
    $info{REVISION},
    $info{REV_COUNT},
    $info{BUILD_ID},
    $info{SYSTEM_RLS},
    $info{TS_YEAR},
    $info{TS_MONTH},
    $info{TS_DATE},
    $info{TS_DAY},
    $info{TS_HOURS},
    $info{TS_MINUTES},
    $info{TS_SECONDS}
    ) = unpack("a32 L L L L a4 a4 a4 a4 S CCCCCC", $data);

    return %info;
}


##############################################################################
# Name:  _fwSysRelPacket
#
# Desc: Handles a firmware system release packet
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns:
##############################################################################
sub _fwSysRelPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    my $data = $parts{DATA};

    (
    $info{SYSREL},
    $info{TAG}
    ) = unpack("LA8", $data);

    return %info;
}

##############################################################################
# Name:  _scsiioResponsePacket
#
# Desc: Handles a scsiio packet
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns:
##############################################################################
sub _scsiioResponsePacket
{
    my ($self,
            $seq,
            $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    my $data = $parts{DATA};
    my $retLen;

    (
     $info{SENSE_KEY},
     $info{ADTL_SENSE_CODE},
     $info{ADTL_SENSE_CODE_QUAL},
     $retLen
    ) = unpack("CCCL", $data);

    $info{DATA} = substr($parts{DATA}, 7);

    return %info;
}

##############################################################################
# Name:  _diskREADWRITEResponsePacket
#
# Desc: Handles a disk Read/Write packet
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
##############################################################################
sub _diskREADWRITEResponsePacket
{
    my ($self,
            $seq,
            $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};
    ($info{DATALENGTH}, $info{DATA}) = unpack("La512", $parts{DATA});
    return %info;
}

##############################################################################
# Name:  _genericResponsePacket
#
# Desc: Handles a generic response packet (no data expected)
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: Empty hash.
#
##############################################################################
sub _genericResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};
    $info{CONTROLLER_TYPE} = $parts{CONTROLLER_TYPE};

    return %info;
}

##############################################################################
# NAME:     _globalCachePacket
#
# DESC:     Handles a status response packet
#
# INPUT:    scalar  $sequenceID         Sequence id
#           scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Output:
#
##############################################################################
sub _globalCachePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    my $rsvd;
    my $rsvd2;
    my $rsvd3;

    (
    $rsvd,
    $info{STATUS_MRP},
    $info{LEN},
    $info{CA_STATUS},
    $rsvd2,
    $info{CA_BATTERY},
    $info{CA_STOPCNT},
    $info{CA_SIZE},
    $info{CA_MAXCWR},
    $info{CA_MAXSGL},
    $info{CA_NUMTAGS},
    $info{CA_TAGSDIRTY},
    $info{CA_TAGSRESIDENT},
    $info{CA_TAGSFREE},
    $info{CA_TAGSFLUSHIP},
    $info{CA_NUMBLKS},
    $info{CA_BLOCKSDIRTY},
    $info{CA_BLOCKRESIDENT},
    $info{CA_BLOCKFREE},
    $info{CA_BLKSFLUSHIP},
    $rsvd3
    ) = unpack("a3CLCCCCLLLLLLLLLLLLLa8", $parts{DATA});

    return %info;
}

##############################################################################
# Name:  _countResponsePacket
#
# Desc: Handles a count response packet
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
#       STATUS                  status of the command
#       COUNT                   count of physical disks
#
##############################################################################
sub _countResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    # TODO: Check command code if possible

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};
    ($info{COUNT}) = unpack("S", $parts{DATA});

    return %info;
}

##############################################################################
# Name:  _listResponsePacket
#
# Desc: Handles a list response packet
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
#       STATUS                  status of the command
##############################################################################
sub _listResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;

    # TODO: Check command code if possible

    my %parts = disassembleXiotechPacket($recvPacket);

    my @idlist;

    (
    $info{COUNT}
    ) = unpack("S", $parts{DATA});

    for ($i = 0; $i < $info{COUNT}; $i++)
    {
        my $start = 2 + (2 * $i);
        my $str = substr($parts{DATA}, $start, 2);

        $idlist[$i] = unpack("S", substr($parts{DATA}, $start, 2));
    }

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};
    $info{LIST} = [@idlist];

    return %info;
}

##############################################################################
# Name:  _portListResponsePacket
#
# Desc: Handles a port list response packet
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
#       STATUS                  status of the command
##############################################################################
sub _portListResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;
    my $rsvd;
    my @idlist;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{NDEVS} = unpack("S", $parts{DATA});

    for ($i = 0; $i < $info{NDEVS}; $i++)
    {
        my $start = 2 + (2 * $i);
        my $str = substr($parts{DATA}, $start, 2);

        $idlist[$i] = unpack("S", $str);
    }


    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};
    $info{LIST} = [@idlist];

    return %info;
}

##############################################################################
# Name:  _stateResponsePacket
#
# Desc: Handles a 32 bit state response packet
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
#       STATUS                  status of the command
##############################################################################
sub _stateResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;
    my $rsvd;
    my @idlist;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    (
    $info{STATE}
    ) = unpack("L", $parts{DATA});

    return %info;
}

##############################################################################
# Name:  _powerUpStateResponsePacket
#
# Desc: Handles a power up state response packet
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
#       STATUS                  status of the command
##############################################################################
sub _powerUpStateResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;
    my $rsvd;
    my @idlist;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    (
    $info{STATE},
    $info{ADDITIONALSTATE}
    ) = unpack("SS", $parts{DATA});

    return %info;
}

##############################################################################
# NAME:     _resyncDataResponsePacket
#
# DESC:     Handles a resync data information packet
#
# INPUT:    scalar  $sequenceID         Sequence id
#           scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# OUTPUT:
##############################################################################
sub _resyncDataResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $count;
    my @cors;
    my @dtlcpy;
    my @rdata;
    my $rsvd;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    (
    $rsvd,
    $info{STATUS_MRP},
    $info{LEN},
    $info{COUNT},
    $info{FORMAT},
    $info{STRCTSIZE},
    ) = unpack("a3CLSCC", $parts{DATA});

    # Calculate the number of COR items contained in the response
    # data.  The number will be the total length minus the header
    # and count (12 bytes) divided by the bytes per COR (192).
    #
    # If the number of bytes changes in the COR then this decoder
    # will need to be adjusted.

    for (my $i = 0; $i < $info{COUNT}; $i++)
    {
        if ($info{FORMAT} == 0)
        {
            my $start = 12 + (192 * $i);

            (
            $rsvd,
            $cors[$i]{LINK},
            $cors[$i]{SCD},
            $cors[$i]{DCD},
            $cors[$i]{CM},

            $cors[$i]{SRCVDD},
            $cors[$i]{DESTVDD},
            $cors[$i]{RMAPTBL},
            $cors[$i]{COPYSTATE},
            $cors[$i]{FLAGS},
            $cors[$i]{CRSTATE},
            $cors[$i]{MIRRORSTATE},

            $cors[$i]{LABEL},

            $cors[$i]{UOPS},
            $cors[$i]{TMR1},
            $cors[$i]{RID},
            $cors[$i]{RCSN},
            $cors[$i]{RCSCL},
            $cors[$i]{RCSVD},
            $cors[$i]{RCDCL},
            $cors[$i]{RCDVD},

            $cors[$i]{RSSN},
            $cors[$i]{RDSN},
            $cors[$i]{RSCL},
            $cors[$i]{RSVD},
            $cors[$i]{RDCL},
            $cors[$i]{RDVD},
            $cors[$i]{GID},
            $cors[$i]{OCSERT1},
            $cors[$i]{OCSERT2},
            $cors[$i]{OCSETO},

            $cors[$i]{RCC},
            $cors[$i]{TOTALSEGS},
            $cors[$i]{PMPSN},
            $cors[$i]{SMPSN},

            $cors[$i]{OCSEPTR},
            $cors[$i]{OCSECEV},
            $cors[$i]{OCSECST},
            $cors[$i]{OCSELEV},
            $cors[$i]{OCSELST},
            $cors[$i]{POWNER},
            $cors[$i]{SOWNER},

            $cors[$i]{CCSEPTR},
            $cors[$i]{CCSECEV},
            $cors[$i]{CCSECST},
            $cors[$i]{CCSELEV},
            $cors[$i]{CCSELST},
            $cors[$i]{STNVRAM},
            $cors[$i]{SEQNUM},
            $cors[$i]{OSTINDX},
            $cors[$i]{CSTINDX},

            $cors[$i]{OSTAREA},

            $cors[$i]{CSTAREA},

            $cors[$i]{RESPSN},
            $cors[$i]{TRANSCPO},
            $cors[$i]{TRANSCSO},
            $cors[$i]{TRANSRMAP},

            $cors[$i]{TREGNUM},
            $rsvd,
            ) = unpack("a$start LLLL LLLCCCC a16 SSLLCCCC LLCCCCCCCC LLLL LCCCCLL LCCCCLSCC a16 a16 LLLL La12", $parts{DATA});
        }
        elsif ($info{FORMAT} == 1)
        {
            my $start = 12 + (28 * $i);

            (
            $rsvd,
            $dtlcpy[$i]{RCSN},
            $dtlcpy[$i]{RID},
            $dtlcpy[$i]{RCSCL},
            $dtlcpy[$i]{RCSDV},
            $dtlcpy[$i]{RCDCL},
            $dtlcpy[$i]{RCDDV},
            $dtlcpy[$i]{CORCSTATE},
            $dtlcpy[$i]{CORRSTATE},
            $dtlcpy[$i]{CORMSTATE},
            $dtlcpy[$i]{CMCSTATE},
            $dtlcpy[$i]{SP2HDLR},
            $dtlcpy[$i]{STYPE},
            $dtlcpy[$i]{DP2HDLR},
            $dtlcpy[$i]{DTYPE},
            $dtlcpy[$i]{VMIRROR},
            $dtlcpy[$i]{VATTR},
            $dtlcpy[$i]{OWNER},
            $dtlcpy[$i]{CTYPE},
            ) = unpack("a$start LLCCCCCCCC CCCCCCCC", $parts{DATA});
        }
        else
        {
            my $start = 12 + (16 * $i);

            (
            $rsvd,
            $rdata[$i]{TRECORD},
            ) = unpack("a$start a16", $parts{DATA});
        }
    }


    # If the data format is a "Copies Not Paused Map", save the
    # map as one continuous array.  I know this is redundant but
    # it makes it easier to decode later.
    if ($info{FORMAT} == 3)
    {
        $info{MIRROR_IO_STATUS_MAP} = unpack ("a64", substr($parts{DATA}, 12));
    }


    if ($info{FORMAT} == 0)
    {
        $info{CORS} = [@cors];
    }
    elsif ($info{FORMAT} == 1)
    {
        $info{DTLCPY} = [@dtlcpy];
    }
    elsif (($info{FORMAT} == 2) ||
           ($info{FORMAT} == 3))
    {
        $info{RDATA} = [@rdata];
    }

    return %info;
}


##############################################################################
# NAME:     _scrubInfoPacket
#
# DESC:     Handles a scrubbing information packet
#
# INPUT:    scalar  $sequenceID         Sequence id
#           scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# OUTPUT:
##############################################################################
sub _scrubInfoPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    my $rsvd1;
    my $rsvd2;

    (
    $rsvd1,
    $info{STATUS_MRP},
    $info{LEN},
    $info{SSTATE},
    $rsvd2,
    $info{PSTATE},
    $info{PASSES},
    $info{SCRUBP},
    $info{SCANR},
    $info{SCRUBB},
    $info{SCANB}
    ) = unpack("a3CLCa3LLSSLL", $parts{DATA});

    if ($info{SSTATE} & SCRUB_ENABLE)
    {
        $info{SCRUBBING} = "ENABLED";
    }
    else
    {
        $info{SCRUBBING} = "DISABLED";
    }
#
#
    if ($info{PSTATE} & SCRUB_PC_MARKED_MASK)
    {
        $info{PC_MARKED} = "YES";
    }
    else
    {
        $info{PC_MARKED} = "NO";
    }
#
    if ($info{PSTATE} & SCRUB_PC_CORRUPT_MASK)
    {
        $info{PC_CORRUPT} = "YES";
    }
    else
    {
        $info{PC_CORRUPT} = "NO";
    }
#
    if ($info{PSTATE} & SCRUB_PC_SPECIFIC_MASK)
    {
        $info{PC_SPECIFIC} = "SPECIFIC";
    }
    else
    {
        $info{PC_SPECIFIC} = "ALL";
    }
#
    if ($info{PSTATE} & SCRUB_PC_CLEARLOGS_MASK)
    {
        $info{PC_CLEARLOGS} = "YES";
    }
    else
    {
        $info{PC_CLEARLOGS} = "NO";
    }
#
    if ($info{PSTATE} & SCRUB_PC_1PASS_MASK)
    {
        $info{PC_1PASS} = "SINGLE";
    }
    else
    {
        $info{PC_1PASS} = "CONTINUOUS";
    }
#
    if ($info{PSTATE} & SCRUB_PC_CORRECT_MASK)
    {
        $info{PC_CORRECT} = "YES";
    }
    else
    {
        $info{PC_CORRECT} = "NO";
    }
#
    if ($info{PSTATE} & SCRUB_PC_ENABLE_MASK)
    {
        $info{PC_ENABLE} = "ENABLED";
    }
    else
    {
        $info{PC_ENABLE} = "DISABLED";
    }

    return %info;
}

##############################################################################
# Name:     _serialNumGetPacket
#
# Desc:     Handles a serial number get response packet
#
# In:
#
# Returns:
##############################################################################
sub _serialNumGetPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_DEBUG_GET_SER_NUM_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};
        ($info{SERIAL_NUM}) = unpack("L", $parts{DATA});
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual disk info packet\n");
    }

    return %info;
}

##############################################################################
# Name:  _statusResponsePacket
#
# Desc: Handles a status response packet
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns:
#
##############################################################################
sub _statusResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    my $rsvd;
    (
    $rsvd,
    $info{STATUS_MRP},
    $info{LEN}) = unpack("a3CL", $parts{DATA});

    return %info;
}

##############################################################################
# Name:  _readMemResponsePacket
#
# Desc: Handles a response packet from a memory read request.
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: hash with the follwing (empty if failure):
#               STATUS
#               LENGTH
#               RD_DATA
#
##############################################################################
sub _readMemResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    my %rdData;

    logMsg("_readMemResponsePacket...begin\n");

    if (defined($recvPacket))
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $rdData{STATUS} = $parts{STATUS};
        $rdData{ERROR_CODE} = $parts{ERROR_CODE};
        ($rdData{LENGTH}) = unpack "L", $parts{DATA};
        $rdData{RD_DATA} = substr $parts{DATA}, 4;
    }

    return %rdData;
}

##############################################################################
# Name:  _readFIDResponsePacket
#
# Desc: Handles a response packet from a FID read request.
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: hash with the following (empty if failure):
#               STATUS
#               LENGTH
#               RD_DATA
#
##############################################################################
sub _readFIDResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    my %rdData;

    logMsg("_readFIDResponsePacket...begin\n");

    if (defined($recvPacket))
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $rdData{STATUS} = $parts{STATUS};
        $rdData{ERROR_CODE} = $parts{ERROR_CODE};
        $rdData{RD_DATA} = $parts{DATA};
    }

    return %rdData;
}


##############################################################################
# Name:  _readMpxResponsePacket
#
# Desc: Handles a response packet from an MPX read request.
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: hash with the following (empty if failure):
#               STATUS
#               LENGTH
#               RD_DATA
#
##############################################################################
sub _readMpxResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    my %rdData;

    logMsg("_readMpxResponsePacket...begin\n");

    if (defined($recvPacket))
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my ($subCmdCode, $partX, $ofN, $flags, $parm1, $parm2) =
            unpack("CCCCLL", $parts{DATA});

        $rdData{STATUS}     = $parts{STATUS};
        $rdData{ERROR_CODE} = $parts{ERROR_CODE};
        $rdData{PARTX}      = $partX;
        $rdData{OFN}        = $ofN;
        $rdData{FLAGS}      = $flags;
        $rdData{RC}         = $parm1;
        $rdData{RD_DATA}    = substr($parts{DATA}, 12);

#        print "$rdData{STATUS}     \n";
#        print "$rdData{ERROR_CODE} \n";
#        print "$rdData{PARTX}      \n";
#        print "$rdData{OFN}        \n";
#        print "$rdData{FLAGS}      \n";
#        print "$rdData{RC}         \n";
#        print "$rdData{RD_DATA}    \n";
    }

    return %rdData;
}


##############################################################################
# Name:  _powerUpStateResponsePacket
#
# Desc: Handles a power up state response packet
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
#       STATUS                  status of the command
##############################################################################
sub _cfgoptionResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my $msg = " ";

    my %info;
    my $i;
    my $rsvd;
    my @idlist;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    (
    $rsvd,
    $info{STATUS_MRP},
    $info{LEN},
    $info{OPTION}
    ) = unpack("a3CLL", $parts{DATA});

    return %info;
}

##############################################################################
# Name:  _structureResponsePacket
#
# Desc: Handles a response packet from a stucture info request.
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: hash with the follwing (empty if failure):
#               STATUS
#               LENGTH
#               RD_DATA
#
##############################################################################
sub _structureResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    my %rdData;

    logMsg("_structureResponsePacket...begin\n");

    if (defined($recvPacket))
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $rdData{STATUS} = $parts{STATUS};
        $rdData{ERROR_CODE} = $parts{ERROR_CODE};
        $rdData{RD_DATA} = $parts{DATA};
    }

    return %rdData;
}

##############################################################################
# Name:     _modeDataResponsePacket
#
# Desc:     Handles a mode data get response packet
#
# In:
#
# Returns:
##############################################################################
sub _modeDataResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $rsvd1;
    my $rsvd2;
    my $rsvd3;
    my $rsvd4;

    if (commandCode($recvPacket) == PI_MISC_GET_MODE_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};
        ($info{CCB_MODE_BITS1},
         $info{CCB_MODE_DPRINTF_BITS},
         $info{CCB_MODE_RSVD1_BITS},
         $info{CCB_MODE_RSVD2_BITS},
         $info{PROC_MODE_BITS1},
         $info{PROC_MODE_BITS2},
         $info{PROC_MODE_BITS3},
         $info{PROC_MODE_BITS4}) = unpack(MODEDATA_BITS_T, $parts{DATA});
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual disk info packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _deviceCountResponsePacket
#
# Desc:     Handles a device count response packet
#
# In:
#
# Returns:
##############################################################################
sub _deviceCountResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_MISC_GET_DEVICE_COUNT_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{COUNT}
        ) = unpack("a3CLL", $parts{DATA});
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a device count info packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _deviceNameResponsePacket
#
# Desc:     Handles a device name response packet
#
# In:
#
# Returns:
##############################################################################
sub _deviceNameResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_PROC_NAME_DEVICE_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{NAME}
        ) = unpack("a3CLa16", $parts{DATA});
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual disk info packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _deviceConfigGetResponsePacket
#
# Desc:     Handles a device config get response packet
#
# In:
#
# Returns:
##############################################################################
sub _deviceConfigGetResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;
    my $rsvd;

    if (commandCode($recvPacket) == PI_MISC_GETDEVCONFIG_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the primary controller bitmap
        (
        $info{COUNT},
        $rsvd
        ) = unpack("Sa2", $parts{DATA});

        my @devices;

        for ($i = 0; $i < $info{COUNT}; $i++)
        {
            my $start = 4 + (32 * $i);

            (
            $devices[$i]{DEVVENDOR},
            $devices[$i]{DEVPRODID},
            $devices[$i]{DEVFLAGS}[0],
            $devices[$i]{DEVFLAGS}[1],
            $devices[$i]{DEVFLAGS}[2],
            $devices[$i]{DEVFLAGS}[3],
            $devices[$i]{DEVFLAGS}[4],
            $devices[$i]{DEVFLAGS}[5],
            $devices[$i]{DEVFLAGS}[6],
            $devices[$i]{DEVFLAGS}[7]
            ) = unpack("a8a16CCCCCCCC", substr($parts{DATA}, $start));
        }

        $info{DEVICES} = [@devices];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a device config get info packet\n");
    }

    return %info;
}

##############################################################################
# NAME:     _deviceListResponsePacket
#
# DESC:     Handles a device list response packet
#
# INPUT:    scalar  $sequenceID         Sequence id
#           scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Output:
#
##############################################################################
sub _deviceListResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %port;
    my %node;

    my %parts = disassembleXiotechPacket($recvPacket);

    my $rsvd;
    my $rsvd2;
    my @idlist;
    my $i;

    (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{NDEVS},
        $rsvd
        ) = unpack("a3CLSS", $parts{DATA});

    for ($i = 0; $i < $info{NDEVS}; $i++)
    {
        my $start = 12 + (32 * $i);
        my $str = substr($parts{DATA}, $start, 32);

        (
            $idlist[$i]{LID},
            $idlist[$i]{MST},
            $idlist[$i]{SST},
            $idlist[$i]{PORT_ID},
            $port{LO_LONG}, $port{HI_LONG},
            $node{LO_LONG}, $node{HI_LONG},
            $idlist[$i]{RSVD1},
            $idlist[$i]{RSVD2}
        )  = unpack("SCCLNNNNLL", substr($parts{DATA}, $start, 32));

        $idlist[$i]{PORT_WWN}       = longsToBigInt(%port);
        $idlist[$i]{PORT_WWN_LO}    = $port{LO_LONG};
        $idlist[$i]{PORT_WWN_HI}    = $port{HI_LONG};
        $idlist[$i]{NODE_WWN}       = longsToBigInt(%node);
        $idlist[$i]{NODE_WWN_LO}    = $node{LO_LONG};
        $idlist[$i]{NODE_WWN_HI}    = $node{HI_LONG};

    }
    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};
    $info{LIST} = [@idlist];

    return %info;
}

##############################################################################
# NAME:     _devicePathResponsePacket
#
# DESC:     Handles a device path response packet
#
# INPUT:    scalar  $sequenceID         Sequence id
#           scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Output:
#
##############################################################################
sub _devicePathResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleXiotechPacket($recvPacket);

    my @idlist;
    my $rsvd;

    (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{NDEVS},
        $info{SIZE}
    ) = unpack("a3CLSS", $parts{DATA});

    #print "NDEVS: $info{NDEVS}\n";
    #print "SIZE:  $info{SIZE}\n";

    # This function is used for device path info read via a Packet
    # Interface command and to process cache data.  Since the cache
    # data is not packed we need a way to skip empty entries.  A
    # change was recently made to CCB code to fill the cache with 0xFFFF.
    # Since valid ID values are < 0xFFFF this will be used to distinguish
    # valid cache entries.
    my $i = 0;
    my $loopCount = 0;

    while ($i < $info{NDEVS})
    {
        my $start = (12 + ($info{SIZE} * $loopCount));
        my $str = substr($parts{DATA}, $start, $info{SIZE});

        if ($info{SIZE} == 8)
        {
            (
                $idlist[$i]{PID},
                $idlist[$i]{PATH_COUNT},
                $idlist[$i]{PATH1},
                $idlist[$i]{PATH2},
                $idlist[$i]{PATH3},
                $idlist[$i]{PATH4}
            )  = unpack("SSCCCC", $str);
        }
        else
        {
            (
                $idlist[$i]{PID},
                $idlist[$i]{BIT_PATH}
            )  = unpack("SS", $str);
        }

        # Increment the item count if the entry is valid
        if ($idlist[$i]{PID} != 0xFFFF)
        {
            $i++;
        }

        # Increment loop count to read the next entry
        $loopCount++;
    }
    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};
    $info{LIST} = [@idlist];

    return %info;
}

##############################################################################
# NAME:     _sosResponsePacket
#
# DESC:     Handles a sos response packet
#
# INPUT:    scalar  $sequenceID         Sequence id
#           scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Output:
#
##############################################################################
sub _sosResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleXiotechPacket($recvPacket);

    my $rsvd;
    my $rsvd2;
    my @idlist;
    my $i;

    (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $rsvd2,
        $info{NEXT},
        $info{PID},
        $info{FLAGS},
        $info{REMAIN},
        $info{TOTAL},
        $info{PDD},
        $info{OWNER},
        $info{COUNT},
        $info{CURRENT},
        $info{PCB}
        ) = unpack("a3CLa8LSSLLLLSSL", $parts{DATA});

    for ($i = 0; $i < $info{COUNT}; $i++)
    {
        my $start = 48 + (16 * $i);
        my $str = substr($parts{DATA}, $start, 16);

        (
            $idlist[$i]{GAP_SIZE},
            $idlist[$i]{SDA},
            $idlist[$i]{LEN},
            $idlist[$i]{RID}
        )  = unpack("LLLL", $str);
    }

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};
    $info{LIST} = [@idlist];

    return %info;
}

##############################################################################
# Name:     _systimePacket
#
# Desc:     Handles an system time get response packet
#
# In:
#
# Returns:
##############################################################################
sub _systimePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    (
        $info{TIME_LO},
        $info{TIME_HI},
    ) = unpack("LL", $parts{DATA});

    return %info;
}

##############################################################################
# Name:     _ipInfoPacket
#
# Desc:     Handles an IP get response packet
#
# In:
#
# Returns:
##############################################################################
sub _ipInfoPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    (
        $info{SERIAL_NUM},
        $info{IP_ADDR},
        $info{SUBNET_MASK},
        $info{GATEWAY_ADDR}
    ) = unpack("LNNN", $parts{DATA});

    return %info;
}

##############################################################################
# Name:     displayTime
#
# Desc:     Display controller time
#
# In:
#
# Returns:
##############################################################################
sub displayTime
{
    my ($self, %info) = @_;

    print "Cntl Time :" . 
           scalar(gmtime($info{TIME_LO})) . "\n";
}

##############################################################################
# Name:     displayIpInfo
#
# Desc:     Display an IP get response packet
#
# In:
#
# Returns:
##############################################################################
sub displayIpInfo
{
    my ($self, %info) = @_;

    $info{IP_ADDR} = $self->long2ip($info{IP_ADDR});
    $info{SUBNET_MASK} = $self->long2ip($info{SUBNET_MASK});
    $info{GATEWAY_ADDR} = $self->long2ip($info{GATEWAY_ADDR});

    print "Cntl Serial Num: $info{SERIAL_NUM}\n";
    print "IP Address:      $info{IP_ADDR}\n";
    print "Subnet Mask:     $info{SUBNET_MASK}\n";
    print "Gateway Address: $info{GATEWAY_ADDR}\n";
}


##############################################################################
# Name:     FmtLogPage32
#
# Desc:     Display log sense page 32h from Ario Data
#
# In:
#
# Returns:
##############################################################################
sub FmtLogPage32
{
    my ($self, $bufferPtr) = @_;

    my $offset = 4;
    my $msg = "Port Statistics\n\n Port 0\n";
    my $fmt;
    my $item1a;
    my $item1b;
    my $item2a;
    my $item2b;
    my $item3a;
    my $item3b;
    my $item4a;
    my $item4b;
    my $item5a;
    my $item5b;
    my $item6a;
    my $item6b;
    my $item7a;
    my $item7b;
    my $item8a;
    my $item8b;
    my $item9a;
    my $item9b;
    my $item10a;
    my $item10b;
    my $item11a;
    my $item11b;
    my $item12a;
    my $item12b;
    my $item13a;
    my $item13b;
    my $item14a;
    my $item14b;
    my $item15a;
    my $item15b;
    my $item16a;
    my $item16b;

    $fmt = sprintf("x%d NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN",$offset);      # generate the format string
    ($item1a, $item1b,
     $item2a, $item2b,
     $item3a, $item3b,
     $item4a, $item4b,
     $item5a, $item5b,
     $item6a, $item6b,
     $item7a, $item7b,
     $item8a, $item8b,
     $item9a, $item9b,
     $item10a, $item10b,
     $item11a, $item11b,
     $item12a, $item12b,
     $item13a, $item13b,
     $item14a, $item14b,
     $item15a, $item15b,
     $item16a, $item16b) = unpack $fmt , $$bufferPtr;

    $msg .= sprintf("   Time since reset:        0x%08x%08x\n", $item1a, $item1b);
    $msg .= sprintf("   Xmitted frames:          0x%08x%08x\n", $item2a, $item2b);
    $msg .= sprintf("   Recieved frames:         0x%08x%08x\n", $item3a, $item3b);
    $msg .= sprintf("   Xmitted words:           0x%08x%08x\n", $item4a, $item4b);
    $msg .= sprintf("   Received words:          0x%08x%08x\n", $item5a, $item5b);
    $msg .= sprintf("   LIP count:               0x%08x%08x\n", $item6a, $item6b);
    $msg .= sprintf("   NOS count:               0x%08x%08x\n", $item7a, $item7b);
    $msg .= sprintf("   Error frames:            0x%08x%08x\n", $item8a, $item8b);
    $msg .= sprintf("   Dumped frames:           0x%08x%08x\n", $item9a, $item9b);
    $msg .= sprintf("   Link failure count:      0x%08x%08x\n", $item10a, $item10b);
    $msg .= sprintf("   Loss of sync count:      0x%08x%08x\n", $item11a, $item11b);
    $msg .= sprintf("   Loss of signal count:    0x%08x%08x\n", $item12a, $item12b);
    $msg .= sprintf("   Prim seq error count:    0x%08x%08x\n", $item13a, $item13b);
    $msg .= sprintf("   Invalid xmit word count: 0x%08x%08x\n", $item14a, $item14b);
    $msg .= sprintf("   Invalid CRC count:       0x%08x%08x\n", $item15a, $item15b);
    $msg .= sprintf("   FCP init IO count:       0x%08x%08x\n", $item16a, $item16b);


    $offset += 128;                         # 128 bytes processed

    $msg .= "\n Port 1\n";

    $fmt = sprintf("x%d NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN",$offset);      # generate the format string
    ($item1a, $item1b,
     $item2a, $item2b,
     $item3a, $item3b,
     $item4a, $item4b,
     $item5a, $item5b,
     $item6a, $item6b,
     $item7a, $item7b,
     $item8a, $item8b,
     $item9a, $item9b,
     $item10a, $item10b,
     $item11a, $item11b,
     $item12a, $item12b,
     $item13a, $item13b,
     $item14a, $item14b,
     $item15a, $item15b,
     $item16a, $item16b) = unpack $fmt , $$bufferPtr;

    $msg .= sprintf("   Time since reset:        0x%08x%08x\n", $item1a, $item1b);
    $msg .= sprintf("   Xmitted frames:          0x%08x%08x\n", $item2a, $item2b);
    $msg .= sprintf("   Recieved frames:         0x%08x%08x\n", $item3a, $item3b);
    $msg .= sprintf("   Xmitted words:           0x%08x%08x\n", $item4a, $item4b);
    $msg .= sprintf("   Received words:          0x%08x%08x\n", $item5a, $item5b);
    $msg .= sprintf("   LIP count:               0x%08x%08x\n", $item6a, $item6b);
    $msg .= sprintf("   NOS count:               0x%08x%08x\n", $item7a, $item7b);
    $msg .= sprintf("   Error frames:            0x%08x%08x\n", $item8a, $item8b);
    $msg .= sprintf("   Dumped frames:           0x%08x%08x\n", $item9a, $item9b);
    $msg .= sprintf("   Link failure count:      0x%08x%08x\n", $item10a, $item10b);
    $msg .= sprintf("   Loss of sync count:      0x%08x%08x\n", $item11a, $item11b);
    $msg .= sprintf("   Loss of signal count:    0x%08x%08x\n", $item12a, $item12b);
    $msg .= sprintf("   Prim seq error count:    0x%08x%08x\n", $item13a, $item13b);
    $msg .= sprintf("   Invalid xmit word count: 0x%08x%08x\n", $item14a, $item14b);
    $msg .= sprintf("   Invalid CRC count:       0x%08x%08x\n", $item15a, $item15b);
    $msg .= sprintf("   FCP init IO count:       0x%08x%08x\n", $item16a, $item16b);


        #  Time Since Reset (Port 0)
        #  Transmitted Frames (Port 0)
        #  Received Frames (Port 0)
        #  Transmitted Words (Port 0)
        #  Received Words (Port 0)
        #  Lip Count (Port 0)
        #  Nos Count (Port 0)
        #  Error Frames (Port 0)
        #  Dumped Frames (Port 0)
        #  Link Failure Count (Port 0)
        #  Loss Of Sync Count (Port 0)
        #  Loss Of Signal Count (Port 0)
        #  Primitive Sequence  Error Count (Port 0)
        #  Invalid Transmitted Word Count (Port 0)
        #  Invalid Crc Count (Port 0)
        #  Fcp Initiator Io Count (Port 0)
        #  Time Since Reset (Port 1)
        #  Transmitted Frames (Port 1)
        #  Received Frames (Port 1)
        #  Transmitted Words (Port 1)
        #  Received Words (Port 1)
        #  Lip Count (Port 1)
        #  Nos Count (Port 1)
        #  Error Frames (Port 1)
        #  Dumped Frames (Port 1)
        #  Link Failure Count (Port 1)
        #  Loss Of Sync Count (Port 1)
        #  Loss Of Signal Count (Port 1)
        #  Primitive Sequence  Error Count (Port 1)
        #  Invalid Transmitted Word Count (Port 1)
        #  Invalid Crc Count (Port 1)
        #  Fcp Initiator Io Count (Port 1)

    return $msg;

}


##############################################################################
# Name:     mmtest
#
# Desc:
#
# Input:
#
# Returns:
##############################################################################
sub mmtest
{
    my ($self, $option, $offset) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF],
                ['d', 0, 0xFFFFFFFF],
                ["mmtest"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_DEBUG_MRMMTEST_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCCL",
                    $option,
                    0,0,0,
                    $offset);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}


1;

#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
