# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Anthony Asleson
#
# Purpose:
#   Generate unique sequence numbers
##############################################################################
package XIOTech::seqNumber;

use constant START_POINT            => 40000;
use constant MAX_SEQ_ID             => 0xFFFF;

sub new
{
    my $class = shift;
    my $self = {};
    bless $self, $class;

    $self->{SEQ_NUM} = START_POINT + int(rand(10000));
    $self->{TIME} = time;

    return $self;
}

sub nextId
{
    $self = shift;

    if (++$self->{SEQ_NUM} > MAX_SEQ_ID)
    {
        $self->{SEQ_NUM} = START_POINT;
    }

    return $self->{SEQ_NUM};
}

sub nextTimeStamp
{
    $self = shift;
    return ++$self->{TIME};
}

1;

##############################################################################
#
# Change log:
# $Log$
# Revision 1.1  2005/05/04 18:53:56  RysavyR
# Initial revision
#
# Revision 1.3  2001/11/07 13:51:13  NigburC
# Change the usage of the ID tag to HEADER tag.
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
