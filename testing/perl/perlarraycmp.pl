#!/usr/bin/perl -w
use strict;

# I.E. It compares the scalar of the array.

my @array1;
my @array2;
#-----------------------------------------------------------------------------
@array1 = (3,5,1,2);
@array2 = sort @array1;

printf "array1=%s\n", join(',', @array1);
printf "array2=%s\n", join(',', @array2);
if (@array1 == @array2) {
    print "Arrays are equal\n";
} else {
    print "Arrays are not equal\n";
}
#-----------------------------------------------------------------------------
@array1 = (3,5,1,2);
@array2 = sort @array1;
@array1 = (3,5,1,2,7);

printf "array1=%s\n", join(',', @array1);
printf "array2=%s\n", join(',', @array2);
if (@array1 == @array2) {
    print "Arrays are equal\n";
} else {
    print "Arrays are not equal\n";
}
#-----------------------------------------------------------------------------
@array1 = (3,5,1,2);
@array2 = sort @array1;
@array1 = (3,5,1);

printf "array1=%s\n", join(',', @array1);
printf "array2=%s\n", join(',', @array2);
if (@array1 == @array2) {
    print "Arrays are equal\n";
} else {
    print "Arrays are not equal\n";
}
#-----------------------------------------------------------------------------
@array1 = (1,2,3,5);
@array2 = sort @array1;

printf "array1=%s\n", join(',', @array1);
printf "array2=%s\n", join(',', @array2);
if (@array1 == @array2) {
    print "Arrays are equal\n";
} else {
    print "Arrays are not equal\n";
}
#-----------------------------------------------------------------------------

exit 0;
