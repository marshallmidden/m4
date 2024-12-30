# $Id: seqNumber.pm 144092 2010-07-13 21:40:51Z m4 $
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
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

##############################################################################

1;

##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
