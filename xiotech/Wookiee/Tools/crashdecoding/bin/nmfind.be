#!/usr/bin/perl -w

my $BACK_NM = "$ARGV[1]";

my $arg;
my $pat;
my $patp;
my $cmd;
my $output;


$arg = $ARGV[0];
$arg =~ s/^0x0*//i;			# Delete possible leading 0x or 0X.
$patp = '^00*' . $arg;			# Pattern will be start of line with 0,
					# followed by any number of zeros,
					# followed by the number on command line.
$pat = $patp . ' . [^	]*	';	# followed by space, a character, space,
					# the "name", then tab.
$cmd = "grep '" . $pat . "' $BACK_NM";
$output = `$cmd | head -n 1`;

# print "output= ($output)\n";

$pat .= ' . ([^	]*)	';		# followed by space, a character, space,
					# the "name", then tab.
chomp($output);

# my $name1 = $output;
# $name1 =~ s/^00*$arg //;
# my $name2 = $name1;
# $name2 =~ s/^. //;
# my $name3 = $name2;
# $name3 =~ s/	.*$//;

my $name = $output;
$name =~ s/^00*$arg . ([^	]*)	.*$/$1/o;
# print "name=($name)\n";

if ($name eq '')
{
    print STDERR "$ARGV[0] has no name in Back.nm\n";
}
else
{
    print "1,\$s/$ARGV[0]/$name/\n";
#    print ":%s/$ARGV[0]/$name/\n";
}

exit 0;
