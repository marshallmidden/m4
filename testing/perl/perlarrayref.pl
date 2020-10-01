#!/usr/bin/perl -w
use strict;

sub displayResults ($$$$)
{
  my ($fru, $fruPort, $count, $aref) = @_;

  print "${$aref}[$fru][$fruPort][$count]\n";
}

my @mrc;
$mrc[1][2][3] = "hi there";

displayResults(1, 2, 3, \@mrc);
exit(0);
