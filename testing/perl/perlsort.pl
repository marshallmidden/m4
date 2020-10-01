#!/usr/bin/perl -w
use strict;

my @array = (3,5,1,2);

my @sort_array = sort @array;

printf "     array=%s\n", join(',', @array);
printf "sort_array=%s\n", join(',', @sort_array);
exit 0;
