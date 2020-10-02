#!/usr/bin/perl -w
use strict;
use warnings;
use bigint;					# Ignore negative bit in 64 bit values.

$| = 1;						# Autoflush after \n stdout.
#-----------------------------------------------------------------------------
my $conn_cmd_list = $ARGV[0];			# &(struct iscsi_conn . conn_cmd_list);
my $i_conn_node = $ARGV[1];			# &(struct iscsi_cmd . i_conn_node);

#-- if (!defined($conn_cmd_list)) { $conn_cmd_list = "0xffff96f87def0388"; }
#-- if (!defined($i_conn_node))   { $i_conn_node   = "0xffff96f83ede21d8"; }
#-- if (!defined($conn_cmd_list)) { $conn_cmd_list = "0xffff92f435205388"; }
#-- if (!defined($i_conn_node))   { $i_conn_node   = "0xffff92f3fefe3f98"; }
#-- if (!defined($conn_cmd_list)) { $conn_cmd_list = "0xffff8ed8c3e2e388"; }	# conn_cmd_list
#-- if (!defined($i_conn_node))   { $i_conn_node   = "0xffff8edd037588d8"; }	# i_conn_node
#-- if (!defined($i_conn_node))   { $i_conn_node   = "0xffff9b4e7ef97398"; }	# i_conn_node
#-- if (!defined($conn_cmd_list)) { $conn_cmd_list = "0xffff9b4ebb3c1b88"; }	# conn_cmd_list
#-- if (!defined($i_conn_node))   { $i_conn_node   = "0xffff8993fea57c18"; }	# i_conn_node
#-- if (!defined($conn_cmd_list)) { $conn_cmd_list = "0xffff8994373ed388"; }	# conn_cmd_list

if (!defined($i_conn_node))   { $i_conn_node   = "0xffff97c57e5d99d8"; }	# i_conn_node
if (!defined($conn_cmd_list)) { $conn_cmd_list = "0xffff97c5b4bd1b88"; }	# conn_cmd_list

#-----------------------------------------------------------------------------
$conn_cmd_list =~ s/^0x//;			# Toss possible leading 0x.
$i_conn_node =~ s/^0x//;			# Toss possible leading 0x.

# Start to end of structure, and where in structure it is.
my $string1 = "$conn_cmd_list";
if (length($string1) != 16)
{
    printf STDERR "First argument is not 16 hex digits long - '%s' (%d)\n", $string1, length($string1);
    exit 1;
}
my $replace_string1 =  "--conn_cmd_list-";
my $string1_struct_start = hex($conn_cmd_list) - 904;				# struct iscsi_conn -0x388 offset
my $replace_string1s = "ll iscsi_conn ll";
my $string1_struct_end = $string1_struct_start + 1152;				# struct iscsi_conn  0x480 size
my $replace_range1 =   "ll 0x%08x ll";
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# Start to end of structure, and where in structure it is.
my $string2 = "$i_conn_node";
if (length($string2) != 16)
{
    printf STDERR "Second argument is not 16 hex digits long - '%s' (%d)\n", $string2, length($string2);
    exit 1;
}
my $replace_string2 =  "---i_conn_node--";
my $string2_struct_start = hex($i_conn_node) - 472;				# struct iscsi_cmd -0x1d8 offset
my $replace_string2s = "nnn iscsi_cmd nnn";

my $string2_struct_se_cmd = $string2_struct_start + 488;			# iscsi_cmd.se_cmd  0x1e8 offset
my $replace_string2se = "iscsicmd.se_cmd ";

my $string2_struct_end = $string2_struct_start + 1088;				# struct iscsi_cmd  0x440 size
my $replace_range2 =   "nn 0x%08x nn";
#-----------------------------------------------------------------------------
printf STDERR "struct iscsi_conn :  0x%016x : conn_cmd_list=0x%016x : end+1=0x%016x\n", $string1_struct_start, hex($conn_cmd_list), $string1_struct_end;
printf STDERR "struct iscsi_cmd  :  0x%016x :   i_conn_node=0x%016x : end+1=0x%016x\n", $string2_struct_start, hex($i_conn_node), $string2_struct_end;
#-----------------------------------------------------------------------------
my $last_task = '';				# Find first task and mark before it.
#-----------------------------------------------------------------------------
my $line_count = 0;

while (<>)
{
    my $line = $_;
    chomp($line);
    my $t = substr($line, 0, 5);
    if ($t =~ /^ *[0-9][0-9]*$/)
    {
	my @tl = split(/\s+/, substr($line, 6));
	my $task = $tl[3];
	if (defined($task) && $task ne $last_task)
	{
	    print STDOUT "==============================================================================\n";
	    $last_task = $task;
	}
    }

    my $lth = length($line);
    my $spaces = ' -' . (' ' x $lth);
    my $sflag = 0;				# Flag if anything put into line of spaces.
    my $w = 0;
    $line_count++;

    my @v = split(/ /, $line);			# get array of non-blanks.
    my $l = scalar(@v);				# number in array. (each space = 1)
    foreach my $v (@v)				# Go through array;
    {
	if ($v eq '')				# fast skip spaces (deleted via split).
	{
	    $w++;
	    next;
	}

	if ($v =~ /^0x/)			# Toss any leading 0x.
	{
	    $w += 2;
	    $v =~ s/^0x//;
	}

	if ($v eq $string1)			# underline for string1
	{
	    $sflag = 1;
	    substr($spaces, $w, length($replace_string1)) = $replace_string1;
	}
	elsif ($v eq $string2)			# underline for string2
	{
	    $sflag = 1;
	    substr($spaces, $w, length($replace_string2)) = $replace_string2;
	}
	else					# Now check for in enclosing array.
	{
	    my $i = hex($v);
	    if ($i == $string1_struct_start)
	    {
		$sflag = 1;
		substr($spaces, $w, length($replace_string1s)) = $replace_string1s;
	    }
	    elsif ($i > $string1_struct_start && $i < $string1_struct_end)
	    {
		$sflag = 1;
		substr($spaces, $w, 16) = sprintf($replace_range1, $i - $string1_struct_start);
	    }
	    elsif ($i == $string2_struct_start)
	    {
		$sflag = 1;
		substr($spaces, $w, length($replace_string2s)) = $replace_string2s;
	    }
	    elsif ($i == $string2_struct_se_cmd)
	    {
		$sflag = 1;
		substr($spaces, $w, length($replace_string2se)) = $replace_string2se;
	    }
	    elsif ($i > $string2_struct_start && $i < $string2_struct_end)
	    {
		$sflag = 1;
		substr($spaces, $w, 16) = sprintf($replace_range2, $i - $string2_struct_start);
	    }
	}
	$w += length($v) + 1;			# Count possible trailing space between arguments.
    }
    print STDOUT "$line\n";
    if ($sflag != 0)
    {
	print STDOUT "$spaces\n";
    }
}

print STDERR "End of file, $line_count lines.\n"; 
# printf STDERR " val1: 0x%016x - 0x%016x - 0x%016x\n", $string1_struct_start, hex($conn_cmd_list), $string1_struct_end;
# printf STDERR " val2: 0x%016x - 0x%016x - 0x%016x\n", $string2_struct_start, hex($i_conn_node), $string2_struct_end;
exit 0;
