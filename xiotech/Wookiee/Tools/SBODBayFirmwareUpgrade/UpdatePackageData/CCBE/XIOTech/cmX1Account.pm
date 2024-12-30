# $Header$
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2002  Xiotech
# ======================================================================
# $RCSfile$
# Author: Randy Rysavy
#
# Purpose:
#   Wrapper for all the different XIOTech X1 Account commands
#   that can be sent to the XIOtech SAN system
##############################################################################
package XIOTech::cmdMgr;

use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;

use XIOTech::error;

use XIOTech::logMgr;

use strict;

#BEGIN {
#    use Exporter   ();
#    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);
#
#    # set the version for version checking
#    $VERSION     = 1.00;
#    # if using RCS/CVS, this may be preferred
#    #$VERSION = do { my @r = (q$Revision: 28577 $ =~ /\d+/g); sprintf "%d."."%02d" x $#r, @r }; # must be all one line, for MakeMaker
#
#                        #
#
#
#    @ISA         = qw(Exporter);
#    @EXPORT      = qw(
#                        &FmtX1EnvironmentalInfo
#
#                        
#
#                      );
#    #%EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],
#
#    # your exported package globals go here,
#    # as well as any optionally exported functions
#    #@EXPORT_OK   = qw($Var1 %Hashit &func3);
#
#    # TestLibs::Logging::logVersion(__PACKAGE__, q$Revision: 28577 $);
#}
#    our @EXPORT_OK;
#


# X1_CONFIG_ACCOUNT
use constant X1_CONFIG_ACCOUNT =>
           "a128       # account.pdiskLocBitmap
            C          # account.clusters
            L          # account.pdiskQuota
            A8         # account.name
            A24        # account.description
            A8         # account.password
            C          # account.r5StripeSize
            C          # account.r10StripeSize
            C          # account.r5Parity
            C          # account.def_raidtype
            S          # account.threshold
            S          # account.raidsPerCreate
            C          # account.flags
            C          # account.reserved
            C";        # account.minPD


