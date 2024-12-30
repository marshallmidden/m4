# $Id: cmdMgr.pm 161192 2013-06-03 18:10:22Z marshall_midden $
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
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

# For tcp keepalive.
use constant SOL_TCP        => 6;       # netinet/tcp.h, or linux/socket.h
use constant TCP_KEEPIDLE   => 4;       # netinet/tcp.h, or linux/tcp.h
use constant TCP_KEEPINTVL  => 5;       # netinet/tcp.h, or linux/tcp.h
use constant TCP_KEEPCNT    => 6;       # netinet/tcp.h, or linux/tcp.h



##############################################################################
# Name: new
#
# Desc: Creates a new commandManager
#
# In:   File handle where you want the errors to be reported to
#
#       example:
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

    if (BFconnect(@_)) {
        $self->storefwVersion($self);
        return(1);
    }
    return 0;
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
        $port,
        $noprint) = @_;

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

    # Set tcp keepalive on sockets, ignore errors.
    if (setsockopt($socket, SOL_SOCKET, SO_KEEPALIVE, 1) < 0) { perror ("setsockopt(SO_KEEPALIVE)"); }
    if (setsockopt($socket, SOL_TCP, TCP_KEEPIDLE, 3) < 0) { perror ("setsockopt(TCP_KEEPIDLE)"); }
    if (setsockopt($socket, SOL_TCP, TCP_KEEPCNT, 4) < 0) { perror ("setsockopt(TCP_KEEPCNT)"); }
    if (setsockopt($socket, SOL_TCP, TCP_KEEPINTVL, 3) < 0) { perror ("setsockopt(TCP_KEEPINTVL)"); }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $packet;

    $packet = assembleXiotechPacket(PI_CONNECT_CMD,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, COMPAT_INDEX_4);

    %rsp = $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            if (!defined($noprint) || $noprint == 0) {
                print("connect returned PI_GOOD\n");
            }
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

sub mpxSendFirmware
{
    my ($self, $filename) = @_;
    return mpxSendFirmwareCommon($self, $filename, "FLASH");
}

# type is either "FLASH" or "DRAM"
sub sendFirmwareCommon
{
    my ($self, $filename, $type) = @_;
    my %errorHash;
    $errorHash{STATUS}     = PI_ERROR;
    $errorHash{ERROR_CODE} = 0;

    logMsg("sendFirmware...begin\n");

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
            printf "%2u/$n) BAD (mpxSendFirmwareCommon)\r", $x;
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
# Desc:     Retrieves and parses a generic command
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
            $packetInfo{PI_ERROR_MSG} = $self->getPIErrorMsg($packetInfo{STATUS},
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
# Name:  _receivePacketASync
#
# Desc: Reads a packet from the SAN box and waits until we get it or a timeout
#       occurs
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

1;

##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
