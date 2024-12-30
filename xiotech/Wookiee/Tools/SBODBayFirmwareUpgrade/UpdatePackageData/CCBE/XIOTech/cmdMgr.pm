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

use IO::Socket;
use IO::Select;

# X1 Modules
use XIOTech::cmX1Account;
use XIOTech::cmX1Logs;
use XIOTech::cmX1Maps;
use XIOTech::cmX1Misc;
use XIOTech::cmX1Server;
use XIOTech::cmX1VDisk;
use XIOTech::cmX1VLink;

use XIOTech::cmPDisk;
use XIOTech::cmDiskBay;
use XIOTech::cmVDisk;
use XIOTech::cmLogs;
use XIOTech::cmStats;
use XIOTech::cmVCG;
use XIOTech::cmServer;
use XIOTech::cmTarget;
use XIOTech::cmVLink;
use XIOTech::cmRaid;
use XIOTech::cmiSCSI;
use XIOTech::errorCodes;
use XIOTech::nvramDump;
use XIOTech::cmSnapshot;
use XIOTech::cmPersistentData;
#ifdef ENGINEERING
use XIOTech::cmEngDebug;
#endif

#Setup inheritance
use XIOTech::initializeable;
@ISA = qw(XIOTech::initializeable);

use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;
use XIOTech::constants;
use XIOTech::error;

use XIOTech::logMgr;
use XIOTech::md5;

use strict;


# X1_SU_ITEM
use constant X1_SU_ITEM =>
           "C       # ID
            NN      # wwn
            a8      # name
            C       # luns
            C";     # type

# X1_LUN_ITEM
use constant X1_LUN_ITEM =>
           "C       # ID
            C       # LUN
            C       # raidType
            S       # vid
            L       # size
            L       # SN
            C       # baseVBlock
            C       # baseVid
            C       # numServers
            C       # numLinks
            a8";    # name



##############################################################################
# Name: new
#
# Desc: Creates a new commandManager
#
# In:   File handle where you want the errors to be reported to
#
#       example:
#
#       my $commandManagerObj = XIOTech::commandManager->new(\*STDOUT);
#
# Returns: A blessed anonymous hash (an object)
#
#
# Note:  We don't actual implement the new here, but in a base class
##############################################################################


