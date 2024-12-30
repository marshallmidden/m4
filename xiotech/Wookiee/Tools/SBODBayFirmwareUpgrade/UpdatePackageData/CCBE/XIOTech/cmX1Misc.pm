# $Header$
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2002, 2003  Xiotech
# ======================================================================
# $RCSfile$
#
# Author:   Mark Schibilla
#
# Purpose:  Support for Miscellaneous X1 related stuff
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
# Name:     X1GetDevConfig
#
# Desc:     Get the device configuration information.
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
sub X1GetDevConfig
{
    my ($self) = @_;
    my %rsp;
    my $rc;

    logMsg("begin\n");

    my $packet = assembleX1Packet(X1PKT_GET_DEV_CONFIG, undef);

    %rsp = $self->_handleX1SyncResponse($packet,
                                        X1RPKT_GET_DEV_CONFIG,
                                        \&_handleResponseX1GetDevConfig);

    if (%rsp)
    {
        logMsg("X1GetDevConfig successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetDevConfig failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     X1PutDevConfig
#
# Desc:     Updates the device configuration information.
#
# Input:    Device configuration array.
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#           (lots...)
#
##############################################################################
sub X1PutDevConfig
{
    my ($self, $inputDevices) = @_;
    my @devices;
    my $i;
    my $iFlags;
    my $rc;
    my %rsp;

    logMsg("begin\n");

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

    my $packet = assembleX1Packet(X1PKT_PUT_DEV_CONFIG, $data);

    %rsp = $self->_handleX1SyncResponse($packet,
                                        X1RPKT_PUT_DEV_CONFIG,
                                        \&_handleResponseX1PutDevConfig);

    if (%rsp)
    {
        logMsg("X1PutDevConfig successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1PutDevConfig failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     X1GetVCGInfo
#
# Desc:     Get X1 VCG Info
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
sub X1GetVCGInfo
{
    my ($self) = @_;
        
    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_VCG_INFO, undef);

    %rsp = $self->_handleX1SyncResponse($packet,
                                        X1RPKT_VCG_INFO,
                                        \&_handleResponseX1GetVCGInfo);

    if (%rsp)
    {
        logMsg("X1GetVCGInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetVCGInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     X1GetProbe
#
# Desc:     Get X1 Probe Packet
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
sub X1GetProbe
{
    my ($self) = @_;
        
    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $data = pack "CCC", 0xFE, 0, 0; # set RMC_COMPAT very high for now,
                                       # major ver, minor ver

    my $packet = assembleX1Packet(X1PKT_PROBE, $data);

    %rsp = $self->_handleX1SyncResponse($packet,
                                        X1RPKT_PROBE,
                                        \&_handleResponseX1GetProbe);

    if (%rsp)
    {
        logMsg("X1GetProbe successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetProbe failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetCPULoad
#
# Desc:     Get X1 CPU Load Packet
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
sub X1GetCPULoad
{
    my ($self) = @_;
        
    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_CPU_LOAD, undef);

    %rsp = $self->_handleX1SyncResponse($packet,
                                        X1RPKT_GET_CPU_LOAD,
                                        \&_handleResponseX1GetCPULoad);

    if (%rsp)
    {
        logMsg("X1GetCPULoad successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetCPULoad failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     X1ResyncGetInfo
#
# Desc:     Get the resync information.
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
sub X1ResyncGetInfo
{
    my ($self) = @_;
    my %rsp;
    my $rc;

    logMsg("begin\n");

    my $packet = assembleX1Packet(X1PKT_GET_RESYNC_INFO, undef);

    %rsp = $self->_handleX1SyncResponse($packet,
                                        X1RPKT_GET_RESYNC_INFO,
                                        \&_handleResponseX1ResyncGetInfo);

    if (%rsp)
    {
        logMsg("X1ResyncGetInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1ResyncGetInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     X1ResyncControl
#
# Desc:     Submits the resync control operation.
#
# Input:    
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#           (lots...)
#
##############################################################################
sub X1ResyncControl
{
    my ($self, $fc, $rid, $csn, $gid, $name) = @_;

    my %rsp;
    my $rc;

    logMsg("begin\n");

    my $data = pack("CLLLa16",
                    $fc,
                    $rid,
                    $csn,
                    $gid,
                    $name);

    my $packet = assembleX1Packet(X1PKT_RESYNC_CONTROL, $data);

    %rsp = $self->_handleX1SyncResponse($packet,
                                        X1RPKT_RESYNC_CONTROL,
                                        \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1ResyncControl successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1ResyncControl failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     _handleResponseX1PutDevConfig
#
# Desc:     Handle an X1 Put Device Configuration return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################

use constant X1_GET_DEV_CONFIG =>
           "S           # Count
            a2";        # rsvd

sub _handleResponseX1GetDevConfig
{
    my ($self, $recvPacket) = @_;
    my $rsvd;
    my %info;
    my $i;
    my @devices;

    logMsg("begin\n");

    my %parts = disassembleX1Packet($recvPacket);

    if (defined($parts{DATA}))
    {
        # Unpack the data for the VCG
        (
        $info{COUNT},
        $rsvd
        ) = unpack X1_GET_DEV_CONFIG, $parts{DATA};

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

    return %info;
}


##############################################################################
# Name:     _handleResponseX1PutDevConfig
#
# Desc:     Handle an X1 Put Device Configuration return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################

use constant X1_PUT_DEV_CONFIG =>
           "C";         # Status

sub _handleResponseX1PutDevConfig
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my $rsvd;
    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    if (defined($parts{DATA}))
    {
        # Unpack the data for the VCG
        ($info{STATUS}) = unpack X1_PUT_DEV_CONFIG, $parts{DATA};
    }

    return %info;
}


##############################################################################
# Name:     _handleResponseX1GetVCGInfo
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

# X1_VCG_CTRL_INFO
use constant X1_VCG_CTRL_INFO =>
           "L           # serialNumber
            N           # ipAddress
            L           # failureState
            a3          # rsvd
            C           # amIMaster
            L";         # rsvd2

use constant X1_VCG_INFO =>
           "L           # vcgID
            L           # powerUpState
            S           # vcgMaxControllers
            S";         # vcgCurrentControllers

sub _handleResponseX1GetVCGInfo
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my $rsvd;
    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    if (defined($parts{DATA}))
    {
        # Unpack the data for the VCG
        (
        $info{VCG_ID},
        $info{VCG_PWR_UP_STATE},
        $info{VCG_MAX_NUM_CONTROLLERS},
        $info{VCG_CURRENT_NUM_CONTROLLERS}
        ) = unpack X1_VCG_INFO, $parts{DATA};

        # Now unpack the data for each controller in the VCG        
        my @controllers;
        for (my $i = 0; $i < $info{VCG_MAX_NUM_CONTROLLERS}; $i++)
        {
            my $start = 12 + (20 * $i);
            (
            $controllers[$i]{SERIAL_NUMBER},
            $controllers[$i]{IP_ADDRESS},
            $controllers[$i]{FAILURE_STATE},
            $rsvd,
            $controllers[$i]{AM_I_MASTER},
            $rsvd
            ) = unpack (X1_VCG_CTRL_INFO, substr($parts{DATA}, $start));

            $controllers[$i]{IP_ADDRESS} = $self->long2ip($controllers[$i]{IP_ADDRESS});
        }
        $info{CONTROLLERS} = [@controllers];
    }

    return %info;
}


##############################################################################
# Name:     _handleResponseX1GetProbe
#
# Desc:     Handle an X1 Probe return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################

# X1_VCG_CTRL_INFO
use constant X1_PROBE_INFO =>
           "C           # length
            L           # serialNumber
            a8";        # rmcName[X1_PROBE_NAME_LEN]
            
sub _handleResponseX1GetProbe
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my $rsvd;
    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    if (defined($parts{DATA}))
    {
        # Unpack the data for the VCG
        (
        $info{LENGTH},
        $info{SERIAL_NUM},
        $info{RMC_NAME}
        ) = unpack X1_PROBE_INFO, $parts{DATA};
    }

    return %info;
}


##############################################################################
# Name:     _handleResponseX1GetCPULoad
#
# Desc:     Handle an X1 Processor Utilization return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################

# X1_CPU_LOAD
use constant X1_CPU_LOAD =>
           "C           # FE proc free
            a24         # Reserved
            C           # BE proc free
            a15";       # Reserved 
sub _handleResponseX1GetCPULoad
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my $rsvd;
    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    if (defined($parts{DATA}))
    {
        # Unpack the data for the CPU Load
        (
        $info{FEPROC},
        $rsvd,
        $info{BEPROC},
        $rsvd
        ) = unpack X1_CPU_LOAD, $parts{DATA};
    }

    return %info;
}


##############################################################################
# Name:     _handleResponseX1ResyncGetInfo
#
# Desc:     Handle an X1 Resync Get Info return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1ResyncGetInfo
{
    my ($self, $recvPacket) = @_;
    my $rsvd;
    my %info;
    my $i;
    my @x1Cors;

    logMsg("begin\n");

    my %parts = disassembleX1Packet($recvPacket);

    if (defined($parts{DATA}))
    {
        # Unpack the data for the VCG
        (
        $info{COUNT}
        ) = unpack "S", $parts{DATA};

        for ($i = 0; $i < $info{COUNT}; $i++)
        {
            my $start = 2 + (48 * $i);

            (
            $rsvd,
            $x1Cors[$i]{COPYSTATE},
            $x1Cors[$i]{FLAGS},
            $x1Cors[$i]{CRSTATE},
            $rsvd,
            $x1Cors[$i]{LABEL},
            $x1Cors[$i]{RID},
            $x1Cors[$i]{RCSN},
            $x1Cors[$i]{RCSCL},
            $x1Cors[$i]{RCSVD},
            $x1Cors[$i]{RCDCL},
            $x1Cors[$i]{RCDVD},
            $x1Cors[$i]{RSSN},
            $x1Cors[$i]{RDSN},
            $x1Cors[$i]{RSCL},
            $x1Cors[$i]{RSVD},
            $x1Cors[$i]{RDCL},
            $x1Cors[$i]{RDVD},
            $x1Cors[$i]{GID},
            $rsvd
            ) = unpack("a$start CCCC a16 LL CCCC LL CCCC Ca3", $parts{DATA});
        }

        $info{X1_CORS} = [@x1Cors];
    }

    return %info;
}


##############################################################################
# Name: DisplayX1GetDevConfig
#
# Desc: Print the virtual controller group information.
#
# In:   Virtual Controller Group Information Hash
##############################################################################
sub DisplayX1GetDevConfig
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
        $msg .= sprintf "  %8s  %16s  0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\n",
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
# Name: DisplayX1VCGInfo
#
# Desc: Print the virtual controller group information.
#
# In:   Virtual Controller Group Information Hash
##############################################################################
sub DisplayX1VCGInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $i;
    my @controllers = $info{CONTROLLERS};

    print "Virtual Controller Group Information:\n";
    printf "  VCG_ID:                %lu (0x%8.8x)\n", $info{VCG_ID}, $info{VCG_ID};
    printf "  VCG_POWER_UP_STATE:    %s\n", _getString_POWERUP($info{VCG_PWR_UP_STATE});
    printf "  VCG_MAX_NUM_CTRLS:     %hu\n", $info{VCG_MAX_NUM_CONTROLLERS};
    printf "  VCG_CURRENT_NUM_CTRLS: %hu\n", $info{VCG_CURRENT_NUM_CONTROLLERS};
    print  "\n";

    print  "  SERIAL# (HEX)            IP_ADDRESS        FAILURE_STATE              RANK\n";
    print  "  -----------------------  ----------------  -------------------------  ------\n";

    for (my $i = 0; $i < $info{VCG_MAX_NUM_CONTROLLERS}; $i++)
    {
        my $failureState = "";

        if ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_UNUSED)
        {
            $failureState = "UNUSED";
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_FAILED)
        {
            $failureState = "FAILED";
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_OPERATIONAL)
        {
            $failureState = "OPERATIONAL";
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_POR)
        {
            $failureState = "POWER ON READY"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_ADD_CONTROLLER_TO_VCG)
        {
            $failureState = "ADDING CTRL TO VCG"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_STRANDED_CACHE_DATA)
        {
            $failureState = "STRANDED CACHE DATA"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_FIRMWARE_UPDATE_INACTIVE)
        {
            $failureState = "FIRMWARE UPDATE INACTIVE"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_FIRMWARE_UPDATE_ACTIVE)
        {
            $failureState = "FIRMWARE UPDATE ACTIVE"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_UNFAIL_CONTROLLER)
        {
            $failureState = "UNFAIL CONTROLLER"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_VCG_SHUTDOWN)
        {
            $failureState = "VCG SHUTDOWN"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_INACTIVATED)
        {
            $failureState = "INACTIVATED"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_ACTIVATE)
        {
            $failureState = "ACTIVATE"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_DISASTER_INACTIVE)
        {
            $failureState = "DISASTER INACTIVE"
        }
        else
        {
            $failureState = "UNKNOWN";
        }

        my $master = "MASTER";

        if ($info{CONTROLLERS}[$i]{AM_I_MASTER} == 0)
        {
            $master = "SLAVE";
        }

        printf "  %-10lu (0x%8.8x)  %-16s  %-25s  %-11s\n",
                $info{CONTROLLERS}[$i]{SERIAL_NUMBER},
                $info{CONTROLLERS}[$i]{SERIAL_NUMBER},
                $info{CONTROLLERS}[$i]{IP_ADDRESS},
                $failureState,
                $master;
    }

    print "\n";
}


##############################################################################
# Name: DisplayX1Probe
#
# Desc: print the X1 Probe information
#
# In:   sos Information Hash
##############################################################################
sub DisplayX1Probe
{
    my (%info) = @_;

    logMsg("begin\n");
    
    my $msg = "";

    $msg .= sprintf "X1 Probe Info:\n\n";
    $msg .= sprintf "  Length of Serial Number (in bytes):  %d\n", $info{LENGTH};
    $msg .= sprintf "  Serial number:                       %d\n", $info{SERIAL_NUM};
    $msg .= sprintf "  Name:                                \"%s\"\n", $info{RMC_NAME};
        
    $msg .= sprintf "\n";
    
    return $msg;
}

##############################################################################
# Name: DisplayX1CPULoad
#
# Desc: print the X1 CPU Load information
#
# In:   sos Information Hash
##############################################################################
sub DisplayX1CPULoad
{
    my (%info) = @_;

    logMsg("begin\n");
    
    my $msg = "";

    $msg .= sprintf "X1 CPU Load (percent free):\n\n";
    $msg .= sprintf "  Front end processor: %3d\n", $info{FEPROC};
    $msg .= sprintf "  Back end processor:  %3d\n", $info{BEPROC};
        
    $msg .= sprintf "\n";
    
    return $msg;
}


##############################################################################
# Name: DisplayX1ResyncGetInfo
#
# Desc: Print the resync information
#
# In:   Resync Information Hash
##############################################################################
sub DisplayX1ResyncGetInfo
{
    my ($self, %info) = @_;
    my $i;
    my $msg;

    logMsg("begin\n");

    $msg .= sprintf "Resync Data (count: %d):\n", $info{COUNT};
    $msg .= sprintf "\n";

    $msg .= sprintf "  RID  GID  CSTATE  FLAGS  CRSTATE     RCSN     RCSCL RCSVD  RCDCL RCDVD     RSSN     RSCL RSVD     RDSN     RDCL RDVD        LABEL     \n";
    $msg .= sprintf "  ---  ---  ------  -----  -------  ----------  ----- -----  ----- -----  ----------  ---- ----  ----------  ---- ----  ----------------\n";

    for ($i = 0; $i < $info{COUNT}; $i++)
    {
        $msg .= sprintf "  %3lu  %3lu    0x%2.2x   0x%2.2x     0x%2.2x  0x%8.8x  %5lu %5lu  %5lu %5lu  0x%8.8x  %4lu %4lu  0x%8.8x  %4lu %4lu  %16s\n",
                        $info{X1_CORS}[$i]{RID},
                        $info{X1_CORS}[$i]{GID},
                        $info{X1_CORS}[$i]{COPYSTATE},
                        $info{X1_CORS}[$i]{FLAGS},
                        $info{X1_CORS}[$i]{CRSTATE},
                        $info{X1_CORS}[$i]{RCSN},
                        $info{X1_CORS}[$i]{RCSCL},
                        $info{X1_CORS}[$i]{RCSVD},
                        $info{X1_CORS}[$i]{RCDCL},
                        $info{X1_CORS}[$i]{RCDVD},
                        $info{X1_CORS}[$i]{RSSN},
                        $info{X1_CORS}[$i]{RSCL},
                        $info{X1_CORS}[$i]{RSVD},
                        $info{X1_CORS}[$i]{RDSN},
                        $info{X1_CORS}[$i]{RDCL},
                        $info{X1_CORS}[$i]{RDVD},
                        $info{X1_CORS}[$i]{LABEL};
    }

    $msg .= sprintf "\n";

    return $msg;
}


1;

##############################################################################
# Change log:
#
# $Log$
# Revision 1.2  2005/08/12 20:43:35  HoltyB
# TBolt00000000:  Fixed probe packet to send right amount of data
#
# Revision 1.1.1.1  2005/05/04 18:53:54  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.11  2004/09/21 18:20:30  NigburC
# TBolt00011260 - Completed the X1ResyncControl code.
# Reviewed by Mark Schibilla.
#
# Revision 1.10  2004/09/21 15:24:30  WilliamsJ
# TBolt00011344 - Merge of resync into main.
#
# Revision 1.9.6.1  2004/09/15 13:11:26  NigburC
# TBolt00011260 - Integrating the first pass at the X1 packet handlers for
# resync functionality.
# Reviewed by Mark Schibilla.
#
# Revision 1.9  2004/05/05 15:20:34  NigburC
# TBolt00010427 - Added new packet requests to support getting and setting
# of the device configuration information.  These code changes do not yet
# save the data to a persistent storage on the CCB.
# Reviewed by Mark Schibilla.
#
# Revision 1.8  2004/04/15 13:13:42  SchibillaM
# TBolt00010249: Added 8 bytes to last reserved field to make the packet the
# correct length for the existing XIOService code.
#
# Revision 1.7  2004/04/08 19:00:48  SchibillaM
# TBolt00010249: Change X1 Processor Utilization request to X1 Get CPU Loads
# which was already defined for Mag and exists in UI middleware.  Reviewed by Chris.
#
# Revision 1.6  2004/03/17 17:55:27  SchibillaM
# TBolt00010249: Add CCB, CCBE support for X1 processor utilization.  Reviewed by Chris.
#
# Revision 1.5  2003/12/15 19:20:12  RysavyR
# TBolt00009629: Added a small tweak to the X1 Probe Packet display routine.
#
# Revision 1.4  2003/12/15 18:15:14  SchibillaM
# TBolt00009629: Added support for the X1 Probe Packet.
#
# Revision 1.3  2003/08/25 21:43:36  McmasterM
# TBolt00008602: GeoRAID: Add "disaster mode" recovery state to CCB startup
# Changed the way the disaster safeguard was being set and reset by the system.
# This work is done to support Chris while he tests the powerup changes, but does
# not completely finish off this defect.
#
# Revision 1.2  2003/08/05 18:03:50  NigburC
# TBolt00008575 - Change the name of two power-up states (BE_READY and
# DISCOVERY) to make them more descriptive for what they do and added
# three additional power-up states for the updated RAID 5 processing.  Added
# a new function to convert the power-up state to a string value.
# Reviewed by Craig Menning.
#
# Revision 1.1  2003/04/30 12:35:52  SchibillaM
# TBolt00007922: Split caching code up into separate files.  Reviewed by CCB group.
#
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
