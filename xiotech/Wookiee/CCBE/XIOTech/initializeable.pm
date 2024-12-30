# $Id: initializeable.pm 144092 2010-07-13 21:40:51Z m4 $
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
#
# Purpose:
#   Base class used for all classed that are to be inheritable
##############################################################################
package XIOTech::initializeable;
use strict;

##############################################################################
#Base class for any class that wants to be inheritable
##############################################################################
sub new
{
    my ($class, @args) = @_;
    my $self = bless{}, ref($class) || $class;
    $self->_init(@args);
    return $self;
}

sub _init
{
    print STDERR "DEBUG- Missing _init for a dervied class.\n";
}

##############################################################################

1;

##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
