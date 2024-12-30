#!/bin/perl -w
# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Randy Rysavy / Chris Nigbur / Tim Swatosh
#
# Purpose:
#   Interactive Command Line Interface for BIGFOOT
##############################################################################
use IO::Socket;
use IO::Select;

use XIOTech::logMgr;
use XIOTech::PI_CommandCodes;
use XIOTech::PI_Constants;
use XIOTech::xioPacket;

use constant SELECT_TIMEOUT         => 30;

##############################################################################
# Name:     sendRequest
#
# Desc:     
#
# Returns:  
##############################################################################
sub sendRequest
{
    my ($socket, $data) = @_;

    print "sendRequest - ENTER\n";

    my $dataLength = 0;
    my $rc = 0;
    my $rspHeader;

    if (defined($data))
    {
        $dataLength = length($data);
    }

    $rc = writeHeader($socket,
                        0,
                        0,
                        $dataLength,
                        0,
                        PI_GOOD,
                        0,
                        0,
                        0);

    if ($rc == 1)
    {
        $rspHeader = readHeader($socket);

        $rc = writeSocketData($socket, $data, $dataLength);

        $rspHeader = readHeader($socket);
    }

    print "sendRequest - EXIT\n";
}

##############################################################################
# Server Start
##############################################################################
my $logFile = XIOTech::logMgr::logStart("ccbServer", "TS");

# Autoflush all stdout writes
STDOUT->autoflush(1);

print "\n";
print "==============================================\n";
print "Welcome to the CCB Asyncronous Client!\n";
print "Logging information to:\n";
print "  $logFile\n";
print "==============================================\n";

my $socket;

$socket = IO::Socket::INET->new(PeerAddr => "10.64.107.125",
                                PeerPort => PI_ASYNC_PORT,
                                Proto => "tcp",
                                Type => SOCK_STREAM,
                                Timeout => 5);

if (defined($socket))
{
    print "Connection established...\n";

    sendRequest($socket, "This is a request.");
    sendRequest($socket, "This is another request.\n");
    sleep 60;
    sendRequest($socket, "This is a third request.");

    close($socket);
}
else
{
    print "Unable to establish a connection to the server.\n";
}

##############################################################################
