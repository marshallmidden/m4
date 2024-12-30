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
use Time::localtime;

use XIOTech::logMgr;
use XIOTech::PI_CommandCodes;
use XIOTech::PI_Constants;
use XIOTech::xioPacket;

use constant SELECT_TIMEOUT         => 30;

##############################################################################
# Name:     displayRequest
#
# Desc:     
#
# Returns:  
##############################################################################
sub displayRequest
{
    my ($request) = @_;

    print ctime();
    print " ";

    print $request;

    if (index($request, "\n", length($request)-1) == -1)
    {
        print "\n";
    }
}

##############################################################################
# Name:     processClientRequests
#
# Desc:     
#
# Returns:  
##############################################################################
sub processClientRequest
{
    my ($socket) = @_;

    my $rc;
    my $header;
    my %info;
    my $logEvent;
    my $dataLength;

    logMsg("writeSocketData - processClientRequest - ENTER\n");

    $header = readHeader($socket);

    if (!defined($header))
    {
        logMsg("writeSocketData - processClientRequest - ERROR: Failed to read a header.\n");
        return undef;
    }

    $rc = writeHeader($socket,
                        $header->{COMMAND_CODE},
                        $header->{SEQUENCE_NUMBER},
                        0,
                        $header->{TIME_STAMP},
                        PI_IN_PROGRESS,
                        0,
                        $header->{PROTOCOL_VER});

    if ($rc == 0)
    {
        logMsg("processClientRequest - ERROR: Failed to write PI_IN_PROGRESS header.\n");
        return undef;
    }

    $dataLength = $header->{DATA_LENGTH};

    if ($dataLength > 0)
    {
        $info{DATA} = readSocketData($socket, $dataLength);

		if ($header->{COMMAND_CODE} == PI_LOG_INFO_CMD) && (defined ($info{DATA})))
		{
        	%info = logInfoData(%info);
		}
		else
		{
			undef %info;
			print "Unexpected Command Code ($header->{COMMAND_CODE})\t";
			print "Data length = $dataLength \n";
		}

        $rc = writeHeader($socket,
                            $header->{COMMAND_CODE},
                            $header->{SEQUENCE_NUMBER},
                            0,
                            $header->{TIME_STAMP},
                            PI_GOOD,
                            0,
                            $header->{PROTOCOL_VER});
    }

    logMsg("processClientRequest - EXIT\n");

    return %info;
}

##############################################################################
# Name:     processClientRequests
#
# Desc:     
#
# Returns:  
##############################################################################
sub processClientRequests
{
    my ($client) = @_;

    my $socket;
    my %rsp;
    my %request;

    logMsg("processClientRequests - ENTER\n");

    my $reader = IO::Select->new();
    $reader->add($client);

    while (!defined($socket))
    {
        logMsg("processClientRequests - Calling select...\n");

        my ($ready) = IO::Select->select($reader,
                                            undef,
                                            undef,
                                            SELECT_TIMEOUT);

        if (!defined($ready))
        {
            logMsg("processClientRequests - Timeout occurred while calling select.\n");
        }
        else
        {
            # this is the socket with data on it
            $socket = @$ready[0];

            logMsg("processClientRequests - processing client request...\n");
            %request = processClientRequest($socket);

            if (%request)
            {
                displayLogInfo(undef,%request);
                undef $socket;
            }
        }
    }

    logMsg("processClientRequests - EXIT\n");
}

##############################################################################
# Server Start
##############################################################################
my $logFile = XIOTech::logMgr::logStart("ccbServer", "TS");

# Autoflush all stdout writes
STDOUT->autoflush(1);

print "\n";
print "==============================================\n";
print "Welcome to the CCB Asyncronous Server!\n";
print "Logging information to:\n";
print "  $logFile\n";
print "==============================================\n";

my $server;
my $client;

$server = IO::Socket::INET->new(LocalPort => PI_ASYNC_PORT,
                                Listen => 2,
                                Reuse => 1,
                                Proto => "tcp",
                                Type => SOCK_STREAM,
                                Timeout => 5);

