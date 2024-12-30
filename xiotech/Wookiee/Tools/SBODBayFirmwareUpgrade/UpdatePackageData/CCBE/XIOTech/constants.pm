# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Keith House
#
# Purpose:
#   Constants used by ACM.pm and acmpi.pm
##############################################################################
package XIOTech::constants;

use Exporter();
@ISA = qw(Exporter);
@EXPORT = qw(
    DEFAULT_TIMEOUT

    RAID_NONE
    RAID_0
    RAID_1
    RAID_5
    RAID_10

    DEFAULT_MIRROR_DEPTH
    DEFAULT_STRIPE_SIZE_RAID_0
    DEFAULT_STRIPE_SIZE_RAID_5
    DEFAULT_STRIPE_SIZE_RAID_10

    NO_ERROR
    ERR_UNKNOWN_PACKET_TYPE
    ERR_HOST_NOT_FOUND
    ERR_HOST_DISCONNECTED
    ERR_NOT_CONNECTED_TBOLT
    ERR_SOCKET_CREATION_FAILED
    ERR_NOT_CONNECTED_ACM
    ERR_WRITING_TO_ACM
    ERR_ALREADY_CONNECTED
    ERR_READING_FROM_SOCKET
    ERR_SOCKET_TIMEOUT
    ERR_FAILED_AUTHENTICATE
    ERR_NOT_LOGGED_IN
    ERR_PACKET_TOO_BIG
    ERR_UNEXPECTED_PACKET_TYPE
    ERR_SERVER_TIMED_OUT
    ERR_UNKNOWN_SERVER
    ERR_PACKET_TIMEOUT
    ERR_NOT_LOGGED_OUT
    ERR_WRITING_TO_SOCKET

    errmsg

    DDR_FID_HEADER_MAGIC_NUM
    DDR_FID_HEADER
);

use strict;

my @errmsg;
my @packet_types;

###############################################################################
# DEFAULT VALUES
###############################################################################
use constant DEFAULT_TIMEOUT                            => 600;

###############################################################################

# Raid Type Constants
use constant RAID_NONE                                  => 0;
use constant RAID_0                                     => 1;
use constant RAID_1                                     => 2;
use constant RAID_5                                     => 3;
use constant RAID_10                                    => 4;


# Default values for raid devices
use constant DEFAULT_MIRROR_DEPTH                       => 2;
use constant DEFAULT_STRIPE_SIZE_RAID_0                 => 512;
use constant DEFAULT_STRIPE_SIZE_RAID_5                 => 64;
use constant DEFAULT_STRIPE_SIZE_RAID_10                => 512;

# error codes
use constant NO_ERROR                   => 0;
use constant ERR_UNKNOWN_PACKET_TYPE    => 1;
use constant ERR_HOST_NOT_FOUND         => 2;
use constant ERR_HOST_DISCONNECTED      => 3;
use constant ERR_NOT_CONNECTED_TBOLT    => 4;
use constant ERR_SOCKET_CREATION_FAILED => 5;
use constant ERR_NOT_CONNECTED_ACM      => 6;
use constant ERR_WRITING_TO_ACM         => 7;
use constant ERR_ALREADY_CONNECTED      => 8;
use constant ERR_READING_FROM_SOCKET    => 9;
use constant ERR_SOCKET_TIMEOUT         => 10;
use constant ERR_FAILED_AUTHENTICATE    => 11;
use constant ERR_NOT_LOGGED_IN          => 12;
use constant ERR_PACKET_TOO_BIG         => 13;
use constant ERR_UNEXPECTED_PACKET_TYPE => 14;
use constant ERR_SERVER_TIMED_OUT       => 15;
use constant ERR_UNKNOWN_SERVER         => 16;
use constant ERR_PACKET_TIMEOUT         => 17;
use constant ERR_NOT_LOGGED_OUT         => 18;
use constant ERR_WRITING_TO_SOCKET      => 19;

