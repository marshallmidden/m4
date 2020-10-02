#!/usr/bin/perl -w

use strict;
use warnings;
#-----------------------------------------------------------------------------
my $l = 0;
while (<>)
{
    my $line = $_;
    chomp($line);
    printf "%s%03d\n", $line, $l;
    $l++;
}