while (1)
{
    logMsg("Listening for a client connection...\n");
    $client = $server->accept();

    if (defined($client))
    {
        logMsg("Client connection established.\n");
        processClientRequests($client);
    }
    else
    {
        logMsg("Received an undefined client from accept.\n");
    }
}

close($server);



##############################################################################
# Name: logInfoData
#
# Desc: Parses the log info packet data and places the information
#       in a hash
#
# In:   hash   $packet data to be parsed 
#
# Returns:
#
#       Empty hash if we have errors else a hash with the following:
#
#       SEQUENCE_NUMBER         Log event sequence number
#       TIME_AND_DATE           Time and date string
#       MESSAGE_TYPE            Message type string
#       MESSAGE_DESC            Message description string
#
##############################################################################
sub logInfoData
{
    my (%parts) = @_;

    my %info;
	my $count;

    # Unpack the data
    (
    $info{EVENT_COUNT},
    $info{MODE},
    $info{SEQUENCE_NUMBER},
    $info{CONTROLLER_SN},
    $info{VCG_ID},
    $info{RSVD_1},
    $info{RSVD_2},
    $info{RSVD_3},
    $info{RSVD_4}
    ) = unpack("SSLLLLLLL", $parts{DATA});

	$count = $info{EVENT_COUNT};

	my $start = 32;

	if (($count != 1) && ($count != 0))
	{
		print "Count not equal to 1\n";
		print "\t Count = $info{EVENT_COUNT}";
		print "\t Mode = $info{MODE}";
		print "\t SeqNum = $info{SEQUENCE_NUMBER}\n";
		undef %info;
		return %info;
	}

	for (my $i = 0; $i < $count; $i++)
	{
		my $data = substr $parts{DATA}, $start;

	    # Unpack the data
	    (
	    $info{$i}{SEQUENCE_NUMBER},
	    $info{$i}{TIME_AND_DATE},
	    $info{$i}{MESSAGE_TYPE},
	    $info{$i}{MESSAGE_LEN}) = unpack("La25a10S", $data);

		# Adjust start by fixed part of log event
		$start += 41;
		$data = substr $parts{DATA}, $start;
		my $len =  $info{$i}{MESSAGE_LEN};

		# Get the variable length of the message
	    my $varLen = "a" . $len;
        ($info{$i}{MESSAGE_DESC}) = unpack($varLen, $data);

		# Adjust start by the variable length message
		$start += $len;


	    # Fix the strings
	    $info{$i}{TIME_AND_DATE} =~ s/\x00//g;
	    $info{$i}{MESSAGE_TYPE} =~ s/\x00//g;
	    $info{$i}{MESSAGE_DESC} =~ s/\x00//g;

	}

    return %info;

}


##############################################################################
# Name:		displayLogInfo
#
# Desc:     Displays the log information.
#
# In:       None.
#
# Returns:  None.
#
##############################################################################
sub displayLogInfo
{
    my ( $filen, %info) = @_;

    my $key = 0;

    # open the output file if one was requested
    my $fh = *STDOUT;
    if (defined $filen) {
        $fh = "*FH";
        open $fh, ">$filen" or $fh = *STDOUT;
    }
   
#   print $fh "Log information:\n";

    my $count = $info{EVENT_COUNT};

#
#   Print the logs in reverse order
#
    if ($count)
    {
	for ($key = $count-1; $key >= 0; $key-- )
	{
	    printf $fh "%5lu", $info{$key}->{SEQUENCE_NUMBER};
	    print $fh "  ";
	    print $fh $info{$key}->{TIME_AND_DATE};
	    print $fh "  ";
	    print $fh $info{$key}->{MESSAGE_TYPE};
	    print $fh "  ";
	    print $fh $info{$key}->{MESSAGE_DESC};
		    print $fh "\n";
	}
    }

    if($fh ne *STDOUT) {
        close $fh;
    }
}


##############################################################################