##############################################################################
# Name:     X1SetAccount
#
# Desc:     Setup an X1 "account".
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
sub X1SetAccount
{
    my ($self,
        $locations,
        $r5stripe,
        $r5parity,
        $r10stripe,
        $dft_raidtype,
        $maxraids,
        $threshold,
        $flags,
        $minPD) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['a'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['i'],
                ['i'],
                ['d', 0, 0xFF],
                ['d', 0, 0xFF],
                ["X1SetAccount"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $i;
    my @locs = @$locations;
    my @bits;
    my $bit;

    for ($i = 0; $i < scalar(@locs); $i++)
    {
        my $loc = uc($locs[$i]);
        my $bay = ord(substr($loc, 0, 1)) - ord("A");
        my $slot = int(substr($loc, 1));
        $bit = ($bay * 32) + $slot;

        push @bits, $bit;
    }

    my $locationMap = BuildBitmap(1024, \@bits);

    if (defined($locationMap))
    {
        my $data = pack "
            C               # accountIndex
            " . 
            X1_CONFIG_ACCOUNT, 

            0,              # accountIndex
            $locationMap,   # account.pdiskLocBitmap
            0,              # account.clusters
            0,              # account.pdiskQuota (L,H)
            "",             # account.name
            "",             # account.description
            "",             # account.password
            $r5stripe,      # account.r5StripeSize
            $r10stripe,     # account.r10StripeSize
            $r5parity,      # account.r5Parity
            $dft_raidtype,  # account.def_raidtype
            $threshold,     # account.threshold
            $maxraids,      # account.raidsPerCreate
            $flags,         # account.flags
            0,              # account.reserved
            $minPD;         # account.minPD

        $data = _CfgAccountEncrypt($self->{MASTER_CRYPT}, $data);

        my $packet = assembleX1Packet(X1PKT_ACCT_SET, $data);

        %rsp = $self->_handleX1SyncResponse(
                $packet,
                X1RPKT_ACCT_SET,
                \&_handleResponseX1SetAccount);
    }

    if (%rsp)
    {
        logMsg("X1SetAccount successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1SetAccount failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetAccount
#
# Desc:     Get an X1 "account".
#
# Input:    index
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#           (lots...)
#
##############################################################################
sub X1GetAccount
{
    my ($self,
        $index) = @_;
        
    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF],
                ["X1GetAccount"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack "S", $index;

    my $packet = assembleX1Packet(X1PKT_ACCT_GET, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_ACCT_GET,
            \&_handleResponseX1GetAccount);

    if (%rsp)
    {
        logMsg("X1GetAccount successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetAccount failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetConfig
#
# Desc:     Get X1 Config info
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
sub X1GetConfig
{
    my ($self) = @_;
        
    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_CONFIG, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_CONFIG,
            \&_handleResponseX1GetConfig);

    if (%rsp)
    {
        logMsg("X1GetConfig successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetConfig failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1LicenseConfig
#
# Desc:     Set/Get X1 License Config info
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
use constant X1_LICENSE_CONFIG =>
           "C           # ctrlType
            C           # cpuCount
            C           # beType
            C           # fePortsCount
            C           # bePortsCount
            C19";       # reserved

sub X1LicenseConfig
{
    my ($self, $ctrlType, $cpuCount, $beType, $fePortsCount, $bePortsCount) = @_;
        
    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF], # ctrlType
                ['d', 0, 0xFF], # cpuCount
                ['d', 0, 0xFF], # beType
                ['d', 0, 0xFF], # fePortsCount
                ['d', 0, 0xFF], # bePortsCount
                ["X1LicenseConfig"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack X1_LICENSE_CONFIG, 
                $ctrlType, $cpuCount, $beType, $fePortsCount, $bePortsCount;

    my $packet = assembleX1Packet(X1PKT_LICENSE_CONFIG, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_LICENSE_CONFIG,
            \&_handleResponseX1LicenseConfig);

    if (%rsp)
    {
        logMsg("X1LicenseConfig successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1LicenseConfig failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetElecSig
#
# Desc:     Get X1 Electronic Signature info
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
sub X1GetElecSig
{
    my ($self) = @_;
        
    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_ELEC_SIG_INFO, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_ELEC_SIG_INFO,
            \&_handleResponseX1GetElecSig);

    if (%rsp)
    {
        logMsg("X1GetElecSig successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetElecSig failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     X1GetEnvironmentalInfo
#
# Desc:     Get X1 Environmental info
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
sub X1GetEnvironmentalInfo
{
    my ($self) = @_;
        
    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_ENVIRON, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_ENVIRON,
            \&_handleResponseX1GetEnviro);

    if (%rsp)
    {
        logMsg("X1GetEnvironmentalInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetEnvironmentalInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetBELoopInfo
#
# Desc:     Get X1 Back End Loop info
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
sub X1GetBELoopInfo
{
    my ($self) = @_;
        
    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_BE_LOOP_INFO, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_BE_LOOP_INFO,
            \&_handleResponseX1GetBELoopInfo);

    if (%rsp)
    {
        logMsg("X1GetBELoopInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetBELoopInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     _handleResponseX1SetAccount
#
# Desc:     Handle an X1 "account" setup return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       STATUS   Status of the command
#
##############################################################################
sub _handleResponseX1SetAccount
{
    my ($self,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    my $status = unpack "C", $parts{DATA};
    if ($status) {
        $info{STATUS} = PI_GOOD;
    }
    else {
        $info{STATUS} = PI_ERROR;
    }

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetAccount
#
# Desc:     Handle an X1 get "account" return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       (many...)
#
##############################################################################
sub _handleResponseX1GetAccount
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    my $reserved; 

    ( $info{PDISKBITMAP},
      $info{CLUSTERS},
      $info{PDISKQUOTA},
      $info{NAME},
      $info{DESCRIPTION},
      $info{PASSWORD},
      $info{R5STRIPESIZE},
      $info{R10STRIPESIZE},
      $info{R5PARITY},
      $info{DEFRAIDTYPE},
      $info{THRESHOLD},
      $info{RAIDSPERCREATE},
      $info{FLAGS},
      $reserved,
      $info{MINPD},
      $reserved ) = unpack X1_CONFIG_ACCOUNT, $parts{DATA};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetConfig
#
# Desc:     Handle an X1 get config return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       (many...)
#
##############################################################################

# X1_CONFIG
use constant X1_CONFIG =>
           "C           # numExpCabinets
            L           # sysSerialNum
            L           # mmcVersion
            L           # procVersion
            L           # habVersion
            L           # biosVersion
            L           # compatibility
            L";         # nextEvent

use constant X1_FW_VER =>
           "L           # Major-minor version
            L           # Sub-version
            C           # compatIndex
            C           # backLevelIndex
            a2          # reserved
            A8";        # tag

sub _handleResponseX1GetConfig
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    my $reserved; 

    # Load main part of X1_CONFIG into hash
    ( $info{NUMEXPCABS},
      $info{SYSTEMSN},
      $info{MMCVER},
      $info{PROCVER},
      $info{HABVER},
      $info{BIOSVER},
      $info{COMPAT},
      $info{NEXTEVENT}) = unpack X1_CONFIG, $parts{DATA};
      
      $info{RMCCOMPAT} = (($info{COMPAT} & 0x000000FF) >> 0);
      $info{CONTROLLER_TYPE} = (($info{COMPAT} & 0x0000F000) >> 12);

    my @fwvers;
    
    # Load the struct for each firmware version entry into the hash
    for (my $i = 0; $i < 13; $i++)
    {
        my $start = 29 + (20 * $i);

        (
        $fwvers[$i]{VERSIONMAJORMINOR},
        $fwvers[$i]{VERSIONSUB},
        $fwvers[$i]{COMPAT_IDX},
        $fwvers[$i]{BACKLVL_IDX},
        $reserved,
        $fwvers[$i]{TAG}
        ) = unpack(X1_FW_VER, substr($parts{DATA}, $start));
    }

    $info{FWVERS} = [@fwvers];
        
    return %info;
}

##############################################################################
# Name:     _handleResponseX1LicenseConfig
#
# Desc:     Handle an X1 license config return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       (many...)
#
##############################################################################

sub _handleResponseX1LicenseConfig
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    # Load into hash
    ( $info{CTRLTYPE},
      $info{CPUCOUNT},
      $info{BETYPE},
      $info{FEPORTSCOUNT},
      $info{BEPORTSCOUNT}) = unpack X1_LICENSE_CONFIG, $parts{DATA};
      
    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetBELoopInfo
#
# Desc:     Handle an X1 Get BE Loop Info return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1GetBELoopInfo
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    my @ports;

    my $start;
    
    # Get the Loop Info for each port
    for (my $i = 0; $i < 4; $i++)
    {
        $start = $i * 32;

        # First get the localPortState
        (
        $ports[$i]{STATE}
        ) = unpack("S", substr($parts{DATA}, $start));

        # printf "port %d  state %04X ", $i, $ports[$i]{STATE}; 

        my @remotePort;

        # Now get the remotePortId array
        for (my $j = 0; $j < 15; $j++)
        {
            $start = $start + 2;

            (
            $remotePort[$j]{ID}
            ) = unpack("S", substr($parts{DATA}, $start));

            # printf "   %04X ", $remotePort[$j]{ID}; 
        }

        printf "\n";

        # Put the remotePortId array into the port info for this port
        $ports[$i]{REMOTEPORTID} = [@remotePort];
    }

    $info{BELOOPINFO} = [@ports];

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetElecSig
#
# Desc:     Handle an X1 Get Electronic Signatures return packet.
#           The XCI_DATA struct is 64 bytes.  For now it is simply
#           handled as a string
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################

# X1_ELEC_SIG
use constant X1_ELEC_SIG =>
           "a64         # CCB
            a64         # Memory
            a64         # Chassis
            a64         # Proc
            a64         # FE Power Supply Asm
            a64         # FE Power Supply IF
            a64         # FE Buffer Board
            a64         # BE Power Supply Asm
            a64         # BE Power Supply IF
            a64";       # BE Buffer Board

sub _handleResponseX1GetElecSig
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    my $reserved; 

    # Load main part of X1_ELEC_SIG into hash
    ( $info{CCB},
      $info{MEMORY},
      $info{CHASSIS},
      $info{PROC},
      $info{FEPSASM},
      $info{FEPSIF},
      $info{FEBUF},
      $info{BEPSASM},
      $info{BEPSIF},
      $info{BEBUF}) = unpack X1_ELEC_SIG, $parts{DATA};

    return %info;
}


##############################################################################
# Name:     _handleResponseX1GetEnviro
#
# Desc:     Handle an X1 Get Environmental Info return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1GetEnviro
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %parts = disassembleX1Packet($recvPacket);

    # Call another function to extract the data into a hash.
    return (_parseX1GetEnviro(%parts));
}

##############################################################################
# Name:     _parseX1GetEnviro
#
# Desc:     Handle an X1 Get Environmental Info return packet.
#           This is a common function to handle the data from both X1 and PI
#           packets.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _parseX1GetEnviro
{
# X1_ENVIRO_INFO
use constant X1_ENVIRO_INFO_1 =>
           "C                   # Controller temperature - Host
            C                   # Controller temperature - Store
            C                   # Controller AC 1
            C                   # Controller AC 2
            C                   # Controller DC 1
            C                   # Controller DC 2
            C                   # Controller Fan 1
            C                   # Controller Fan 2
            C                   # Controller Buffer Host
            C                   # Controller Buffer Store
            a2";                # Fibre Bay Map

use constant X1_ENVIRO_INFO_2 =>
           "L                   # Server MB/sec
            L                   # Server IO/sec
            S                   # RESERVED
            L                   # Back-end proc heartbeat
            L                   # Front-end proc heartbeat
            a2";                # SATA Bay Map
            
    my (%parts) = @_;

    my %info;
    my $i;
    my $reserved; 
    my $start = 0; 
    
    # Load first part of X1_ENVIRO_INFO into the hash
    ( $info{CTRLTEMPHOST},
      $info{CTRLTEMPSTORE},
      $info{CTRLAC1},
      $info{CTRLAC2},
      $info{CTRLDC1},
      $info{CTRLDC2},
      $info{CTRLFAN1},
      $info{CTRLFAN2},
      $info{CTRLBUFHOST},
      $info{CTRLBUFSTORE},
      $info{FIBREBAYMAP}) = unpack X1_ENVIRO_INFO_1, $parts{DATA};

    # Move start past the data just unpacked
    $start = 12; 

    my @fibreBayTempIn1;

    # Get the array of Fibre Bay Input 1 temperatures
    for ($i = 0; $i < 16; $i++)
    {
        (
        $fibreBayTempIn1[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the bayTempIn1 array into the info hash
    $info{FIBREBAYTEMPIN1} = [@fibreBayTempIn1];

    my @fibreBayTempIn2;
    # Get the array of Fibre Bay Input 2 temperatures
    for ($i = 0; $i < 16; $i++)
    {
        (
        $fibreBayTempIn2[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the bayTempIn2 array into the info hash
    $info{FIBREBAYTEMPIN2} = [@fibreBayTempIn2];

    my @fibreBayTempOut1;
    # Get the array of Fibre Bay Output 1 temperatures
    for ($i = 0; $i < 16; $i++)
    {
        (
        $fibreBayTempOut1[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the bayTempOut1 array into the info hash
    $info{FIBREBAYTEMPOUT1} = [@fibreBayTempOut1];

    my @fibreBayTempOut2;
    # Get the array of Fibre Bay Output 2 temperatures
    for ($i = 0; $i < 16; $i++)
    {
        (
        $fibreBayTempOut2[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the bayTempOut2 array into the info hash
    $info{FIBREBAYTEMPOUT2} = [@fibreBayTempOut2];

    my @fibreBayPSFan;
    # Get the array of Fibre Bay Power Supply Fans
    for ($i = 0; $i < 16; $i++)
    {
        (
        $fibreBayPSFan[$i]
        ) = unpack("x$start C", $parts{DATA});
        $start = $start + 1;
    }

    # Put the bayPSFan array into the info hash
    $info{FIBREBAYPSFAN} = [@fibreBayPSFan];


    # Load last part of X1_ENVIRO_INFO into the hash
    ( $info{SERVERMB},
      $info{SERVERIO},
      $reserved,
      $info{BEPROCHB},
      $info{FEPROCHB},
      $info{SATABAYMAP}) = unpack("x$start LL S LL a2", $parts{DATA});


    #
    # Additions for SATA bays
    #
    my @sataBayTempOut1;
    $start = $start + 20;   # Accounts for the unpack above 
    
    # Get the array of SATA Bay Output 1 temperatures
    for ($i = 0; $i < 16; $i++)
    {
        (
        $sataBayTempOut1[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the sataBayTempOut1 array into the info hash
    $info{SATABAYTEMPOUT1} = [@sataBayTempOut1];

    my @sataBayTempOut2;
    # Get the array of SATA Bay Output 2 temperatures
    for ($i = 0; $i < 16; $i++)
    {
        (
        $sataBayTempOut2[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the sataBayTempOut2 array into the info hash
    $info{SATABAYTEMPOUT2} = [@sataBayTempOut2];

    my @sataBayPS;
    # Get the array of SATA Bay Power Supplies 
    for ($i = 0; $i < 16; $i++)
    {
        (
        $sataBayPS[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the sataBayPS array into the info hash
    $info{SATABAYPS} = [@sataBayPS];

    my @sataBayFan;
    # Get the array of SATA Bay Fans 
    for ($i = 0; $i < 16; $i++)
    {
        (
        $sataBayFan[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the sataBayfan array into the info hash
    $info{SATABAYFAN} = [@sataBayFan];

    return %info;
}


##############################################################################
# Name:     DisplayX1ElecSig
#
# Desc:     Display the X1 electronic Signature Info
#
# Input:    hash containing X1 account data
#
##############################################################################
sub DisplayX1ElecSig
{
    my (%info) = @_;

    print  "X1 Electronic Signature Info:\n";
    printf "  CCB:                  [%s]\n", _fixString($info{CCB});
    printf "  Memory:               [%s]\n", _fixString($info{MEMORY});
    printf "  Chassis:              [%s]\n", _fixString($info{CHASSIS});
    printf "  FE power suply asm:   [%s]\n", _fixString($info{FEPSASM});
    printf "  FE power supply IF:   [%s]\n", _fixString($info{FEPSIF});
    printf "  FE buffer booard:     [%s]\n", _fixString($info{FEBUF});
    printf "  BE power suply asm:   [%s]\n", _fixString($info{BEPSASM});
    printf "  BE power supply IF:   [%s]\n", _fixString($info{BEPSIF});
    printf "  BE buffer board:      [%s]\n", _fixString($info{BEBUF});

}


##############################################################################
# Name:     DisplayX1EnvironmentalInfo
#
# Desc:     Display the X1 Controller and Disk Bay Environmental Info
#
# Input:    hash containing X1 account data
#
##############################################################################
sub DisplayX1EnvironmentalInfo
{
    my (%info) = @_;

    my $msg;

    FmtX1EnvironmentalInfo(undef, \$msg, %info);

    printf "%s", $msg;

}

##############################################################################
# Name:     FmtX1EnvironmentalInfo
#
# Desc:     Format X1 Controller and Disk Bay Environmental Info
#
# Input:    hash containing X1 account data
#
##############################################################################
sub FmtX1EnvironmentalInfo
{
    my ($self, $msgPtr, %info ) = @_;

    my @fibreBays = ParseBitmap($info{FIBREBAYMAP});
    my @sataBays =  ParseBitmap($info{SATABAYMAP});

    my $msg;

    $msg = sprintf("X1 Environmental Info:\n");
    $msg .= sprintf("  Controller temperature - Host:      %d\n", $info{CTRLTEMPHOST});
    $msg .= sprintf("  Controller temperature - Store:     %d\n", $info{CTRLTEMPSTORE});
    $msg .= sprintf("  Controller AC 1:                  0x%02X\n", $info{CTRLAC1});
    $msg .= sprintf("  Controller AC 2:                  0x%02X\n", $info{CTRLAC2});
    $msg .= sprintf("  Controller DC 1:                  0x%02X\n", $info{CTRLDC1});
    $msg .= sprintf("  Controller DC 2:                  0x%02X\n", $info{CTRLDC2});
    $msg .= sprintf("  Controller Fan 1:                 0x%02X\n", $info{CTRLFAN1});
    $msg .= sprintf("  Controller Fan 2:                 0x%02X\n", $info{CTRLFAN2});
    $msg .= sprintf("  Controller buffer - Host:         0x%02X\n", $info{CTRLBUFHOST});
    $msg .= sprintf("  Controller buffer - Store:        0x%02X\n", $info{CTRLBUFSTORE});
    $msg .= sprintf("  Fibre Bay Map:                    @fibreBays\n");


    # Array of Fibre Bay Input 1 temperatures
    
    $msg .= sprintf("\n  Bay number:                                  ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("%3d  ", $i);
    }

    $msg .= sprintf("\n                                               ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("---  ");
    }

    $msg .= sprintf("\n  Fibre Bay Input 1 temps (bay0 - bay15):      ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("%3d  ", $info{FIBREBAYTEMPIN1}[$i]);
    }

    # Print the array of Fibre Bay Input 2 temperatures
    $msg .= sprintf("\n  Fibre Bay Input 2 temps (bay0 - bay15):      ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("%3d  ", $info{FIBREBAYTEMPIN2}[$i]);
    }

    # Print the array of Fibre Bay Output 1 temperatures
    $msg .= sprintf("\n  Fibre Bay Output 1 temps (bay0 - bay15):     ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("%3d  ", $info{FIBREBAYTEMPOUT1}[$i]);
    }

    # Print the array of Fibre Bay Output 2 temperatures
    $msg .= sprintf("\n  Fibre Bay Output 2 temps (bay0 - bay15):     ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("%3d  ", $info{FIBREBAYTEMPOUT2}[$i]);
    }

    # Print the array of Fibre Bay Power Supply Fan values 
    $msg .= sprintf( "\n  Fibre Bay Power Supply Fans (bay0 - bay15): ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("0x%02X ", $info{FIBREBAYPSFAN}[$i]);
    }

    $msg .= sprintf("\n  Server MB/Sec:                    %d\n", $info{SERVERMB});
    $msg .= sprintf("  Server IO/Sec:                    %d\n", $info{SERVERIO});
    $msg .= sprintf("  Back-end processor heartbeat:     %d\n", $info{BEPROCHB});
    $msg .= sprintf("  Front-end processor heartbeat:    %d\n\n", $info{FEPROCHB});


    # Changes below for SATA bays
    $msg .= sprintf("  SATA Bay Map:                     @sataBays");

    # Print the array of SATA Bay Output 1 temperatures
    $msg .= sprintf("\n  SATA Bay Output 1 temps (bay0 - bay15):      ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("%3d  ", $info{SATABAYTEMPOUT1}[$i]);
    }

    # Print the array of SATA Bay Output 2 temperatures
    $msg .= sprintf("\n  SATA Bay Output 2 temps (bay0 - bay15):      ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("%3d  ", $info{SATABAYTEMPOUT2}[$i]);
    }

    # Print the array of Fibre Bay Power Supply values 
    $msg .= sprintf( "\n  SATA Bay Power Supply (bay0 - bay15):       ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("0x%02X ", $info{SATABAYPS}[$i]);
    }

    # Print the array of Fibre Bay Power Supply Fan values 
    $msg .= sprintf( "\n  SATA Bay  Fans (bay0 - bay15):              ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("0x%02X ", $info{SATABAYFAN}[$i]);
    }

    $$msgPtr .= $msg;

}



##############################################################################
# Name:     DisplayX1Config
#
# Desc:     Display the contents of an X1 account
#
# Input:    hash containing X1 account data
#
##############################################################################
sub DisplayX1Config
{
    my (%info) = @_;

    my @fwtypes = ( FW_VER_SYSTEM,
                    FW_VER_BE_BOOT,
                    FW_VER_BE_DIAG,
                    FW_VER_BE_RUNTIME,
                    FW_VER_FE_BOOT,
                    FW_VER_FE_DIAG,
                    FW_VER_FE_RUNTIME,
                    FW_VER_CCB_BOOT,
                    FW_VER_CCB_RUNTIME,
                    FW_VER_QL2200_EF,
                    FW_VER_QL2200_EFM,
                    FW_VER_QL2300_EF,
                    FW_VER_QL2300_EFM);

    my @fwnames = ( "SYSTEM",
                    "BE_BOOT",
                    "BE_DIAG",
                    "BE_RUN",
                    "FE_BOOT",
                    "FE_DIAG",
                    "FE_RUN",
                    "CCB_BOOT",
                    "CCB_RUN",
                    "QL2200EF",
                    "QL2200EFM",
                    "QL2300EF",
                    "QL2300EFM");
         
    print  "X1 Config Info:\n";
    printf "  Number of expansion cabinets:   %u\n", $info{NUMEXPCABS};
    printf "  System serial number:           %u\n", $info{SYSTEMSN};
    printf "  MMC version:                    %u\n", $info{MMCVER};
    printf "  Proc version:                   %u\n", $info{PROCVER};
    printf "  HAB version:                    %u\n", $info{HABVER};
    printf "  BIOS version:                   %u\n", $info{BIOSVER};
    printf "  Compatibility:                  0x%08X\n", $info{COMPAT};
    printf "  RMC Compatibility:              %u\n", $info{RMCCOMPAT};
    printf "  Controller Type:                %u\n", $info{CONTROLLER_TYPE};
    printf "  Next log event:                 %u\n\n", $info{NEXTEVENT};
    print  "  Firmware versions:\n\n";
    print  "  FW_TYPE     VER Maj Min    Sub     COMP  BL   TAG \n";
    print  "  ----------------------------------------------------- \n";

    for (my $i = 0; $i < 13; ++$i)
    {
        my $type = $fwtypes[$i];
        my $name = $fwnames[$i];

        printf "  %-9s   0x%08X  0x%08X  %-3d  %-3d  %-8s \n",
            $name,
            $info{FWVERS}[$i]{VERSIONMAJORMINOR},
            $info{FWVERS}[$i]{VERSIONSUB},
            $info{FWVERS}[$i]{COMPAT_IDX},
            $info{FWVERS}[$i]{BACKLVL_IDX},
            $info{FWVERS}[$i]{TAG};
    }
}


##############################################################################
# Name:     DisplayX1LicenseConfig
#
# Desc:     Display the X1 license configuration
#
# Input:    hash containing the license data
#
##############################################################################
sub DisplayX1LicenseConfig
{
    my (%info) = @_;

    print  "X1 License Config Info:\n";
    printf "  Controller Type:  %s\n", $info{CTRLTYPE};
    printf "  CPU Count:        %s\n", $info{CPUCOUNT};
    printf "  BE Type:          %s\n", $info{BETYPE};
    printf "  FE Port Count:    %s\n", $info{FEPORTSCOUNT};
    printf "  BE Port Count:    %s\n", $info{BEPORTSCOUNT};
}

##############################################################################
# Name:     DisplayX1BELoopInfo
#
# Desc:     Display the X1 Back End Loop Info
#
# Input:    hash containing X1 Back End Loop Info
#
##############################################################################
sub DisplayX1BELoopInfo
{
    my (%info) = @_;

    print  "X1 Back End Loop Info:\n\n";

    printf " Port    State       Remote ID  \n";
    printf "------- ----------- ----------- \n";

    for (my $i = 0; $i < 4; $i++)
    {
        printf "  %d      0x%04X", $i, $info{BELOOPINFO}[$i]{STATE};

        # Only display the remote port ID if the local port is installed.
        # STATE==0x8000 indicates no port installed.

        if ($info{BELOOPINFO}[$i]{STATE} == 0x8000)
        {
                printf "      No connection\n";
        }
        else
        {
            for (my $j = 0; $j < 15; $j++)
            {
                if ($j != 0)
                {
                    printf "               ";
                }
                printf "      0x%04X\n", $info{BELOOPINFO}[$i]{REMOTEPORTID}[$j]{ID};
            }
        }
        printf "\n";
    }
}


##############################################################################
# Name:     DisplayX1Account
#
# Desc:     Display the contents of an X1 account
#
# Input:    hash containing X1 account data
#
##############################################################################
sub DisplayX1Account
{
    my (%info) = @_;

    my @pDisks = ParseBitmap($info{PDISKBITMAP});

    print  "pdiskBitmap:    @pDisks\n";
    print  "clusters:       $info{CLUSTERS}\n";
    printf "pdiskQuota:     0x%08X\n", $info{PDISKQUOTA};
    print  "name:           \"$info{NAME}\"\n";
    print  "description:    \"$info{DESCRIPTION}\"\n";
    print  "password:       \"$info{PASSWORD}\"\n";
    print  "r5StripeSize:   $info{R5STRIPESIZE}\n";
    print  "r10StripeSize:  $info{R10STRIPESIZE}\n";
    print  "r5Parity:       $info{R5PARITY}\n";
    print  "def_raidtype:   $info{DEFRAIDTYPE}\n";
    print  "threshold:      $info{THRESHOLD}\n";
    print  "raidsPerCreate: $info{RAIDSPERCREATE}\n";
    print  "flags:          $info{FLAGS}\n";
    print  "minPD:          $info{MINPD}\n";
 
}

##############################################################################
# Name:     _CfgAccountEncrypt
#
# Desc:     "Encrypt" the VDisk Config data 
#
# Input:    the data to encrypt
#
# Returns:  the encrypted data
#
##############################################################################
sub _CfgAccountEncrypt
{
    my ($masterCrypt, $data) = @_;
    
    my ($b1, $b2, $b3, $b4, $theRest) = 
        unpack "C C C C a*", $data;

    my $crypt1 = ($masterCrypt & 0xFF) ^ $b2;
    my $crypt2 = (($masterCrypt >> 8) & 0xFF) ^ $b3;
        
    $data .= pack "CC", $crypt1, $crypt2;

    return $data;
}


##############################################################################
# Name: _fixString
#
# Desc: Remove certain annoying characters and replace with .
#
# In:   string
#
# Out:  String
##############################################################################
sub _fixString
{
    my ($str) = @_;

    ###
    ## The negated list is ' ' through '~' (which is the class of printables)
    ##   is to be replaced with a question mark symbol.
    ###
	$str =~ s/[^ -~]/\./g;

    return $str;
}


1;

##############################################################################
# Change log:
# $Log$
# Revision 1.1  2005/05/04 18:53:54  RysavyR
# Initial revision
#
# Revision 1.20  2005/01/31 15:45:06  SchibillaM
# TBolt00012221: Add support for X1PKT_GET_MIRROR_IO_STATUS.
# Reviewed by Chris.
#
# Revision 1.19  2004/10/19 21:47:17  RysavyR
# TBolt00011525: Add X1 "License Config" packet.
# Also added color for Linux operation.
#
# Revision 1.18  2004/08/12 17:39:15  RysavyR
# TBolt00011024: Add 'Get Config' packet to 2341 connection login so that a
# Bigfoot/Wookiee controller type can be established.
#
# Revision 1.17  2004/07/27 12:13:26  SchibillaM
# TBolt00010893: Add support for X1 environmental packet in PI interface.
# Reviewed by Chris.
#
# Revision 1.16  2004/03/08 14:54:45  SchibillaM
# TBolt00010148: SATA changes - devType in PDisk and Bay Info, new environmental
# info for SATA bays.  Reviewed by Randy.
#
# Revision 1.15  2003/08/05 18:58:57  SchibillaM
# TBolt00008793: Complete GeoPool support in CCB and CCBE.
# Reviewed by Randy.
#
# Revision 1.14  2003/03/13 21:32:57  SchibillaM
# TBolt00000000: Changes so this file works with XTC and CCBE.
#
# Revision 1.13  2003/03/13 14:01:22  SchibillaM
# TBolt00000000: Change construction of X1 Environmental data to allow use
# with XTC test code.
#
# Revision 1.12  2003/03/07 21:42:04  SchibillaM
# TBolt00000000: Add support for X1 Environmental Info packet.
#
# Revision 1.11  2003/02/26 20:41:40  NigburC
# TBolt00007474 - Modified the use of the bitmap in the account packet from
# the X1 packets.  It now contains a bitmap of physical disk locations in a 32
# by 32 map (32 bays by 32 disks per bay).  This change required us to cache
# the drive locations, this is built whenever the physical disks are refreshed.
# Reviewed by Tim Swatosh.
#
# Revision 1.10  2003/02/14 21:10:00  RysavyR
# TBolt00006771:  Added fwBackLevelindex & fwCompatIndex support.
# Rev by TimSw
#
# Revision 1.9  2003/01/24 13:59:46  SchibillaM
# TBolt00006514: Change definition of X1_BE_LOOP_INFO_RSP packet.
# Include information on remote CN ID and remote port.  Reviewed by Chris.
#
# Revision 1.8  2003/01/17 17:38:06  SchibillaM
# TBolt00006514: Add support for X1 BELoop info.  Roll compatibility to 0x0A.
#
# Revision 1.7  2003/01/16 21:44:06  SchibillaM
# TBolt00006514: Add support for X1 Electrical Signatures and version in X1 Config.
#
# Revision 1.6  2003/01/15 13:57:25  SchibillaM
# TBolt00006514: Add firmware version info to the X1 config packet.
# Reviewed by Chris.
#
# Revision 1.5  2002/12/13 22:07:31  RysavyR
# TBolt00003598: Added debug statements to SetAccount to see if the decrypt
# is working correctly or not. We can make the decrypt live once we know it works.
# Rev by TimSw.
#
# Revision 1.4  2002/12/13 15:51:00  NigburC
# TBolt00006462, TBolt00006408 - Modified the create vdisk structure to handle the
# threshold and max raids.  Added handling in the CCBE to support the these
# two new requirements.  Added handling for the X1 create vdisk operation to
# convert the X1 values and pass them to the PROC.
# Reviewed by Jeff Williams.
#
# Revision 1.3  2002/12/11 16:22:43  NigburC
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
# Revision 1.2  2002/10/25 16:04:41  RysavyR
# TBolt00006013: Added X1 config "encryption" support and added intermediate
# VDisk config ACK.  Rev by Mark S.
#
# Revision 1.1  2002/10/16 21:02:42  RysavyR
# TBolt00006136: Added support for X1GETACCOUNT and X1SETACCOUNT.
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