$errmsg[NO_ERROR]                   = "No error";
$errmsg[ERR_UNKNOWN_PACKET_TYPE]    = "Unknown packet type";
$errmsg[ERR_HOST_NOT_FOUND]         = "Host not found";
$errmsg[ERR_HOST_DISCONNECTED]      = "Host disconnected";
$errmsg[ERR_NOT_CONNECTED_TBOLT]    = "No socket to TBOLT";
$errmsg[ERR_SOCKET_CREATION_FAILED] = "Socket creation failed";
$errmsg[ERR_NOT_CONNECTED_ACM]      = "No socket to ACM";
$errmsg[ERR_WRITING_TO_ACM]         = "Error writing to ACM";
$errmsg[ERR_ALREADY_CONNECTED]      = "Already connected";
$errmsg[ERR_READING_FROM_SOCKET]    = "Error reading from socket";
$errmsg[ERR_SOCKET_TIMEOUT]         = "Socket timeout";
$errmsg[ERR_FAILED_AUTHENTICATE]    = "Failed to authenticate to ACM";
$errmsg[ERR_NOT_LOGGED_IN]          = "Not logged in to the ACM";
$errmsg[ERR_PACKET_TOO_BIG]         = "Packet size exceeded 0xFFFF";
$errmsg[ERR_UNEXPECTED_PACKET_TYPE] = "Unexpected packet type";
$errmsg[ERR_SERVER_TIMED_OUT]       = "Server not responding";
$errmsg[ERR_UNKNOWN_SERVER]         = "Attempted connection to an unknown server";
$errmsg[ERR_PACKET_TIMEOUT]         = "Requested packet not found within timeout period";
$errmsg[ERR_NOT_LOGGED_OUT]         = "Must log out before logging in";
$errmsg[ERR_WRITING_TO_SOCKET]      = "Error writing to socket";

sub errmsg
{
    return $errmsg[$_[0]];
}

use constant DDR_FID_HEADER_MAGIC_NUM => 0x312A2A66;
use constant DDR_FID_HEADER =>
           "L          # magicNumber
            L          # fid
            L          # version
            L          # startAddr
            Z8         # id
            L2";       # reserved
            
1;

##############################################################################
#
# Change log:
# $Log$
# Revision 1.1  2005/05/04 18:53:54  RysavyR
# Initial revision
#
# Revision 1.10  2004/12/14 21:31:20  RysavyR
# TBolt00011557: Fixed the timestamp decoding on the CCB MRP backtrace for Wookiee captured flight recorder data. Rev by Bryan H.
#
# Revision 1.9  2002/01/29 21:24:49  HoltyB
# changed DEFAULT_TIMEOUT to 600
#
# Revision 1.8  2001/12/18 14:07:05  NigburC
# Updated default timeout to 30 seconds.
#
# Revision 1.7  2001/12/07 20:13:23  NigburC
# Added the TIMEOUT command.
#
# Revision 1.6  2001/11/27 16:52:49  NigburC
# Removed unused constants and a couple of unused functions.
# Added constants for RAID parameters.
#
# Revision 1.5  2001/11/13 15:39:55  NigburC
# Added constants for RAID types.
#
# Revision 1.4  2001/11/07 13:51:13  NigburC
# Change the usage of the ID tag to HEADER tag.
#
# Revision 1.3  2001/11/02 20:35:35  NigburC
# Code cleanup time, lots of changes.
#
# Added code handling for the 3100 port and new packet interface.  BigfootCL
# is now able to connect to a machine running the new packet interface and
# send a pdiskCount command.
#
# Revision 1.2  2001/10/31 15:42:02  NigburC
# Updated the command line to include the "logInfo" command to display
# the last N log messages.
#
# Revision 1.1.1.1  2001/10/31 12:51:30  NigburC
# Initial integration of Bigfoot command line.
#
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
