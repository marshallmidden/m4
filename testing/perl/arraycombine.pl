#!/usr/bin/perl -w
use strict;

# Combine two perl arrays.

my @array1;
my @array2;
#-----------------------------------------------------------------------------
@array1 = (3,5,1,2);
@array2 = sort @array1;
push(@array1, @array2);

printf "array1=%s\n", join(',', @array1);
printf "array2=%s\n", join(',', @array2);
#-----------------------------------------------------------------------------

exit 0;
