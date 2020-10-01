#!/usr/bin/perl -w
use strict;
use warnings;

my $save_record_separator = $/;
print STDERR "save_record_separator='$save_record_separator'\n";

my $line1 = <>;
my $line2 = <>;

print STDERR "line1='$line1'\n";
print STDERR "line2='$line2'\n";

exit 0;