##############################################################################
# Name:     login
#
# Desc:     Logs into the tbolt
#
# In:       scalar  $host   host name or IP address
#           scalar  $port   port of SAN box to connect to
#
#
# Returns:  1 on success, else returns 0
#
##############################################################################
sub login
{
    my ($self,
        $host,
        $port) = @_;

    logMsg("login...begin ($host, $port)\n");

    # verify parameters
    my $args = [['i'],
                ['s'],
                ['d', 1, 65535],
                ["login"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    # Save the host and port in our hash
    $self->{HOST}       = $host;
    $self->{PORT}       = $port;

    if($port == PI_X1_PORT)
    {
        return X1probe(@_);
    }

    return BFconnect(@_);
}

##############################################################################
# Name:     BFconnect
#
# Desc:     Finishes up the BF port "login" and issues a connect packet.
#
# In:       scalar  $host   host name or IP address
#           scalar  $port   port of SAN box to connect to
#
# NOTE:     It is intended that this subroutine be called only from 'login'
#           above, since no parameter verification is being done here etc.
#
# Returns:  1 on success, else returns 0
#
##############################################################################
sub BFconnect
{
    my ($self,
        $host,
        $port) = @_;

    logMsg("connect...begin ($host, $port)\n");

    my $rc = 0;
    my $socket;
    my %rsp;

    # Initialize more of the command managers fields.
    $self->{'unpack'} = \&disassembleXiotechHeader;
    $self->{'last_error_num'} = NO_ERROR;
    $self->{'header_size'} = PACKET_HEADER_SIZE_3100;
    $self->{TIMEOUT} = DEFAULT_TIMEOUT;

    # get a send socket (uses the PORT in our hash)
    $socket = $self->_get_socket("send");

    if (!defined($socket))
    {
        printf ("Unable to get send socket. %2u ",$rc);
        logMsg("Unable to get send socket.\n");
        return $rc;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $packet;

    $packet = assembleXiotechPacket(PI_CONNECT_CMD,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, COMPAT_INDEX_3);

    %rsp = $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print("connect returned PI_GOOD\n");
            $self->{CONTROLLER_TYPE} = $rsp{CONTROLLER_TYPE};
            $rc = 1;
        }
        else
        {
            print("connect DID NOT return PI_GOOD\n");
            $rc = 0;
        }
    }
    else
    {
        $rc = 0;
    }

    # If the connection was made to the controller we want to set the default
    # timeoout on the PacketInterface higher.
    if ($rc == 1)
    {
        logMsg("Setting the packet interface timeout to 1800 secs...\n");

        %rsp = $self->timeoutMRP("CCB", 1800);

        if (!%rsp || $rsp{STATUS} != PI_GOOD)
        {
            logMsg("Failed to set the packet interface timeout (1800 secs).\n");
        }

        logMsg("Getting and saving the serial number information...\n");

        %rsp = $self->serialNumGet();

        if (!%rsp || $rsp{STATUS} != PI_GOOD)
        {
            logMsg("Failed to get the serial number information.\n");
            $self->{SERIAL_NUM} = 0;
        }
        else
        {
            $self->{SERIAL_NUM} = $rsp{1}{SERIAL_NUM};
        }
    }

    if ($rc == 0)
    {
        my %error = $self->getLastError();
        logMsg("Error while sending connect information, \n");
               #. "reason = $error{string}\n");
        logMsg("connect::Unable to connect\n");
    }

    logMsg("connect...exit\n");
    return $rc;
}

##############################################################################
# Name:     X1probe
#
# Desc:     Finishes up the X1 port "login" and issues a probe packet.
#
# In:       scalar  $host   host name or IP address
#           scalar  $port   port of SAN box to connect to
#
# NOTE:     It is intended that this subroutine be called only from 'login'
#           above, since no parameter verification is being done here etc.
#
# Returns:  1 on success, else returns 0
#
##############################################################################
sub X1probe
{
    my ($self,
        $host,
        $port) = @_;

    logMsg("X1probe...begin ($host, $port)\n");

    my $rc = 0;
    my $socket;
    my %rsp;

    # Initialize more of the command managers fields.
    $self->{'unpack'} = \&disassembleX1Header;
    $self->{'last_error_num'} = NO_ERROR;
    $self->{'header_size'} = PACKET_HEADER_SIZE_X1;
    $self->{TIMEOUT} = DEFAULT_TIMEOUT;

    # get a send socket (uses the PORT in our hash)
    $socket = $self->_get_socket("send");

    if (!defined($socket))
    {
        logMsg("Unable to get send socket.\n");
        return $rc;
    }

    my $packet;
    my $data = pack "CCC", 0xFE, 0, 0; # set RMC_COMPAT very high for now,
                                       # major ver, minor ver

    print "Sending the probe packet...\n";
    $packet = assembleX1Packet(X1PKT_PROBE, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_PROBE,
            \&_handleResponseX1Probe);

    if (%rsp)
    {
        logMsg("X1probe successful\n");
        $self->{SERIAL_NUM} = $rsp{SERIAL_NUM};
        $rc = 1;
    }
    else
    {
        $rc = 0;
    }

    # Now send the secure login packet
    if ($rc == 1)
    {
        print "Sending the secure login packet...\n";

        %rsp = $self->X1SecureLogin();

        if (!%rsp || $rsp{STATUS} != PI_GOOD)
        {
            logMsg("Secure Login Failed!\n");
            $rc = 0;
        }
    }

    # After that, send the get config packet
    if ($rc == 1)
    {
        print "Sending the get config packet...\n";

        %rsp = $self->X1GetConfig();

        if (!%rsp || $rsp{STATUS} != PI_GOOD)
        {
            logMsg("Get Config Failed!\n");
            $rc = 0;
        }
        else
        {
            $self->{CONTROLLER_TYPE} = $rsp{CONTROLLER_TYPE};
        }
    }

    # If the connection was made to the controller we want to set the default
    # timeout on the PacketInterface higher.
    if ($rc == 1)
    {
        logMsg("Setting the packet interface timeout to 1800 secs...\n");

        %rsp = $self->timeoutMRP("CCB", 1800);

        if (!%rsp || $rsp{STATUS} != PI_GOOD)
        {
            logMsg("Failed to set the packet interface timeout (1800 secs).\n");
            $rc = 0;
        }
    }

    if ($rc == 0)
    {
        my %error = $self->getLastError();
        logMsg("Error while sending connect information, \n");
               #. "reason = $error{string}\n");
        logMsg("X1probe::Unable to connect\n");
    }

    logMsg("X1probe...exit\n");
    return $rc;
}

##############################################################################
# Name:     X1SecureLogin
#
# Desc:     Exchange keys to access configuration features
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
sub X1SecureLogin
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["X1SecureLogin"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;

    my $myTime = time();
    my $crypt = $myTime ^ ((0x532fffff - $self->{SERIAL_NUM}) & 0xffffffff) ^ 0x12435678;

    my $data = pack "LL", $myTime, $crypt;

    my $packet = assembleX1Packet(X1PKT_SECURE_LOGIN, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_SECURE_LOGIN,
            \&_handleResponseX1SecureLogin);

    if (%rsp)
    {
        logMsg("X1 Secure Login successful\n");
        $self->{MASTER_CRYPT} = $rsp{MASTER_CRYPT};
        $rc = 1;
    }
    else
    {
        logMsg("X1 Secure Login failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     logout
#
# Desc:     logs out of the san box, thus allowing you to log into
#           another san box
#
# In:       none
#
#
# Returns:  1 on success, else returns 0
#
##############################################################################
sub logout
{
    my ($self) = @_;

    # verify parameters
    my $args = [['i'],
                ["logout"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    if (defined($self->{'socket'}))
    {
        $self->{'socket'}->close();
        delete $self->{'socket'};
    }
}

##############################################################################
# Name:     sendFirmware
#
# Desc:     Send a firmware image file to the CCB.
#
# In:       filename
#
# Returns:  1 if successful, 0 if failure
#
##############################################################################
sub sendFirmware
{
    my ($self, $filename) = @_;
    return sendFirmwareCommon($self, $filename, "FLASH");
}

sub tryFirmware
{
#ifdef ENGINEERING
    my ($self, $filename) = @_;
    return sendFirmwareCommon($self, $filename, "DRAM");
#endif
}

sub mpxSendFirmware
{
    my ($self, $filename) = @_;
    return mpxSendFirmwareCommon($self, $filename, "FLASH");
}

sub mpxTryFirmware
{
#ifdef ENGINEERING
    my ($self, $filename) = @_;
    return mpxSendFirmwareCommon($self, $filename, "DRAM");
#endif
}

# type is either "FLASH" or "DRAM"
sub sendFirmwareCommon
{
    my ($self, $filename, $type) = @_;
    my %errorHash;
    $errorHash{STATUS}     = PI_ERROR;
    $errorHash{ERROR_CODE} = 0;

    logMsg("sendFirmware...begin\n");

    if ($self->{PORT} == PI_X1_PORT) {
        print "Firmware Update not supported on X1 port.\n";
        return %errorHash;
    }

    # verify parameters
    my $args = [['i'],
    ['s'],
    ["SendFirmware"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    # read up the file in binary mode
    if( (open FW, "$filename") &&
        (binmode FW) )
    {
        my $buffer;

        read FW, $buffer, -s $filename;
        close FW;

        my $seq = $self->{SEQ}->nextId();
        my $ts = $self->{SEQ}->nextTimeStamp();
        my $cmd = PI_FIRMWARE_DOWNLOAD_CMD;
        if ($type eq "DRAM") {
            $cmd = PI_TRY_CCB_FW_CMD;
        }

        my $packet = assembleXiotechPacket($cmd,
                                            $seq,
                                            $ts,
                                            $buffer,
                                            $self->{PORT}, VERSION_1);

        return $self->_handleSyncResponse($seq,
                                          $packet,
                                          \&_genericResponsePacket);
    }

    print "ERROR: Unable to open firmware file $filename\n";
    return %errorHash;
}

# Sub-command codes
use constant MPX_FW_SCMD        => 1;
use constant MPX_FILEIO_SCMD    => 2;
use constant MPX_MEMIO_SCMD     => 3;

# Flags
use constant MPX_WRITE       => 0x01;
use constant MPX_PROC_MASK   => 0x06; # This is a 2 bit field used with MEMIO:
                                      # 0 => CCB
                                      # 1 => FE
                                      # 2 => BE
use constant MPX_WRITE_NO_HDR => 0x08; # No header on file write

# Others constants
use constant MPX_MAX_TX_DATA_SIZE => 0xF800;  # 64K - 2048 -> need to allow
                                              # for header and misc stuff.
# type is either "FLASH" or "DRAM"
sub mpxSendFirmwareCommon
{
    my ($self, $filename, $type) = @_;

    my $rc = 1;

    logMsg("mpxSendFirmware...begin\n");

    # verify parameters
    my $args = [['i'],
    ['s'],
    ["mpxSendFirmware"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    # read up the file in binary mode
    my $buffer;
    $rc = open FW, "$filename";
    binmode FW;
    read FW, $buffer, -s $filename;
    close FW;


    my $fLen = length($buffer);
    my $x;
    my $n = int(($fLen + MPX_MAX_TX_DATA_SIZE - 1) / MPX_MAX_TX_DATA_SIZE);
    my %hash;

    for($x = 1; $x <= $n; $x++) {

        my $data = pack("CCCCLL", MPX_FW_SCMD,  # subCmdCode;
                                  $x,           # partX
                                  $n,           # ofN
                                  MPX_WRITE,    # flags
                                  $type eq "DRAM" ? 1 : 0, # parm1
                                  0);           # parm2

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
            printf "%2u/$n) OK\r", $x;
        }
        else {
            printf "%2u/$n) BAD\r", $x;
            last;
        }
    }
    print "\n\n";
    return %hash;
}

##############################################################################
# Name:     genericMRP
#
# Desc:     Retieves and parses a generic command
#
# Input:    Binary data representing the MRP input packet
##############################################################################
sub genericMRP
{
    my ($self, $mrpCmd, $rspDataLn, $data) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args;
    if(defined $data) {
        $args = [['i'],
        ['d', 1, 65535],
        ['d', 1, 0x100000],
        ['i'],
        ["genericMRP"]];
    }
    else {
        $args = [['i'],
        ['d', 1, 65535],
        ['d', 1, 0x100000],
        ["genericMRP"]];
    }

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $dataPkt;
    if(defined $data) {
        $dataPkt = pack("LLL", $mrpCmd, length($data), $rspDataLn);
        $dataPkt .= $data; # tack on the data payload
    }
    else {
        $dataPkt = pack("LLL", $mrpCmd, 0, $rspDataLn);
    }

    my $packet = assembleXiotechPacket(PI_GENERIC_MRP_CMD,
                                        $seq,
                                        $ts,
                                        $dataPkt,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_genericMRPResponsePacket);
}

##############################################################################
# Name:     genericCommand
#
# Desc:     Retieves and parses a generic command
#
# Input:    Which command
##############################################################################
sub genericCommand
{
    my ($self, $cmd, @parms) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['s'],
                ['i'],
                ["genericCommand"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmdCode;
    $cmdCode = PI_GENERIC_FUNCTION_CALL     if $cmd =~ /^FUNCTION$/i;
    $cmdCode = PI_GENERIC_RESET             if $cmd =~ /^RESET$/i;
    $cmdCode = PI_GENERIC_DEBUG_ADDRESS     if $cmd =~ /^DEBUGADDR$/i;
    $cmdCode = PI_GENERIC_DO_ELECTION       if $cmd =~ /^ELECTION$/i;
    $cmdCode = PI_GENERIC_FAILURE_MANAGER   if $cmd =~ /^FAILURE$/i;
    $cmdCode = PI_GENERIC_ERROR_TRAP        if $cmd =~ /^ERROR_TRAP$/i;
    $cmdCode = PI_GENERIC_SET_PDISK_LED     if $cmd =~ /^SET_LED$/i;
    $cmdCode = PI_GENERIC_SEND_LOG_EVENT    if $cmd =~ /^LOG_EVENT$/i;
    $cmdCode = PI_GENERIC_CACHE_TEST        if $cmd =~ /^CACHE_TEST$/i;
    $cmdCode = PI_GENERIC_DISASTER_TEST     if $cmd =~ /^DISASTER_TEST$/i;
    $cmdCode = PI_GENERIC_KEEP_ALIVE_TEST   if $cmd =~ /^KEEP_ALIVE_TEST$/i;
    $cmdCode = PI_GENERIC_FIO_MAP_TEST      if $cmd =~ /^FIO_MAP_TEST$/i;
    $cmdCode = PI_GENERIC_FCM_COUNTER_TEST  if $cmd =~ /^FCM_COUNTER_TEST$/i;

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

    return $self->_handleSyncResponse($seq, $packet, \&_genericResponsePacket);
}

##############################################################################
# Name:     generic2Command
#
# Desc:     Retieves and parses a generic2 command (debug report)
#
# Input:    Which command (for a report, one of: HEAP, TRACE, PCB, PROFILE)
##############################################################################
sub generic2Command
{
    my ($self, $cmd) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['s'],
                ["generic2Command"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmdCode;
    $cmdCode = GET_HEAP_STATS    if $cmd =~ /^HEAP$/i;
    $cmdCode = GET_TRACE_STATS   if $cmd =~ /^TRACE$/i;
    $cmdCode = GET_PCB_STATS     if $cmd =~ /^PCB$/i;
    $cmdCode = GET_PROFILE_STATS if $cmd =~ /^PROFILE$/i;
    $cmdCode = GET_PACKET_STATS  if $cmd =~ /^PACKET$/i;


    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LL64L",
                    $cmdCode,
                    0);

    my $packet = assembleXiotechPacket(PI_GENERIC2_CMD,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_genericCmd2ResponsePacket);
}

##############################################################################
# Name:     write_packet
#
# Desc:     writes a packet to $self->{'socket'}
#
# In:       hash    hash    packet parts
#
# Returns:  1 on success, 0 on failure
#
##############################################################################
sub write_packet
{
    my $self = shift;
    my %hash = @_;
    my ($packet, $bytes_written, $rc);

    logMsg("write_packet begin...\n");

    $rc = 1;

    $packet = $hash{'packet'};

    logMsg("writing " . length($packet) . " bytes\n");
    $rc = $self->writeSocketData($self->{'socket'}, $packet, length($packet));

    return $rc;
}

##############################################################################
# Name:     read_packet
#
# Desc:     reads a packet by $self->{'socket'}
#
# Returns:  Reference to a hash of success, or undef on failure
#
##############################################################################
sub read_packet
{
    my ($self) = @_;

    my $packet;
    my $header;
    my $data;

    logMsg("read_packet begin...\n");

    my $size = $self->{'header_size'};
    my $offset = 0;
    my $socket = $self->{'socket'};
    my $reader = IO::Select->new();
    $reader->add($socket);

    # Do we have something to read?
    my ($ready) = IO::Select->select($reader,
                                    undef,
                                    undef,
                                    $self->{TIMEOUT});

    if (!defined($ready))
    {
        $self->{'last_error_num'} = ERR_SOCKET_TIMEOUT;

        logMsg("Timed out reading socket from " .
                "$self->{HOST}:$self->{PORT} with " .
                "timeout = $self->{TIMEOUT}\n");

        return undef;
    }
    else
    {
        # this is the socket with data on it
        $socket = @$ready[0];
    }

    logMsg("Read the header... ($size bytes)\n");
    $header = $self->readSocketData($socket, $size);

    if (!defined($header))
    {
        logMsg("Error reading the header, exiting...\n");
        return undef;
    }

    if ($self->{PORT} != PI_X1_PORT) {
        $header = decrypt($header, $size);

        my $localheader;

        # we have the header, how much data is there?
        $localheader = $self->{'unpack'}->($header, $self->{PORT});
        $offset = 0;
        my $reqSize = $localheader->{DATA_LENGTH};
        my $readSize = $localheader->{PAYLOAD_LENGTH};
        undef $localheader;

        # need to save the original packet
        $packet->{'header'} = $header;

        if ($readSize > 0xFFFFFFFF) # limit packet size
        {
            $self->{'last_error_num'} = ERR_PACKET_TOO_BIG;
            return undef;
        }

        if ($readSize > 0) # get the data packet
        {
            logMsg("Read the data... ($readSize bytes)\n");
            $data = $self->readSocketData($socket, $readSize);

            if (defined($data))
            {
                $data = decrypt($data, $readSize);
                $packet->{'data'} = substr($data, 0, $reqSize);
            }
            else
            {
                logMsg("Error reading the data, exiting...\n");
                return undef;
            }
        }

    }
    # Must be an X1 packet
    else {
        my $localheader = $self->{'unpack'}->($header, $self->{PORT});
        my $readSize = $localheader->{LENGTH};

        if ($readSize > 0) # get the data packet
        {
            if ($readSize > 0xFFFF) # limit packet size
            {
                $self->{'last_error_num'} = ERR_PACKET_TOO_BIG;
                return undef;
            }

            logMsg("Read the data... ($readSize bytes)\n");

            $data = $self->readSocketData($socket, $readSize);

            if (defined($data))
            {
                $packet->{'command'} = unpack "C", $data;
                $data = substr($data, 1);
                if(length($data)) {
                    $packet->{'data'} = $data;
                }
            }
            else
            {
                logMsg("Error reading the data, exiting...\n");
                return undef;
            }
        }
    }

    # Temporary: we need to know the port to properly disassemble the packet
    $packet->{PORT} = $self->{PORT};

    $self->{'packets_read'} += 1;
    return $packet;
}

##############################################################################
# Name:     readSocketData
#
# Desc:     Reads a length of data off the socket.
#
# Returns:  Reference to a data buffer on success, or undef on failure
##############################################################################
sub readSocketData
{
    my ($self, $socket, $len) = @_;

    logMsg("readSocketData...begin (read $len bytes)\n");

    my $data;
    my $bytes_read;
    my $offset = 0;

    while (1)
    {
        $bytes_read = $socket->sysread($data, $len, $offset);

        if (defined($bytes_read))
        {
            $len -= $bytes_read;

            if ($bytes_read == 0)
            {
                $self->{'last_error_num'} = ERR_HOST_DISCONNECTED;
                logMsg("Host disconnected while listening to " .
                        "$self->{'HOST'}:$self->{'PORT'}\n");

                return undef;
            }
            elsif ($len > 0)
            {
                $offset += $bytes_read;
            }
            else
            {
                last;
            }
        }
        else
        {
            $self->{'last_error_num'} = ERR_READING_FROM_SOCKET;
            logMsg("Error while reading from the socket.\n");
            return undef;
        }
    }

    return $data;
}

##############################################################################
# Name:     writeSocketData
#
# Desc:     Writes a length of data to the socket.
#
# Returns:  1 on success, 0 on failure
##############################################################################
sub writeSocketData
{
    my ($self, $socket, $data, $len) = @_;

    logMsg("writeSocketData...begin (write $len bytes)\n");

    my $bytes_written;
    my $offset = 0;
    my $rc = 1;
    my $chunk;
    my $chunkLen;

    while (1)
    {
        # Send only 100K at a time.  If we send more, on some
        # systems, the syswrite() blows chunks.
        $chunkLen = $len>100000 ? 100000 : $len;
        $chunk = substr($data, $offset, $chunkLen);

        $bytes_written = $socket->syswrite($chunk, $chunkLen);

        if (defined($bytes_written))
        {
            logMsg("writeSocketData: $bytes_written bytes written\n");

            $len -= $bytes_written;

            if ($bytes_written == 0)
            {
                $self->{'last_error_num'} = ERR_HOST_DISCONNECTED;
                logMsg("Host disconnected while listening to " .
                        "$self->{'HOST'}:$self->{'PORT'}\n");

                return 0;
            }
            elsif ($len > 0)
            {
                $offset += $bytes_written;
            }
            else
            {
                last;
            }
        }
        else
        {
            logMsg("Error while writing to the socket: $!.\n");
            $self->{'last_error_num'} = ERR_WRITING_TO_SOCKET;
            return 0;
        }
    }

    $self->{'packets_written'} += 1;

    return 1;
}

##############################################################################
# Name:     getLastError
#
# Desc:     Returns a hash for last error encountered
#           error code is cleared after the operation is performed
#
# Returns:  undef if no error to report else returns hash with error number
#           and corresponding error string
##############################################################################
sub getLastError
{
    my $self = shift;

    my ($errorNum, %hash);

    $errorNum = $self->{'last_error_num'};
    $self->{'last_error_num'} = NO_ERROR;

    if ($errorNum != NO_ERROR)
    {
        %hash = ('number' => $errorNum, 'string' => errmsg($errorNum));
    }

    return(%hash);
}

##############################################################################
# Name:     calcRaidParms
#
# Desc:     Based on the raid type passed in and how many disk we have
#           returns the correct parameters to be used in the create vdisk
#           packet
#
# Returns:  Returns a hash with the following information on success,
#           else returns an empty hash
#
#   MIRROR_DEPTH
#   RAID_CLASS
#   PARITY
#   STRIPE_SIZE
#   DISKS           A reference to an array of physical disks id's
#
##############################################################################
sub calcRaidParms
{
    my ($self,
        $physicalDisks,
        $raidType) = @_;

    my @disks = @$physicalDisks;

    my %info;
    my %rc;
    my $errorOccured = 0;

    if (@disks)
    {
        if ($raidType == RAID_NONE ||
            $raidType == RAID_0 ||
            $raidType == RAID_1 ||
            $raidType == RAID_5 ||
            $raidType == RAID_10)
        {
            my $numDisks = scalar(@disks);

            $info{DISKS} = \@disks;
            $info{MIRROR_DEPTH} = DEFAULT_MIRROR_DEPTH;

            # Set mirror to three for all cases even though not used
            # unless we are doing raid 5
            $info{PARITY} = 3;

            if ($raidType == RAID_0)
            {
                # Raid 0 is striping no redundancy

                $info{STRIPE_SIZE} = DEFAULT_STRIPE_SIZE_RAID_0;

                if ($numDisks == 1)
                {
                    $info{RAID_CLASS} = RAID_NONE;
                }
                else
                {
                    $info{RAID_CLASS} = RAID_0;
                }
            }
            elsif ($raidType == RAID_5)
            {
                # Rotating parity raid
                $info{STRIPE_SIZE} = DEFAULT_STRIPE_SIZE_RAID_5;
                $info{RAID_CLASS} = RAID_5;

                if ($numDisks < 3)
                {
                    logMsg("Need at least three disks for raid 5\n");
                    $errorOccured = 1;
                }
                elsif ($numDisks < 5)
                {
                    $info{PARITY} = 3;
                }
                else
                {
                    $info{PARITY} = 5;
                }
            }
            elsif ($raidType == RAID_1)
            {
                # Rotating parity raid
                $info{MIRROR_DEPTH} = $numDisks;
                $info{RAID_CLASS} = RAID_1;
                $info{STRIPE_SIZE} = DEFAULT_STRIPE_SIZE_RAID_10;
                $info{PARITY} = 0;

                if ($numDisks < 2)
                {
                    logMsg("Need at least three disks for raid 5\n");
                    $errorOccured = 1;
                }
            }
            else
            {
                if ($numDisks == 1)
                {
                    $info{RAID_CLASS} = RAID_NONE;
                    $info{STRIPE_SIZE} = 512;
                    $info{MIRROR_DEPTH} = DEFAULT_MIRROR_DEPTH;
                    $info{PARITY} = 0;
                }
                else
                {
                    # Set the stripe size, forget this
                    $info{STRIPE_SIZE} = DEFAULT_STRIPE_SIZE_RAID_10;

                    # Striped mirrors
                    if ($numDisks == 2)
                    {
                        # Can't stripe, but we can mirror
                        $info{RAID_CLASS} = RAID_0;
                    }
                    else
                    {
                        # Striped mirrors, best of all worlds
                        $info{RAID_CLASS} = RAID_10;
                    }
                }
            }
        }
        else
        {
            logMsg("Invalid raid type specified: $raidType\n");
            $errorOccured = 1;
        }
    }
    else
    {
        $errorOccured = 1;
    }

    #Check for errors
    if (!$errorOccured)
    {
        %rc = %info;
    }

    return %rc;
}

##############################################################################
# Name: getOperationalPhysicalDiskIDs
#
# Desc: Returns an array of all the physical disk id's that are operational
#       or an empty array if we have errors
#
# In: None
#
# Returns: An array of physical disk id's , empty array if we encounter errors
#
##############################################################################
sub getOperationalPhysicalDiskIDs
{
    my($self) = @_;

    my @physicalDisk = ();

    my %rsp = $self->physicalDiskList();

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            my $i;
            my $disksFound = 0;

            for $i (0..$#{$rsp{LIST}})
            {
                my $pid = $rsp{LIST}[$i];
                my %info = $self->physicalDiskInfo($pid);

                if (%info)
                {
                    if ($info{STATUS} == PI_GOOD)
                    {
                        if ($info{PD_DEVSTAT} == DEVSTATUS_OPERATIONAL &&
                            $info{PD_CLASS} == LABEL_TYPE_DATA)
                        {
                            $physicalDisk[$disksFound++] = $info{PD_PID};
                        }
                    }
                    else
                    {
                        logMsg("Unable to retrieve physical disk information ($pid).\n");
                    }
                }
                else
                {
                    logMsg("ERROR: Did not receive a response packet.\n");
                }
            }
        }
        else
        {
            logMsg("Unable to retrieve list of physical disk identifiers.");
        }
    }
    else
    {
        logMsg("ERROR: Did not receive a response packet.\n");
    }

    return @physicalDisk;
}

##############################################################################
# Name: rangeToList
#
# Desc: Returns an array identifiers contained in the input range
#       specification.  The range specification is a comma separated
#       list of identifiers and ranges in the form of #-#.
#
# In:   Input range specification which is a comma separated list of
#       identifiers and ranges (#-#).  Example: 1,3,5-10,13,20-50.
#
# Returns: An array of identifiers in the list
#
##############################################################################
sub rangeToList
{
    my ($self, $inputRangeSpec) = @_;

    my $i;
    my $iRange;

    my @list = ();
    my $listCount = 0;

    my @rangeArray;
    my @range;

    @rangeArray = split /,/, $inputRangeSpec;

    for ($i = 0; $i < scalar(@rangeArray); $i++)
    {
        # ISCSI
        if ($rangeArray[$i] !~ /^?\d+$/)
        {
            $list[$listCount] = $rangeArray[$i];
            ++$listCount;
        }
        else
        {
            @range = split /-/, $rangeArray[$i];

            if (scalar(@range) == 1)
            {
                $list[$listCount] = $range[0];
                ++$listCount;
            }
            else
            {
                for ($iRange = $range[0]; $iRange <= $range[1]; $iRange++)
                {
                    $list[$listCount] = $iRange;
                    ++$listCount;
                }
            }
        }
    }

    return @list;
}

##############################################################################
# Name: parseIP
#
# Desc: Returns an array identifiers contained in the input range
#       specification.  The range specification is a comma separated
#       list of identifiers and ranges in the form of #-#.
#
# In:   Input IP in dot notation. Example: 10.64.131.186
#
# Returns: Array of four integers
#
##############################################################################
sub parseIP
{
    my ($self, $inputRangeSpec) = @_;

    my @lst;

    @lst = split (/\./, $inputRangeSpec);

    return @lst;
}

##############################################################################
#
#   All commands that start with a _ should not be used and are considered
#   private.
#
##############################################################################

##############################################################################
# Name:     _processCommandInProgress
#
# Desc:
#
# In:
#
# Out:
##############################################################################
sub _processCommandInProgress
{
    my ($self, $seq) = @_;
    my $rc = 0;

    logMsg("begin ($seq)\n");

    my $packet = $self->_receivePacketSync($seq);

    if (defined($packet))
    {
        my %parts = disassembleXiotechPacket($packet);

        if ($parts{STATUS} == PI_IN_PROGRESS)
        {
            logMsg("We received the PI_IN_PROGRESS.\n");
            $rc = 1;
        }
    }

    logMsg("end\n");

    return $rc;
}

##############################################################################
# Name:     _processCommandComplete
#
# Desc:
#
# In:
#
# Out:
##############################################################################
sub _processCommandComplete
{
    my ($self,
        $seq,
        $expected) = @_;
    my $rc = 0;

    logMsg("begin ($seq, $expected)\n");

    my $packet = $self->_receivePacketSync($seq);

    if (defined($packet))
    {
        my %parts = disassembleXiotechPacket($packet);

        if ($parts{STATUS} == $expected)
        {
            logMsg("We received the expected completion status.\n");
            $rc = 1;
        }
        else
        {
            my $msg = "We DID NOT receive the expected completion " .
                        "status (received: " .
                        $parts{STATUS} .
                        ", expected: " .
                        $expected .
                        ")\n";
            logMsg($msg);
        }
    }

    logMsg("end\n");

    return $rc;
}

##############################################################################
# Name:  _receivePacketSync
#
# Desc: Reads a packet from the SAN box and waits until we get it or a timeout
#       occurs
#
# In:   Sequence number we are requesting
#
# Returns:  undef if we had errors, otherwise a packet that has had it's md5
#           checked
#
##############################################################################
sub _receivePacketSync
{
    my ($self, $seq) = @_;

    logMsg("_receivePacketSync...begin ($seq)\n");

    my $rc = undef;

    my $packet = $self->read_packet();

    if (defined($packet))
    {
        logMsg("validating xiotech packet...\n");

        if ($self->_validateXiotechPacket($packet))
        {
            if (seqNum($packet) != $seq)
            {
                my $msg = "ASSERTION: Mismatched sequence keys (" .
                            seqNum($packet) .
                            ", " .
                            $seq .
                            ")\n";
                logMsg($msg);
            }
            else
            {
                $rc = $packet;
            }
        }
        else
        {
            $self->_handleError($packet);
            logMsg("Packet is invalid.\n");
        }
    }
    else
    {
        my %error = $self->getLastError();
        logMsg("Error while receiving packet, error reason = $error{'string'}\n");
    }

    return $rc;
}

##############################################################################
# Name:  _receiveX1PacketSync
#
# Desc: Reads a packet from the SAN box and waits until we get it or a timeout
#       occurs
#
# In:   Command Response Code we are expecting
#
# Returns:  undef if we had errors, otherwise a packet that has had it's md5
#           checked
#
##############################################################################
sub _receiveX1PacketSync
{
    my ($self, $responseCC, $flush) = @_;

    logMsg("begin (looking for cmd $responseCC)\n");

    my $rc = undef;
    my $count = 0;

    while (1) {
        my $packet;

        # flush input queue
        if (defined($flush))
        {
            my $socket = $self->{'socket'};
            my $reader = IO::Select->new();
            $reader->add($socket);

            # Do we have something to read?
            if (! IO::Select->select($reader, undef, undef, 0))
            {
                last;
            }
        }

        $packet = $self->read_packet();
        $count++;

        if (defined($packet))
        {
            logMsg("checking x1 packet...\n");

            if (($packet->{'command'} != $responseCC) ||
                (($packet->{'command'} == $responseCC) &&
                 ($packet->{'command'} == X1RPKT_LOG_ENTRY) &&
                 (length($packet->{'data'}) <= 52))) {
                logMsg("expected cmd $responseCC, ".
                        "got $packet->{'command'} -- discarding...\n");

                if ($packet->{'command'} == X1RPKT_LOG_ENTRY)
                {
                    my ($evNum, $ts, $type, $etype, $flags, $status) =
                        unpack "LLCCCC", $packet->{'data'};

                    my $sev = "?????";
                    $sev = "(F)" if $flags == 3;
                    $sev = "(E)" if $flags == 2;
                    $sev = "(W)" if $flags == 1;
                    $sev = "(I)" if $flags == 0;

                    my $data = substr $packet->{'data'}, 12;

                    if ($type == 0xB0)
                    {
                        my ($msg) =  unpack "a40", $data;
                        my $etypenum = sprintf ("0x%02X" , $etype);
                        print " !Text Log event: etype: $etypenum, $evNum " .
                              scalar(localtime($ts)) . " $sev $msg\n";
                    }
                    elsif ($type == 0xE1)
                    {
                        my ($types) =  unpack "c", $data;
                        my $data = substr $data, 1;

                        if ($types == 0)
                        {
                            my ($prpStat, $scsiStat, $encl, $slotID, $cdb) =
                                unpack "CCCCC16", $packet->{'data'};
                            print " !Short SCSI Event:  $evNum " . scalar(localtime($ts)) .
                                                                  " $sev \n";
                            print "\t Device $encl$slotID  prpStat = $prpStat scsiStat = $scsiStat \n";

                        }
                        elsif ($types == 1)
                        {
                            my ($prpStat, $scsiStat, $encl, $slotID, $cdb) =
                                unpack "CCCCC32", $packet->{'data'};
                            print " !Long SCSI Event:  $evNum " . scalar(localtime($ts)) .
                                                                  " $sev \n";
                            print "\t Device $encl$slotID  prpStat = $prpStat scsiStat = $scsiStat \n";
                        }
                        else
                        {
                            my ($srcEn, $srcId, $destEn, $destId, $eCode1, $rsvd1,
                                $srcSN, $srcWWN_hi,  $srcWWN_lo,
                                $destWWN_hi,  $dest_WWN_lo, $rsvd2) =
                                unpack "CCCCCa2a12NNNNa4", $data;
                            print " !PDISK Event:  $evNum " . scalar(localtime($ts)) .
                                                                  " $sev \n";
                            printf "    srcEn:          %d\n", $srcEn;
                            printf "    srcEn + 'A':    %c\n", ($srcEn + ord('A'));
                            printf "    srcId:          %d\n", $srcId;
                            printf "    destEn:         %d\n", $destEn;
                            printf "    destEn + 'A':   %c\n", ($destEn + ord('A'));
                            printf "    destId:         %d\n", $destId;
                            printf "    srcSN:          %s\n", $srcSN;
                            printf "    eCode1:         %d\n", $eCode1;
                            printf "    srcWWN:         %8.8X%8.8X\n", $srcWWN_hi, $srcWWN_lo;
                            printf "    destWWN:        %8.8X%8.8X\n", $destWWN_hi, $dest_WWN_lo;
                        }


                    }
                    else
                    {
                        printf( " !UNEXPECTED Log type recieved (0x%02X) \n", $type);
                    }
                }

                elsif ($packet->{'command'} == X1PKT_PCHANGED) {
                    print " !Pdisk changed event received\n";
                }
                elsif ($packet->{'command'} == X1PKT_RCHANGED) {
                    print " !Raid changed event received\n";
                }
                elsif ($packet->{'command'} == X1PKT_VCHANGED) {
                    print " !Vdisk changed event received\n";
                }
                elsif ($packet->{'command'} == X1PKT_HCHANGED) {
                    print " !Hab changed event received\n";
                }
                elsif ($packet->{'command'} == X1PKT_VCG_CHANGED) {
                    print " !VCG changed event received\n";
                }
                elsif ($packet->{'command'} == X1PKT_ACHANGED) {
                    print " !Account changed event received\n";
                }
                elsif ($packet->{'command'} == X1PKT_ZCHANGED) {
                    print " !Zone changed event received\n";
                }
                elsif ($packet->{'command'} == X1RPKT_CONFIG_VDISK_ACK) {
                    print " !Config VDisk ACK received\n";
                }
                elsif ($packet->{'command'} == X1RPKT_GET_ENVIRON)
                {
                    # This is the Environmental Info "heartbeat".  It occurs
                    # every 10 sec. Basically we ignore it.
                    print " !Environmental/heartbeat packet received\n";
                }
                elsif ($packet->{'command'} == X1PKT_REPLY_SU_ITEM)
                {
                    #  VLink Storage Unit Item
                    my %suInfo;
                    my %suWwn;

                    my $suData = $packet->{'data'};

                    # Get the storage unit info from the packet.
                    (
                        $suInfo{ID},
                        $suWwn{LO_LONG}, $suWwn{HI_LONG},
                        $suInfo{NAME},
                        $suInfo{LUNS},
                        $suInfo{TYPE}
                    ) = unpack X1_SU_ITEM, $suData;

                    # Now fixup all the 64 bit  numbers
                    $suInfo{WWN} = longsToBigInt(%suWwn);
                    $suInfo{WWN_HI} = $suWwn{HI_LONG};
                    $suInfo{WWN_LO} = $suWwn{LO_LONG};

                    printf "> SU ID=%d  WWN=%8.8X%8.8X  name=%s  luns=%d  type=%d\n",
                           $suInfo{ID}, $suInfo{WWN_LO}, $suInfo{WWN_HI},
                           $suInfo{NAME}, $suInfo{LUNS}, $suInfo{TYPE};
                }
                elsif ($packet->{'command'} == X1PKT_REPLY_LUN_ITEM)
                {
                    #  VLink LUN Info Item
                    my %lunInfo;

                    my $lunData = $packet->{'data'};

                    # Get the storage unit info from the packet.
                    (
                        $lunInfo{ID},
                        $lunInfo{LUN},
                        $lunInfo{RTYPE},
                        $lunInfo{VID},
                        $lunInfo{SIZE},
                        $lunInfo{SN},
                        $lunInfo{VBLOCK},
                        $lunInfo{BASEVID},
                        $lunInfo{NUMSERVERS},
                        $lunInfo{NUMLINKS}
                    ) = unpack X1_LUN_ITEM, $lunData;

                    printf "> SU ID=%d  lun=%d  rType=0x%02X  vid=%d  size=%d  ".
                           "SN=%d  vBlock=%d  baseVid=%d  #servers=%d  #links=%d\n",
                           $lunInfo{ID}, $lunInfo{LUN}, $lunInfo{RTYPE},
                           $lunInfo{VID}, $lunInfo{SIZE}, $lunInfo{SN},
                           $lunInfo{VBLOCK}, $lunInfo{BASEVID},
                           $lunInfo{NUMSERVERS}, $lunInfo{NUMLINKS};
                }
                elsif ($packet->{'command'} == X1RPKT_GET_RAIDPER)
                {
                    #  Raid init percenatage Info Item
                    my %raidPctInfo;

                    my $raidPctData = $packet->{'data'};

                    # Get the raid init info from the packet.
                    (
                        $raidPctInfo{ID},
                        $raidPctInfo{PCTREM}
                    ) = unpack "SC", $raidPctData;

                    printf " !Raid Init Percentage - RID: %d, PCTREM: %d\n",
                             $raidPctInfo{ID}, $raidPctInfo{PCTREM};
                }
                else
                {
                    printf(" !Recv'd async message %u (0x%02X), discarding...\n",
                            $packet->{'command'}, $packet->{'command'});
                }
                next;
            }
            else {
                $rc = $packet;
                last;
            }
        }
        else
        {
            my %error = $self->getLastError();
            logMsg("Error while receiving packet, error reason = $error{'string'}\n");
        }
    }

    logMsg("done (got cmd $responseCC)\n");

    return (defined($flush)) ? $count : $rc;
}

##############################################################################
# Name:     _validateXiotechPacket
#
# Desc:     Validates that the tbolt packet is correct
#
# In:       scalar  $packet     Packet to be checked
#
# Out:      none (can leave out if not needed (this is for variables passed by
#           reference and modfied)
#
# Returns:  1 if all ok else returns 0
##############################################################################
sub _validateXiotechPacket
{
    my ($self, $packet) = @_;

    logMsg("_validateXiotechPacket...begin\n");

    my $header = $packet->{'header'};
    my $data = $packet->{'data'};
    my $partialHdr = substr($header, 0, length($header) - 16);
    my %pkt = disassembleXiotechPacket($packet);
    my $key = pack("H32", PI_MD5_KEY);

    #Check if header md5 ok
    my $md5 = XIOTech::md5->new();
    $md5->add($partialHdr);
    $md5->add($key);

    if( $md5->digest ne $pkt{HEADER_MD5} )
    {
        logMsg("_validateXiotechPacket: header MD5 bad\n");
        print("_validateXiotechPacket: header MD5 bad\n");
        return 0;
    }

    #Check data portion of md5sum
    if(defined($pkt{DATA}) and length($pkt{DATA}))
    {
        undef $md5;
        $md5 = XIOTech::md5->new();
        $md5->add($pkt{DATA});

        if( $md5->digest ne $pkt{DATA_MD5} )
        {
            logMsg("_validateXiotechPacket: data MD5 bad\n");
            print("_validateXiotechPacket: data MD5 bad\n");
            return 0;
        }
    }

    return 1;
}

##############################################################################
# Name:  _handleError
#
# Desc: Generic error handler which will log the packet and/or give a
#       meaningful error message.
#
# In:    Packet in question
#
# Returns: None
#
##############################################################################
sub _handleError
{
    my ($self, $packet) = @_;

    logMsg("_handleError...begin\n");

    my $errorMessage = "";

    if (defined($packet))
    {
        my %parts = disassembleXiotechPacket($packet);

        $errorMessage = $self->getErrorMsg($parts{STATUS},
            $parts{ERROR_CODE}, $parts{COMMAND_CODE});

        if (defined($errorMessage))
        {
            print "$errorMessage\n";
        }
    }

    logMsg("_handleError...exit\n");
}

##############################################################################
# Name:     _handleSyncResponse
#
# Desc:     In all the simple cases we send a packet with a command, wait for
#           the command in progress and then the actual command information
#           packet and then check for command successful.
#
#           This function can be use in all those cases as it checks for the
#           command in Progress packet and then calls a function by reference
#           supplied by the caller that can handle the information packet etc.
#
# In:       scalar  $seq            Sequence number
#           scalar  $packet         Command packet to send
#           scalar  $packetHandler  Reference to the function to handle the
#                               informational packet
#
# Returns:  Empty hash if we had errors, else returns a hash with the user
#           requested information
##############################################################################
sub _handleSyncResponse
{
    my ($self,
        $seq,
        $packet,
        $commandHandler) = @_;
    my $rc = 1;
    my $recvPacket;
    my %packetInfo;
    my $headerInfo;

    if ($self->{PORT} == PI_X1_PORT) {
        my %X1packetInfo =
            _handleX1SyncResponse(
                    $self,
                    $packet,
                    X1RPKT_BIGFOOT_CMD,
                    undef);

        my $bfPacket;
        $bfPacket->{'header'} = substr($X1packetInfo{DATA}, 0, 128);
        $bfPacket->{'data'} = substr($X1packetInfo{DATA}, 128);
        $bfPacket->{'PORT'} = $self->{PORT};

        logMsg("Calling command handler on X1/BF packet\n");
        %packetInfo = &$commandHandler($self, $seq, $bfPacket);

        $headerInfo = disassembleXiotechHeader($bfPacket->{'header'},
                                                    $self->{PORT});
        $packetInfo{STATUS} = $headerInfo->{STATUS};
        $packetInfo{ERROR_CODE} = $headerInfo->{ERROR_CODE};
        $packetInfo{STATUS_MSG} = $self->getStatusMsg($packetInfo{STATUS});
        $packetInfo{ERROR_MSG} = $self->getErrorMsg($packetInfo{STATUS},
                $packetInfo{ERROR_CODE}, $headerInfo->{COMMAND_CODE});
        return %packetInfo;
    }

    logMsg("Sending packet\n");

    $rc = $self->write_packet('packet' => $packet);

    if (!$rc)
    {
        # Set error status so we don't send anymore commands
        $rc = 0;

        my %error = $self->getLastError();
        logMsg("Error while sending packet, reason = $error{'string'}\n");
    }

    if ($rc)
    {
        $rc = $self->_processCommandInProgress($seq);

        if (!$rc)
        {
            logMsg("Did not get command in progress\n");
        }
    }

    if ($rc)
    {
        $recvPacket = $self->_receivePacketSync($seq);

        if (!defined($recvPacket))
        {
            logMsg("Unable to retrieve response packet.\n");
            $rc = 0;
        }
    }

    if ($rc)
    {
        $headerInfo = disassembleXiotechHeader($recvPacket->{'header'},
                                                    $self->{PORT});

        if ($headerInfo->{STATUS} == PI_GOOD)
        {
            %packetInfo = &$commandHandler($self, $seq, $recvPacket);
        }
        else
        {
            if (($headerInfo->{COMMAND_CODE} == PI_GENERIC_MRP_CMD) or
               ($headerInfo->{COMMAND_CODE} == PI_DEBUG_SCSI_COMMAND_CMD))
            {
                %packetInfo = &$commandHandler($self, $seq, $recvPacket);
            }

            $packetInfo{STATUS} = $headerInfo->{STATUS};
            $packetInfo{ERROR_CODE} = $headerInfo->{ERROR_CODE};
            $packetInfo{STATUS_MSG} = $self->getStatusMsg($packetInfo{STATUS});
            $packetInfo{ERROR_MSG} = $self->getErrorMsg($packetInfo{STATUS},
            $packetInfo{ERROR_CODE}, $headerInfo->{COMMAND_CODE});
        }
    }

    if (!$rc)
    {
        $packetInfo{STATUS} = PI_SOCKET_ERROR;
        $packetInfo{ERROR_CODE} = 0;
        $packetInfo{STATUS_MSG} = $self->getStatusMsg($packetInfo{STATUS});
        $packetInfo{ERROR_MSG} = $self->getErrorMsg($packetInfo{STATUS},
                $packetInfo{ERROR_CODE}, $headerInfo->{COMMAND_CODE});
    }

    return %packetInfo;
}

##############################################################################
# Name:     _handleX1SyncResponse
#
# Desc:     In all the simple cases we send a packet with a command, wait for
#           the command in progress and then the actual command information
#           packet and then check for command successful.
#
#           This function can be use in all those cases as it checks for the
#           command in Progress packet and then calls a function by reference
#           supplied by the caller that can handle the information packet etc.
#
#           scalar  $packet         Command packet to send
#           scalar  $returnCC       Expected return command code
#           scalar  $packetHandler  Reference to the function to handle the
#                               informational packet
#
# Returns:  Empty hash if we had errors, else returns a hash with the user
#           requested information
##############################################################################
sub _handleX1SyncResponse
{
    my ($self,
            $packet,
            $returnCC,
            $commandHandler) = @_;
    my $rc = 1;
    my $recvPacket;
    my %packetInfo;

    logMsg("Sending X1 packet\n");

    if ($self->{PORT} != PI_X1_PORT) {
        print "This command is only supported on the X1 port (".PI_X1_PORT.")!\n\n";
        logMsg "This command is only supported on the X1 port (".PI_X1_PORT.")\n";
        my %hash;
        $hash{STATUS} = PI_ERROR;
        $hash{ERROR_CODE} = 0;
        return %hash;
    }

    $rc = $self->write_packet('packet' => $packet);

    if (!$rc)
    {
        # Set error status so we don't send anymore commands
        $rc = 0;

        my %error = $self->getLastError();
        logMsg("Error while sending X1 packet, reason = $error{'string'}\n");
    }

    else
    {
        $recvPacket = $self->_receiveX1PacketSync($returnCC);

        if (!defined($recvPacket)) {
            logMsg("Unable to retrieve X1 response packet.\n");
            $rc = 0;
        }
        else {
            if(defined($commandHandler)) {
                %packetInfo = &$commandHandler($self, $recvPacket);
            }
        }
    }

    if ($rc) {
        if (!(defined($packetInfo{STATUS}))) {
            $packetInfo{STATUS} = PI_GOOD;
        }
        if (!(defined($packetInfo{ERROR_CODE}))) {
            $packetInfo{ERROR_CODE} = 0;
        }
        $packetInfo{DATA} = $recvPacket->{'data'};
    }
    else {
        $packetInfo{STATUS} = PI_ERROR;
        $packetInfo{ERROR_CODE} = 0;
    }

    return %packetInfo;
}

##############################################################################
# Name:     _get_socket
#
# Desc:     gets a read or write socket
#
# In:       scalar      "recv" or "send"
#
# Returns:  a socket on success, undef on failure
#
##############################################################################
sub _get_socket
{
    my ($self, $type) = @_;
    my ($socket);

    logMsg("get_socket begin...\n");

    if ($type eq "recv")
    {
        $socket = $self->_get_recv_socket();
    }
    elsif ($type eq "send")
    {
        $socket = $self->_get_send_socket();
    }
    else
    {
        logMsg("Can only use 'recv' or 'send' in get_socket function\n");
        return undef;
    }

    if (defined $socket)
    {
        $self->{'socket'} = $socket;
        return $socket;
    }
    else
    {
        $self->{'last_error_num'} = ERR_SOCKET_CREATION_FAILED;
        logMsg("Socket creation failed with $self->{'HOST'}:$self->{'PORT'}\n");
        return undef;
    }
}

##############################################################################
# Name:     _get_recv_socket
#
# Desc:     gets a read socket
#
# Returns:  a socket on success, undef on failure
#
##############################################################################
sub _get_recv_socket
{
    my ($self) = @_;

    return(IO::Socket::INET->new(LocalAddr => $self->{'HOST'},
                                    LocalPort => $self->{'PORT'},
                                    Listen => 2,
                                    Reuse => 1,
                                    Proto => "tcp",
                                    Type => SOCK_STREAM,
                                    Timeout => 5));
}

##############################################################################
# Name:     get_send_socket
#
# Desc:     gets a send socket
#
# Returns:  a socket on success, undef on failure
#
##############################################################################
sub _get_send_socket
{
    my ($self) = @_;

    return(IO::Socket::INET->new(PeerAddr => $self->{'HOST'},
                                    PeerPort => $self->{'PORT'},
                                    Proto => "tcp",
                                    Type => SOCK_STREAM,
                                    Timeout => 5));
}

##############################################################################
# Name:  _init
#
# Desc: Initializes this class
#
# In:   Arguments from new
#
# Returns: An instance of this class, aka. an object to use
#
##############################################################################
sub _init
{
    my ($self, @arguments) = @_;

    logMsg("_init...begin\n");

    # Make sure we don't initialize more than once for any given object
    # This is only really a problem when you are using multiple inheritance
    # and you have the dreaded "diamond" inheritance scenario
    my $pak = __PACKAGE__;
    return if $self->{_init}{$pak}++;

    my $proc = $$;

    $self->{SEQ} = XIOTech::seqNumber->new();

    # Create a hash to store asyc information in
    $self->{ASYNC} = {};
}

##############################################################################
# Name:  _genericMRPResponsePacket
#
# Desc: Handles a response packet from a generic MRP request.
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: hash with the follwing (empty if failure):
#               STATUS
#               RSP_DATA
#
##############################################################################
sub _genericMRPResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    my %rdData;

    logMsg("_genericMRPResponsePacket...begin\n");

    if (defined($recvPacket))
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $rdData{STATUS} = $parts{STATUS};
        $rdData{ERROR_CODE} = $parts{ERROR_CODE};
        $rdData{RSP_DATA} = $parts{DATA};
        $rdData{STATUS_MSG} = $self->getStatusMsg($rdData{STATUS});
        $rdData{ERROR_MSG} = $self->getErrorMsg($rdData{STATUS},
            $rdData{ERROR_CODE}, $parts{COMMAND_CODE});
    }

    return %rdData;
}

##############################################################################
# Name:     _handleResponseX1Probe
#
# Desc:     Handle an X1 Secure Login return packet
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       STATUS         Status of the command
#       SERIAL_NUM     Serial number of controller
#
##############################################################################
sub _handleResponseX1Probe
{
    my ($self,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    my ($length, $sn) = unpack "CL", $parts{DATA};

    $info{STATUS} = PI_GOOD;
    $info{SERIAL_NUM} = $sn;

    return %info;
}

##############################################################################
# Name:     _handleResponseX1SecureLogin
#
# Desc:     Handle an X1 Secure Login return packet
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#       STATUS         Status of the command
#       MASTER_CRYPT   Master encryption key to use from here on
#
##############################################################################
sub _handleResponseX1SecureLogin
{
    my ($self,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    my ($status, $mcrypt) = unpack "CL", $parts{DATA};
    if ($status == 0) {
        $info{STATUS} = PI_GOOD;
        $info{MASTER_CRYPT} = $mcrypt;
    }
    else {
        $info{STATUS} = PI_ERROR;
        logMsg("status = $status\n");
    }

    return %info;
}

##############################################################################
# Name:  DESTROY
#
# Desc: Cleans up an object
#
# In:   None
#
# Returns: None
#
# Notes:    DO NOT CALL DIRECTLY
#
##############################################################################
sub DESTROY
{
    my ($self) = @_;
}

##############################################################################
# Name:  _receivePacketASync
#
# Desc: Reads a packet from the SAN box and waits until we get it or a timeout
#       occurs
#
# In:
#
# Returns:  undef if we had errors, otherwise a packet that has had it's md5
#           checked
#
##############################################################################
sub _receivePacketASync
{
    my ($self,
        $seq,
        $opt,
        $packet,
        $commandHandler) = @_;
    my $rc = 1;
    my $recvPacket;
    my %packetInfo;
    my $headerInfo;
    my $possible_exit = 0;

    logMsg("Sending packet\n");
    printf("Sending packet\n");

    $rc = $self->write_packet('packet' => $packet);

    logMsg("_receivePacketASync...begin \n");
    printf("_receivePacketASync...begin opt=($opt)\n");

    while(1)
    {
        my $packet = $self->read_packet();

        if (defined($packet))
        {
            logMsg("validating xiotech packet...\n");

            if ($self->_validateXiotechPacket($packet))
            {
                # Interpret packet
                $headerInfo = disassembleXiotechHeader($packet->{'header'},
                                                            $self->{PORT});

                if (($headerInfo->{COMMAND_CODE} == PI_LOG_EVENT_MESSAGE))
                {
                    my $outfile;
                    my %parts = disassembleXiotechPacket($packet);
                    my %info = logInfoData(%parts);
                    printf("Async Log Event:\n");
                    $self->displayLogInfo($outfile, $info{MODE}, 0x01, 1, %info);
                }
                elsif (($headerInfo->{COMMAND_CODE} == PI_ASYNC_CHANGED_EVENT))
                {
                    my %parts = disassembleXiotechPacket($packet);
                    my $event = changedEventData(%parts);
                    my $i = changedEventString($event);
                    printf("Async Changed Event %u (0x%x)",$event, $event);
                    if (defined($i))
                    {
                        printf(" ($i)\n");
                    }
                    else
                    {
                        printf(" (unknown?)\n");
                    }
                }
                elsif (($headerInfo->{COMMAND_CODE} == PI_ASYNC_PING_EVENT))
                {
                    printf("PI_ASYNC_PING_EVENT\n");
                }
                elsif (($headerInfo->{COMMAND_CODE} == PI_REGISTER_EVENTS_CMD))
                {
                    my $printfail = 0;
                    if ($headerInfo->{STATUS} != 0 &&
                         $headerInfo->{STATUS} != PI_ERROR_INV_PKT_SIZ)
                    {
                        $printfail = 1;
                    }
                    if ($printfail != 0)
                    {
                        printf("PI_REGISTER_EVENTS_CMD:\n");
                        foreach my $i (keys(%$headerInfo))
                        {
                            if ($i eq 'HEADER_MD5') {next; };
                            if ($i eq 'DATA_MD5') {next; };
                            printf("    headerInfo->$i='$headerInfo->{$i}'\n");
                        }
                        print "Register Events failed!\n";
                        print "\n";
                    }
                    my %parts = disassembleXiotechPacket($packet);
                    if ($printfail != 0)
                    {
                        printf "Status Code:    0x%02x  ", $parts{STATUS};
                        if (defined($parts{STATUS_MSG}))
                        {
                            printf " \"%s\"", $parts{STATUS_MSG};
                        }
                        print "\n";
                        printf "Error Code:     0x%02x  ", $parts{ERROR_CODE};
                        if (defined($parts{ERROR_MSG}))
                        {
                            printf " \"%s\"", $parts{ERROR_MSG};
                        }
                        print "\n";

                        foreach my $i (keys(%parts))
                        {
                            if ($i eq 'HEADER_MD5') {next};
                            if ($i eq 'DATA_MD5') {next};
                            printf("    parts->$i=");
                            if (defined($parts{$i}))
                            {
                                if ($i eq 'DATA')
                                {
                                   my @k = unpack('C' x $parts{DATA_LENGTH}, $parts{DATA});
                                   for (my $j = 0; $j < $parts{DATA_LENGTH}; $j++)
                                   {
                                     printf("%-2.2x", $k[$j]);
                                   }
                                   printf("\n");
                                }
                                else
                                {
                                    printf("'$parts{$i}'\n");
                                }
                            }
                            else
                            {
                                printf("undefined\n");
                            }
                        }
                    }
                    else
                    {
                        if (defined($parts{DATA_LENGTH}) && $parts{DATA_LENGTH} > 0)
                        {
                            if ($opt == 0)
                            {
                                printf("Register Events successful, DATA=");
                            }
                            else
                            {
                                printf("Get Registered Events successful, DATA=");
                            }
                            my $k = changedEventData(%parts);
                            printf("%-16.16x\n", $k);
                            my $i = changedEventString($k);
                            if (defined($i))
                            {
                                printf("  Events: $i\n");
                            }
                            $possible_exit = 1;
                        }
                    }
                }
                else
                {
                    printf("Unexpected response packet (COMMAND_CODE=%u [0x%x])?\n",
                            $headerInfo->{COMMAND_CODE},$headerInfo->{COMMAND_CODE});
                }
            }
            else
            {
                $self->_handleError($packet);
                logMsg("Packet is invalid.\n");
                printf("Packet is invalid.\n");
            }
        }
        else
        {
            my %error = $self->getLastError();
            logMsg("Error while receiving packet, error reason = $error{'string'}\n");
            printf("Error while receiving packet, error reason = $error{'string'}\n");
            last;
        }
        if ($opt == 1 && $possible_exit == 1)
        {
            last;
        }
    }
    return $rc;
}

##############################################################################
# Name:  _receiveX1PacketSync
#
# Desc: Reads a packet from the SAN box and waits until we get it or a timeout
#       occurs
#
# In:   Command Response Code we are expecting
#


1;

##############################################################################
#
# Change log:
# $Log$
# Revision 1.10  2006/12/20 11:16:54  BharadwajS
# TBolt00017375 Changing PI_COMPATIBILITY to 3
#
# Revision 1.9  2006/08/17 17:32:44  NigburC
# TBolt00000000
# Fixed some formatting issues with vdisks and fixed indentation issues in code
# itself.
#
# Revision 1.8  2006/07/21 14:53:03  HoltyB
# TBolt00000000
# Initial checkin of ewok persistent data ccbe library and ccbcl access.
#
# Revision 1.7  2006/07/17 20:38:32  RustadM
# TBolt00014770
# Move 750 branch onto main.
#
# Revision 1.6.2.3  2006/04/26 09:22:29  BharadwajS
# CCBE changes for PI Versioning
#
# Revision 1.6.2.2  2006/04/26 09:06:20  BharadwajS
# CCBE changes for PI Versioning
#
# Revision 1.6.2.1  2006/04/10 19:11:17  wirtzs
# updates for 750
#
# Revision 1.6  2005/12/27 08:29:37  BalemarthyS
# TBolt00000000 - Removed new line and tab characters
#
# Revision 1.5  2005/12/23 08:13:48  BalemarthyS
# Merged ISCSI & GEORAID related changes
#
# Revision 1.4  2005/08/12 20:43:35  HoltyB
# TBolt00000000:  Fixed probe packet to send right amount of data
#
# Revision 1.3  2005/06/17 20:00:35  RysavyR
# TBolt00013002: Fixed the return codes on writebuf and scsicmd so that they return an "ecode" instead of an MRP return code. Rev by Bryan Holty.
#
# Revision 1.2  2005/05/16 13:53:02  McmasterM
# TBolt00012856: CCB Seg Fault when updating code via CCBCL with bad file name.
# The CCBE was not checking for a failed open, and the CCB was not checking
# for a NULL packet pointer.  I changed the CCBE to verify the open's success and
# changed the CCB to check for a NULL packet pointer.
# Reviewed by Randy Rysavy
#
# Revision 1.1.1.1  2005/05/04 18:53:54  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.101  2005/04/22 20:46:53  RysavyR
# TBolt00012443: Cleanup of old MPX commands and addition of a -S option FIDREAD and a -N option to FIDWRITE. Rev by Holty.
#
# Revision 1.100  2004/08/12 17:39:15  RysavyR
# TBolt00011024: Add 'Get Config' packet to 2341 connection login so that a
# Bigfoot/Wookiee controller type can be established.
#
# Revision 1.99  2004/06/04 15:17:37  RysavyR
# TBolt00000000: Updates for Wookiee controller co-existence.  Also added
# SETCONTROLLERTYPE to manually set the current connection to a specific
# type of controller (necessary, for now anyway, when logging in on the 2341 port).
#
# Revision 1.98  2004/04/28 18:17:38  HoltyB
# TBolt00000000:  Added Wookiee andling for the CCBE amd CCBCL.
#
# Revision 1.97  2004/03/31 16:00:00  NigburC
# TBolt00000000 - Added the range list feature to allow a combination of
# comma separated values and ranges where available.
# Reviewed by Randy Rysavy.
#
# Revision 1.96  2004/03/04 22:16:30  RysavyR
# Add a message for when an env/heartbeat message comes in asynchronously
# on the 2341 port. Rev by Mark S.
#
# Revision 1.95  2004/03/04 21:54:26  RysavyR
# Flush the 2341 port asynchronous messages every time an enter is pressed
# with no command present.  Also flush the queue before x1getenvir
# is called. Rev by Mark.
#
# Revision 1.94  2003/10/14 14:33:25  McmasterM
# TBolt00009397: Add logic to CCB to gather FCAL counters in background
# Added logic to CCB to collect and process the FCAL counters.  The data is
# stored in several arrays in the CCB DRAM, and are retrievable through the CCBE
# using the command 'fidread 299'.  The snapshot tools and DDR decoder have
# also been modified so that they are able to process the new arrays.
# Portions reviewed by Brett Tollefson
#
# Revision 1.93  2003/08/27 18:23:37  NigburC
# TBolt00000000 - Changed the default parity for RAID 5 devices to be 5
# instead of 9.
# Reviewed by Jeff Williams.
#
# Revision 1.92  2003/08/12 19:06:38  McmasterM
# TBolt00000000: GeoRaid election changes (network access to CQ down)
# This checkin consists of a first pass of GeoRaid election support.  It is about
# 60 precent complete, and not totally full function.  It does have some crude
# disaster detection logic in place, but does not change current powerup logic.
#
# Revision 1.91  2003/08/01 19:54:01  NigburC
# TBolt00000000 - Added code to save the controller serial number after
# connecting to the controller on the normal 3100/3200 ports.  Added code
# in the validate module to be smart about which server statistics to gather, it
# will now only gather stats for the servers the controller owns.
# Reviewed by Craig Menning.
#
# Revision 1.90  2003/07/17 18:44:40  HoltyB
# TBolt00008692:  Added Raid5 Init_Percent_Left packet support.
# Reviewed by Mark Schibilla.
#
# Revision 1.89  2003/07/01 21:08:45  McmasterM
# TBolt00008601: GeoRAID: Add "disaster mode" signature to NVRAM
# TBolt00008603: GeoRAID: Add "keep-alive" DCN designation to masterConfig structure
# Added basic level of keepAlive and disasterMode support to the masterConfig and
# NVRAM definitions.  Reviewed by Chris Nigbur
#
# Revision 1.88  2003/05/07 13:19:31  HoltyB
# TBolt00007922:  Remove SNMP from the CCBE code.
#
# Revision 1.87  2003/04/29 17:28:51  SchibillaM
# TBolt00007922: Add support for X1VCGINFO.
#
# Revision 1.86  2003/03/27 15:18:51  SchibillaM
# TBolt00007915: Add X1 support for VLink commands - VLinked To, VLink LUN
# Info and VLink Storage Unit Info.
#
# Revision 1.85  2003/03/19 21:11:28  RysavyR
# TBolt00007392: Added additional snapshot support - FID write capability.
# Rev by TimSw.
#
# Revision 1.84  2003/01/22 15:07:07  RysavyR
# TBolt00006073: Added support for readFid() and readMemory() so snapshot
# data can be captured on the XSSA.  Rev by TimS.
#
# Revision 1.83  2003/01/16 22:37:34  HoltyB
# TBolt00006803:  Multiple changes to the way caching works.  Also made
# a queue for X1 async notifications.  Fixes problem of being forked to death.
#
# Revision 1.82  2003/01/02 17:10:44  HoltyB
# TBolt00006385:  Added X1LogEntry command to retrieve log entries through
# the X1 packet interface.
#
# Revision 1.81  2002/12/20 20:44:42  RysavyR
# TBolt00006508: Properly detect and handle socket timeout when issuing a new command.
#
# Revision 1.80  2002/12/17 21:52:30  HoltyB
# TBolt00006477: Fixed the X1S PDisk log message packets (PdiskEvent).
#
# Revision 1.79  2002/12/05 17:05:44  RysavyR
# TBolt00006420:  Got rid of the extra packet padding necessary for encryption (not
# needed anymore).  Rev by TimSw.
#
# Revision 1.78  2002/11/26 15:30:21  SwatoshT
# Tbolt00006344: Added support for retrieving packet statistics.
#
# Revision 1.77  2002/10/30 15:30:17  HoltyB
# TBolt00006236: Added checksum to persistent data
#
# Revision 1.76  2002/10/29 20:39:28  RysavyR
# TBolt00006013: Removed all Blowfish encryption files and calls in the code.
# Removed Big Number stuff since no Diffie-Hellman needed.  Removed the
# secondary encrypted port support.  Rev by Tim Swatosh.
#
# Revision 1.75  2002/10/29 17:27:46  RysavyR
# TBolt00006013: Adjust (down) max packet size on X1 port. Rev by Bryan H.
#
# Revision 1.74  2002/10/25 16:04:41  RysavyR
# TBolt00006013: Added X1 config "encryption" support and added intermediate
# VDisk config ACK.  Rev by Mark S.
#
# Revision 1.73  2002/10/22 21:37:10  RysavyR
# TBolt00006013:  Started adding VDISK Config support on the X1 port.
#
# Revision 1.72  2002/10/19 19:14:13  HoltyB
# TBolt00006201:  Added persistent data functionality to CCB.
#
# Revision 1.71  2002/10/17 23:52:16  SwatoshT
# TBolt00006013: Added support for X1sending of VCG Async Events and
# initial support for packed log events. Removed sending of debug messages
# on X1 port.
# Reveiwed by Mark S.
#
# Revision 1.70  2002/10/16 21:02:42  RysavyR
# TBolt00006136: Added support for X1GETACCOUNT and X1SETACCOUNT.
#
# Revision 1.69  2002/10/15 14:50:30  RysavyR
# Renamed X1FWUPDATE, X1TRYCCB & X1READFID to MPX... since these
# are not X1 specific commands.  I will reserve anything that begins with X1 to
# commands that are only supported on the X1 port.
#
# Revision 1.68  2002/10/14 21:29:15  RysavyR
# TBolt00006136: Added multi-packet support for transferring fw and large files
# within packets that are <64K.  Rev. by TimSw.
#
# Revision 1.67  2002/10/02 15:30:34  RysavyR
# X1 port improvements.  Decode Async Log messages and display as they
# come in.
#
# Revision 1.66  2002/10/02 14:05:17  RysavyR
# TBolt00006013:  Remove some debug code before the build...
#
# Revision 1.65  2002/10/01 19:01:27  RysavyR
# TBolt00006013:  Add the ability to handle and process BF style packets on
# the X1 port. Reviewed by TimSw.
#
# Revision 1.64  2002/09/03 14:32:52  SchibillaM
# TBolt00005858: Changes to CCBCL files to allow a subset of function to be built
# for field use.  The tool BuildCCBCLSubset.pl builds the subset.  These changes
# also remove ENVSTATS which is replaced by STATSENV.
# Reviewed by Randy and Craig.
#
# Revision 1.63  2002/08/07 14:24:11  RysavyR
# TBolt00005692: Add version data exchange on initial connect. Rev by
# Mark S.
#
# Revision 1.62  2002/07/10 01:28:54  HoltyB
# TBolt00005254: Changed Loginfo to display in the current computers local
# time zone.  Also added an option to still display the GMT time.
# TBolt00005248: Added new interface to write a debug message to the
# CCB logs.
#
# Revision 1.61  2002/07/02 13:58:10  NigburC
# TBolt00000000 - Removed the time set for the controller at every login from
# the CCBE.  Added descriptive text to the POWERUPSTATE display.
#
# Revision 1.60  2002/06/24 12:37:50  NigburC
# TBolt00004720 - Added code to set the time of a controller when the login
# is called.
# Reviewed by Mark Schibilla.
#
# Revision 1.59  2002/06/04 19:18:07  RysavyR
# TBolt00003598: Added the first pass at configuration snapshotting.
#
# Revision 1.58  2002/05/22 15:50:48  RysavyR
# TBolt00004476: Rewrote writeSocketData() so that it only sends 100K at a time.
# This gets around the error where syswrite() blows on very large writes.
#
# Revision 1.57  2002/05/15 18:34:55  HoltyB
# TBolt00004254: Added the ability to error trap the Front End, Back End, or the
# CCB
#
# Revision 1.56  2002/05/09 12:59:53  RysavyR
# TBolt00000000: Allow connection attempt on any port.
#
# Revision 1.55  2002/04/24 13:58:25  RysavyR
# TBolt00001738: Added generic SCSI command. Rev by TimSw
#
# Revision 1.54  2002/04/10 13:44:50  RysavyR
# TBolt00003360: Add MD5 to the main port (3100)
#
# Revision 1.53  2002/04/05 19:45:47  NigburC
# TBolt00003119 - Modified the login to set the default timeout for the
# packet interface to 1800 seconds.  Eventually the CCB will change to have
# a timeout of 60 seconds or less and we still want the CCBE to function with
# the longer timeout.
#
# Revision 1.52  2002/04/01 21:32:59  RysavyR
# Added TRYCCB functionality
#
# Revision 1.51  2002/04/01 16:44:21  HoltyB
# Added generic function to call failure manager
#
# Revision 1.50  2002/03/25 16:59:16  HoltyB
# TBolt00003442: Added support for SNMP configuration on the ccb to allow
# ip addresses to be sent to the ccb to be used for generating traps
#
# Revision 1.49  2002/03/19 22:45:14  RysavyR
# TBolt00003360:  Add MD5 signing and checking into the packet interface.
# Add in a second listening port (3110) for encrytion development testing.
# For now, only check MD5 on port 3110.
#
# Revision 1.48  2002/03/12 17:50:44  HoltyB
# TBolt00003309: Added a generic command to kick off an election
#
# Revision 1.47  2002/02/19 19:08:25  HoltyB
# Added DEBUGADR to set the address of the debug console
#
# Revision 1.46  2002/02/14 21:27:24  RysavyR
# Allow connections on port 3100 or 3110.  3110 will be the encrypted test port.
#
# Revision 1.45  2002/02/13 20:50:57  RysavyR
# TBolt00003070: Changed the Port Server / Packet Interface protocol to use a 128 byte header and to send / receive even multiples of 16 bytes.
#
# Revision 1.44  2002/02/11 19:08:03  HoltyB
# moved command codes to PI_CommandCodes.pm
#
# Revision 1.43  2002/02/11 16:14:36  HoltyB
# fixed a problem in genMRP that was not allowing input
# packets to be passed in
#
# Revision 1.42  2002/01/31 21:56:51  HoltyB
# changed calcRaidParams to allow raid 1  2 disks
#
# Revision 1.41  2002/01/29 22:01:33  HoltyB
# changed _handleSyncResponse to allow return of data for
# PI_GENERIC_MRP_CMD
#
# Revision 1.40  2002/01/22 23:18:36  HoltyB
# updated calcRaidParms for raid 1 to set mirror_depth = to the number of drives
#
# Revision 1.39  2002/01/21 18:34:11  HoltyB
# added status code for PI_PARAMETER_ERROR
#
# Revision 1.38  2002/01/14 21:46:39  RysavyR
# TBolt00002816: Added a generic function call.  The user can code anything
# he wants in PI_GenFunc.c. This function can then be passed 0-8 parameters
# through the command line/packet interface.
#
# Revision 1.37  2002/01/11 22:28:20  HoltyB
# added new use statement for XIOTech::cmDiskBay;
#
# Revision 1.36  2002/01/11 19:53:52  RysavyR
# TBolt00002816: Add a "Generic MRP" command. Fix the generic RESET subCommand.
#
# Revision 1.35  2002/01/11 18:24:29  RysavyR
# TBolt00002816: Add a "Generic MRP" command
#
# Revision 1.34  2002/01/09 17:03:21  TeskeJ
# t2668 - broken loop miscompare changes - device status codes changed
# rev by Tim
#
# Revision 1.33  2002/01/09 13:39:53  HoltyB
# modified _handleSyncResponse to place string messages for the status
# and error code into the return hash if errors exist
#
# Revision 1.32  2002/01/03 21:18:11  HoltyB
# fixed log error
#
# Revision 1.31  2002/01/03 21:15:51  HoltyB
# fixed small problem with error message that prints to screen when a client
# unsuccessfully attempts to connect to a san box that already has a
# connection from another client
#
# Revision 1.30  2001/12/11 22:36:09  NigburC
# Added code to handle a failed status returned in a data packet.  If failure
# we will not call the command handler and just return the given STATUS and
# ERROR_CODE values.
#
# Revision 1.29  2001/12/07 21:47:12  NigburC
# Added the ENGDEBUG command.
#
# Revision 1.28  2001/12/07 20:13:23  NigburC
# Added the TIMEOUT command.
#
# Revision 1.27  2001/12/07 19:32:46  RysavyR
# Command timeout changed to 20 minutes to handle multiple drive bays
# being updated with the fwupdate command...  We'll fix this properly on the
# next release.
#
# Revision 1.26  2001/12/07 19:18:30  NigburC
# Modified the read packet to be 600 seconds.
#
# Revision 1.25  2001/12/07 17:11:04  NigburC
# Added VLINK commands.
# Added DEVSTATUS command.
# Added RAIDCOUNT and RAIDLIST commands.
# Reverted the byte swapping done on capacity and count 64 bit integer values
# since they really did not need to be swapped.  Only WWNs should be
# swapped.
# Fixed other bugs found during debugging.
#
# Revision 1.24  2001/12/05 14:19:09  NigburC
# Fixed some problems in calcRaidParms when using only one physical disk.
#
# Revision 1.23  2001/11/30 14:35:28  NigburC
# Changed the timeout value when calling SELECT, it is now 10 seconds.
#
# Revision 1.22  2001/11/28 16:12:21  NigburC
# Added many additional command handlers...
# TARGET commands
# SERVER commands
# Many others...
# Replaced LOGIN and LOGOUT with CONNECT and DISCONNECT
#
# Revision 1.21  2001/11/27 20:21:21  RysavyR
# Added the ability to reset the CCB via the "generic" command handler.
#
# Revision 1.20  2001/11/27 17:00:26  NigburC
# Missed a raid type in the calcRaidParms.
#
# Revision 1.19  2001/11/27 16:56:19  NigburC
# Added the following utility commands:
# calcRaidParms
# getOperationalPhysicalDiskIDs
#
# Revision 1.18  2001/11/14 13:03:04  NigburC
# Modified verify parameters to return a hash instead of just the message.
# This will allow the code to return it to the command line where it will be
# processed like any other error, except there is a MESSAGE in the
# hash which will be displayed describing the parameter error that occurred.
#
# Revision 1.17  2001/11/14 12:49:05  NigburC
# Added the VCG module.
#
# Revision 1.16  2001/11/13 23:14:31  NigburC
# Commented out the command complete processing.
#
# Revision 1.15  2001/11/13 23:02:12  NigburC
# Modified the command line interface to always expect a response packet
# returned from the commands called in the command manager.  This will
# enable the command line to check the STATUS to determine if the command
# was good or bad and then interrogate the ERROR_CODE to determine what
# the underlying error really was.
#
# Revision 1.14  2001/11/13 17:27:54  RysavyR
# Added Memory Read/Write functionality
#
# Revision 1.13  2001/11/12 22:05:25  NigburC
# Fixed the list response packet handler.  I was handling the MRP list response
# packet and not the packet interface list response.
#
# Revision 1.12  2001/11/09 14:44:44  RysavyR
# Added ls, cd & pwd commands
#
# Revision 1.11  2001/11/08 13:33:09  NigburC
# Added the _countResponsePacket and _listResponsePacket routines
# to be used to handle these response packets.
# Added cmVDisk to the file usages.
#
# Revision 1.10  2001/11/07 22:19:06  NigburC
# Removed the code for the 3007 port.
#
# Revision 1.9  2001/11/07 13:51:13  NigburC
# Change the usage of the ID tag to HEADER tag.
#
# Revision 1.8  2001/11/06 22:29:33  RysavyR
# Added firmware download capability to the port 3100 interface.
#
# Revision 1.7  2001/11/05 20:53:07  NigburC
# More cleanup work.
# - Moved environmental statistics code to cmStats.pm.
# - Added encrypt/decrypt functionality (stubs).
#
# Revision 1.6  2001/11/05 17:26:22  NigburC
# Added port to the handleSyncResponse call in the environmental stats code.
#
# Revision 1.5  2001/11/05 17:00:14  SwatoshT
# Added support for environmental statistics
#
# Revision 1.4  2001/11/02 20:35:35  NigburC
# Code cleanup time, lots of changes.
#
# Added code handling for the 3100 port and new packet interface.  BigfootCL
# is now able to connect to a machine running the new packet interface and
# send a pdiskCount command.
#
# Revision 1.3  2001/10/31 22:54:37  RysavyR
# Added FWUPDATE capability
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
