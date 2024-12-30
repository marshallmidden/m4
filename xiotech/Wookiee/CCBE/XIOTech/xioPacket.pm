# $Id: xioPacket.pm 144095 2010-07-13 22:53:59Z m4 $
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
##############################################################################
package XIOTech::xioPacket;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
    PACKET_HEADER_SIZE

    encrypt
    decrypt
    readHeader
    readSocketData
    writeHeader
    writeSocketData
);

use IO::Socket;
use IO::Select;

use XIOTech::logMgr;
use strict;

# Some constants for the packet structure
use constant PACKET_HEADER_SIZE                         => 128;

##############################################################################
# Name:     readHeader
#
# Desc:     Reads a header off the socket.
#
# Returns:  Reference to hash with header information, or undef on failure
##############################################################################
sub readHeader
{
    my ($socket) = @_;

    my $header = readSocketData($socket, PACKET_HEADER_SIZE);

    if (!defined($header))
    {
        return undef;
    }

    # The time stamp is an 8 byte field, but we only use lower 4 bytes
    my ($headerLength,
            $dataLength,
            $payloadLength,
            $protocolVersion,

            $commandCode,
            $sequenceNumber,
            $discard,
            $timeStamp,

            $rsvd1,
            $statusCode,
            $errorCode,
            $rsvd2,
            $rsvd3,
            
            $rsvd4) = unpack("LLLL LLLL a3CLLL C80", $header);

    return {COMMAND_CODE => $commandCode,
            SEQUENCE_NUMBER => $sequenceNumber,
            DATA_LENGTH => $dataLength,
            PAYLOAD_LENGTH => $payloadLength,
            TIME_STAMP => $timeStamp,
            STATUS => $statusCode,
            ERROR_CODE => $errorCode,
            PROTOCOL_VER => $protocolVersion};
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
    my ($socket, $len) = @_;

    my $payloadLength = ($len + 15) & 0xFFFFFFF0;

    logMsg("readSocketData - ENTER ($len/$payloadLength)\n");

    my $data;
    my $bytes_read;
    my $offset = 0;

    while (1)
    {
        $bytes_read = $socket->sysread($data, $payloadLength, $offset);

        if (defined($bytes_read))
        {
            $payloadLength -= $bytes_read;

            if ($bytes_read == 0)
            {
                logMsg("readSocketData - Host disconnected while reading.\n");

                return undef;
            }
            elsif ($payloadLength > 0)
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
            logMsg("readSocketData - Error while reading from the socket.\n");
            return undef;
        }
    }

    $data = substr($data, 0, $len);

    return $data;
}

##############################################################################
# Name:     writeHeader
#
# Desc:     Write a header to the socket.
#
# Returns:  
##############################################################################
sub writeHeader
{
    my ($socket,
        $commandCode,
        $sequenceNumber,
        $dataLength,
        $timeStamp,
        $statusCode,
        $errorCode,
        $protocolVersion) = @_;

    my $header;
    my $payloadLength = ($dataLength + 15) & 0xFFFFFFF0;

    $header .= pack("L", PACKET_HEADER_SIZE);
    $header .= pack("L", $dataLength);
    $header .= pack("L", $payloadLength);
    $header .= pack("L", $protocolVersion);
    
    $header .= pack("L", $commandCode);
    $header .= pack("L", $sequenceNumber);
    $header .= pack("LL", 0, $timeStamp);
    
    $header .= pack("CCC", 0, 0, 0);        # rsvd1
    $header .= pack("C", $statusCode);
    $header .= pack("L", $errorCode);
    $header .= pack("L", 0);                # rsvd2
    $header .= pack("L", 0);                # rsvd3

    $header .= pack("a80", "");

    return writeSocketData($socket, $header, PACKET_HEADER_SIZE);
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
    my ($socket, $data, $len) = @_;

    logMsg("writeSocketData - ENTER ($len)\n");

    my $bytes_written;
    my $offset = 0;
    my $rc = 1;

    while (1)
    {
        $bytes_written = $socket->syswrite($data, $len);

        if (defined($bytes_written))
        {
            $len -= $bytes_written;

            if ($bytes_written == 0)
            {
                logMsg("writeSocketData - Host disconnected while writing.\n");
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
            logMsg("writeSocketData - Error while writing to the socket.\n");
            return 0;
        }
    }

    return 1;
}

##############################################################################

1;

##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
