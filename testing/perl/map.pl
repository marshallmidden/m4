#!/usr/bin/perl -w
use strict;
use warnings;

my $str = "a,b,
c,d,e,f,g,h,
i,j,q // Letras";

# $str = join "",map {s/,/:/g ;(split)[0]}  split '\n', $str;
$str = join "",map {s/,/:/g ; (split)[0] }  split '\n', $str;

print "{$str}\n";

# Sample output
# 
# {a:b:c:d:e:f:g:h:i:j:q}
