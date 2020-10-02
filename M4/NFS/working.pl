#!/usr/bin/perl -w
use strict;
use warnings;

#-----------------------------------------------------------------------------
my $title = shift(@ARGV);
#-----------------------------------------------------------------------------
# What desired to see is:
#         foreach vm.swappiness
#                 x = vm.vfs_cache_pressure
#                 y = free -m
#                         Mem: used
#                              free
#                              buff/cache
#                              available
#                         Swap: used
#                     numactl --hardware
#                         Node0: free
#                         Node1: free
#                     slabtop -o
#                         Active / Total Size (% used)       : 51751549.02K / 53482550.05K (96.8%)
#                                  vvvvvvvv
#                         37850292 36807384  97%    0.88K 1051397       36  33644704K xfs_inode
#
# a) New Graph:
#         vm.swappiness = #
# b) X point on graph:
#         vm.vfs_cache_pressure = #
# c) Y points on graph (newline for each):
#                                 total        used        free      shared  buff/cache   available
#                   Mem:         257603       64307       11236          10      182059      147114
#     1) mem_used M                           ^^^^^
#     2) mem_free M                                       ^^^^^
#     3) mem_avail                                                                           ^^^^^^
#     4) buff_cache                                                              ^^^^^^
#                   Swap:         31982           0       31982
#     5) swap_used                            ^^^^^
#                   node 0 free: 6380 MB
#     6) node0_free              ^^^^
#                   node 1 free: 4856 MB
#     7) node1_free              ^^^^
#                    Active / Total Size (% used)       : 51751549.02K / 53482550.05K (96.8%)
#     8) cache_total                                                     ^^^^^^^^^^^ /1024 => M
# 		      OBJS   ACTIVE  USE OBJ_SIZE   SLABS OBJ/SLAB CACHE_SIZE NAME
#                   37850292 36807384  97%    0.88K 1051397       36  33644704K xfs_inode
#     9) xfs_inode_SIZE (name from end of line)                       ^^^^^^^^ /1024 => M
#                    8481216  8265476  97%    1.00K  265038       32   8481216K kmalloc-1k
#     10) kmalloc-1k_SIZE (name from end of line)                      ^^^^^^^ /1024 => M
#-----------------------------------------------------------------------------
use constant
{
    X_page => 8.5,		# Width of page
    Y_page => 11.0,		# Height of page
    X_offset => 0.25,		# Leave this much on right & left of page.
    Y_offset => 0.25,		# Leave this much on top & bottom of page.
    X_inc => 8.0,		# Move right this much on page.
    X_size => 6.00,		# Size in inches of the graph X
#++    Y_size => 4.0,		# Size in inches of the graph Y
    Y_size => 9.25,		# Size in inches of the graph Y
#++    Y_inc => 5.25,		# Move down this much on page.

    # Curve numbers (graph "curve {int}" -- starting at 0.
    mem_used => 0,
    mem_free => 1,
    mem_avail => 2,
    buff_cache => 3,
    swap_used => 4,
    node0_free => 5,
    node1_free => 6,
    cache_total => 7,
    time_takes => 8,
    slabtops => 9,

    MINY => 0.0,
    USE_MINY => 0.7,
};
use constant Y_inc => (Y_size + Y_offset + 1.0);	# Move down this much on page.

#	Color Name		Decimal(R,G,B)	PS-rgb
use constant                   maroon	=>	(0.50,0.00,0.00);
use constant                 dark_red	=>	(0.55,0.00,0.00);
use constant                    brown	=>	(0.65,0.16,0.16);
use constant                firebrick	=>	(0.70,0.13,0.13);
use constant                  crimson	=>	(0.86,0.08,0.24);
use constant                      red	=>	(1.00,0.00,0.00);
use constant                   tomato	=>	(1.00,0.39,0.28);
use constant                    coral	=>	(1.00,0.50,0.31);
use constant               indian_red	=>	(0.80,0.36,0.36);
use constant              light_coral	=>	(0.94,0.50,0.50);
use constant              dark_salmon	=>	(0.91,0.59,0.48);
use constant                   salmon	=>	(0.98,0.50,0.45);
use constant             light_salmon	=>	(1.00,0.63,0.48);
use constant               orange_red	=>	(1.00,0.27,0.00);
use constant              dark_orange	=>	(1.00,0.55,0.00);
use constant                   orange	=>	(1.00,0.65,0.00);
use constant                     gold	=>	(1.00,0.84,0.00);
use constant          dark_golden_rod	=>	(0.72,0.53,0.04);
use constant               golden_rod	=>	(0.85,0.65,0.13);
use constant          pale_golden_rod	=>	(0.93,0.91,0.67);
use constant               dark_khaki	=>	(0.74,0.72,0.42);
use constant                    khaki	=>	(0.94,0.90,0.55);
use constant                    olive	=>	(0.50,0.50,0.00);
use constant                   yellow	=>	(1.00,1.00,0.00);
use constant             yellow_green	=>	(0.60,0.80,0.20);
use constant         dark_olive_green	=>	(0.33,0.42,0.18);
use constant               olive_drab	=>	(0.42,0.56,0.14);
use constant               lawn_green	=>	(0.49,0.99,0.00);
use constant              chart_reuse	=>	(0.50,1.00,0.00);
use constant             green_yellow	=>	(0.68,1.00,0.18);
use constant               dark_green	=>	(0.00,0.39,0.00);
use constant                    green	=>	(0.00,0.50,0.00);
use constant             forest_green	=>	(0.13,0.55,0.13);
use constant                     lime	=>	(0.00,1.00,0.00);
use constant               lime_green	=>	(0.20,0.80,0.20);
use constant              light_green	=>	(0.56,0.93,0.56);
use constant               pale_green	=>	(0.60,0.98,0.60);
use constant           dark_sea_green	=>	(0.56,0.74,0.56);
use constant      medium_spring_green	=>	(0.00,0.98,0.60);
use constant             spring_green	=>	(0.00,1.00,0.50);
use constant                sea_green	=>	(0.18,0.55,0.34);
use constant       medium_aqua_marine	=>	(0.40,0.80,0.67);
use constant         medium_sea_green	=>	(0.24,0.70,0.44);
use constant          light_sea_green	=>	(0.13,0.70,0.67);
use constant          dark_slate_gray	=>	(0.18,0.31,0.31);
use constant                     teal	=>	(0.00,0.50,0.50);
use constant                dark_cyan	=>	(0.00,0.55,0.55);
use constant                     aqua	=>	(0.00,1.00,1.00);
use constant                     cyan	=>	(0.00,1.00,1.00);
use constant               light_cyan	=>	(0.88,1.00,1.00);
use constant           dark_turquoise	=>	(0.00,0.81,0.82);
use constant                turquoise	=>	(0.25,0.88,0.82);
use constant         medium_turquoise	=>	(0.28,0.82,0.80);
use constant           pale_turquoise	=>	(0.69,0.93,0.93);
use constant              aqua_marine	=>	(0.50,1.00,0.83);
use constant              powder_blue	=>	(0.69,0.88,0.90);
use constant               cadet_blue	=>	(0.37,0.62,0.63);
use constant               steel_blue	=>	(0.27,0.51,0.71);
use constant         corn_flower_blue	=>	(0.39,0.58,0.93);
use constant            deep_sky_blue	=>	(0.00,0.75,1.00);
use constant              dodger_blue	=>	(0.12,0.56,1.00);
use constant               light_blue	=>	(0.68,0.85,0.90);
use constant                 sky_blue	=>	(0.53,0.81,0.92);
use constant           light_sky_blue	=>	(0.53,0.81,0.98);
use constant            midnight_blue	=>	(0.10,0.10,0.44);
use constant                     navy	=>	(0.00,0.00,0.50);
use constant                dark_blue	=>	(0.00,0.00,0.55);
use constant              medium_blue	=>	(0.00,0.00,0.80);
use constant                     blue	=>	(0.00,0.00,1.00);
use constant               royal_blue	=>	(0.25,0.41,0.88);
use constant              blue_violet	=>	(0.54,0.17,0.89);
use constant                   indigo	=>	(0.29,0.00,0.51);
use constant          dark_slate_blue	=>	(0.28,0.24,0.55);
use constant               slate_blue	=>	(0.42,0.35,0.80);
use constant        medium_slate_blue	=>	(0.48,0.41,0.93);
use constant            medium_purple	=>	(0.58,0.44,0.86);
use constant             dark_magenta	=>	(0.55,0.00,0.55);
use constant              dark_violet	=>	(0.58,0.00,0.83);
use constant              dark_orchid	=>	(0.60,0.20,0.80);
use constant            medium_orchid	=>	(0.73,0.33,0.83);
use constant                   purple	=>	(0.50,0.00,0.50);
use constant                  thistle	=>	(0.85,0.75,0.85);
use constant                     plum	=>	(0.87,0.63,0.87);
use constant                   violet	=>	(0.93,0.51,0.93);
use constant       	      magenta	=>	(1.00,0.00,1.00);
use constant                  fuchsia	=>	(1.00,0.00,1.00);
use constant                   orchid	=>	(0.85,0.44,0.84);
use constant        medium_violet_red	=>	(0.78,0.08,0.52);
use constant          pale_violet_red	=>	(0.86,0.44,0.58);
use constant                deep_pink	=>	(1.00,0.08,0.58);
use constant                 hot_pink	=>	(1.00,0.41,0.71);
use constant               light_pink	=>	(1.00,0.71,0.76);
use constant                     pink	=>	(1.00,0.75,0.80);
use constant            antique_white	=>	(0.98,0.92,0.84);
use constant                    beige	=>	(0.96,0.96,0.86);
use constant                   bisque	=>	(1.00,0.89,0.77);
use constant          blanched_almond	=>	(1.00,0.92,0.80);
use constant                    wheat	=>	(0.96,0.87,0.70);
use constant                corn_silk	=>	(1.00,0.97,0.86);
use constant            lemon_chiffon	=>	(1.00,0.98,0.80);
use constant  light_golden_rod_yellow	=>	(0.98,0.98,0.82);
use constant             light_yellow	=>	(1.00,1.00,0.88);
use constant             saddle_brown	=>	(0.55,0.27,0.07);
use constant                   sienna	=>	(0.63,0.32,0.18);
use constant                chocolate	=>	(0.82,0.41,0.12);
use constant                     peru	=>	(0.80,0.52,0.25);
use constant              sandy_brown	=>	(0.96,0.64,0.38);
use constant               burly_wood	=>	(0.87,0.72,0.53);
use constant                      tan	=>	(0.82,0.71,0.55);
use constant               rosy_brown	=>	(0.74,0.56,0.56);
use constant                 moccasin	=>	(1.00,0.89,0.71);
use constant             navajo_white	=>	(1.00,0.87,0.68);
use constant               peach_puff	=>	(1.00,0.85,0.73);
use constant               misty_rose	=>	(1.00,0.89,0.88);
use constant           lavender_blush	=>	(1.00,0.94,0.96);
use constant                    linen	=>	(0.98,0.94,0.90);
use constant                 old_lace	=>	(0.99,0.96,0.90);
use constant              papaya_whip	=>	(1.00,0.94,0.84);
use constant                sea_shell	=>	(1.00,0.96,0.93);
use constant               mint_cream	=>	(0.96,1.00,0.98);
use constant               slate_gray	=>	(0.44,0.50,0.56);
use constant         light_slate_gray	=>	(0.47,0.53,0.60);
use constant         light_steel_blue	=>	(0.69,0.77,0.87);
use constant                 lavender	=>	(0.90,0.90,0.98);
use constant             floral_white	=>	(1.00,0.98,0.94);
use constant               alice_blue	=>	(0.94,0.97,1.00);
use constant              ghost_white	=>	(0.97,0.97,1.00);
use constant                 honeydew	=>	(0.94,1.00,0.94);
use constant                    ivory	=>	(1.00,1.00,0.94);
use constant                    azure	=>	(0.94,1.00,1.00);
use constant                     snow	=>	(1.00,0.98,0.98);
use constant                    black	=>	(0.00,0.00,0.00);
use constant                 dim_gray	=>	(0.41,0.41,0.41);
use constant                 dim_grey	=>	(0.41,0.41,0.41);
use constant                     gray	=>	(0.50,0.50,0.50);
use constant                     grey	=>	(0.50,0.50,0.50);
use constant                dark_gray	=>	(0.66,0.66,0.66);
use constant                dark_grey	=>	(0.66,0.66,0.66);
use constant                   silver	=>	(0.75,0.75,0.75);
use constant               light_gray	=>	(0.83,0.83,0.83);
use constant               light_grey	=>	(0.83,0.83,0.83);
use constant                gainsboro	=>	(0.86,0.86,0.86);
use constant              white_smoke	=>	(0.96,0.96,0.96);
use constant                    white	=>	(1.00,1.00,1.00);

#-----------------------------------------------------------------------------
my @slabcolors = ((brown), (dark_green), (black), (yellow), (sandy_brown), (tan), (light_pink));
#-----------------------------------------------------------------------------
my $x_start = X_offset;
my $y_start = Y_page - Y_offset - Y_inc;
#-----------------------------------------------------------------------------
my $x_t = $x_start;		# X Translation on page in inches.
my $y_t = $y_start;		# Y Translation on page in inches.
my $newpage = 0;		# 1 means put out a newpage directive.
my $pts = 1;			# flag pts in curve printed out.
my $slabtop = 0;		# for multiple slabtop output lines.
my @slabs;			# names of curves. Index=curve number.
my $graph = 0;
my $maxy = 0;			# max y value for all graphs.
my $miny = 8589934591;		# min y value for all graphs.
my $maxx = 0;			# max x value for all graphs.
my $minx = 8589934591;		# min x value for all graphs.

my @g_p;			# graphing pre-stuff.
my @g_m;			# graphing middle-stuff.
my $g = 0;			# index for above 3 items.

my $mem_cnt = 0;
my $swap_cnt = 0;
my $node0_cnt = 0;
my $node1_cnt = 0;
my $cb_cnt = 0;
my $first_time;			# seconds first time through.
my $time_cnt = 0;
my @slab_cnt;
my @pts_x;
my @pts_y;

my $x = 0;			# vm.vfs_cache_pressure value.

#-----------------------------------------------------------------------------
# Get rid of all newlines, carriage returns, and tabs/spaces at end of lines.
sub chompit($)
{
    my $c;
    my $wp = $_[0];
    my $w;

    if ($wp eq '')
    {
	return '';
    }

    do
    {
	$w = $wp;
	$c = chop($wp);
	if ($c ne "\n" && $c ne "\r" && $c ne " " && $c ne "\t")
	{
	    return $w;
	}
    } while $wp ne '';
    return $wp;
}   # end of chompit

#-----------------------------------------------------------------------------
# Process line 'vm.swappiness = '
sub vm_swappiness($)
{
    my $value = $_[0];

    if ($pts == 0)
    {
	$g_m[$g] .= sprintf "  curve %d linetype none color %1.2f %1.2f %1.2f pts 100 100 200 200\n", $g, (white);
    }

    $g = $graph;
    $g_p[$g] = "";
    $g_m[$g] = "";

    $graph++;			# next one is a new graph.
    if ($newpage == 1)
    {
	$g_p[$g] .= sprintf "\n";
	$g_p[$g] .= sprintf "newpage\n";
	$newpage = 0;		# No longer need to put out a newpage directive.
    }

    # Position graph on page.
    $g_p[$g] .= sprintf "\n";
    $g_p[$g] .= sprintf "graph %d x_translate %2.2f y_translate %2.2f\n", $graph, $x_t, $y_t;

    # The title for the graph.
    $g_p[$g] .= sprintf "  title fontsize 16 : $title vm.swappiness = %d\n", $value;

    $g_p[$g] .= sprintf "  curve %d linetype solid color %1.2f %1.2f %1.2f label : %s\n",
    				mem_used, (red), "Memory Used";
    $g_p[$g] .= sprintf "  curve %d linetype solid color %1.2f %1.2f %1.2f label : %s\n",
    				mem_free, (green), "Memory Free";
    $g_p[$g] .= sprintf "  curve %d linetype solid color %1.2f %1.2f %1.2f label : %s\n",
    				mem_avail, (dark_blue), "Memory Available";
    $g_p[$g] .= sprintf "  curve %d linetype solid color %1.2f %1.2f %1.2f label : %s\n",
    				buff_cache, (light_blue), "Buff/Cache";
    $g_p[$g] .= sprintf "  curve %d linetype solid color %1.2f %1.2f %1.2f label : %s\n",
    				swap_used, (magenta), "Swap Used";
    $g_p[$g] .= sprintf "  curve %d linetype solid color %1.2f %1.2f %1.2f label : %s\n",
    				node0_free, (black), "Node 0 Free";
    $g_p[$g] .= sprintf "  curve %d linetype solid color %1.2f %1.2f %1.2f label : %s\n",
    				node1_free, (orange), "Node 1 Free";
    $g_p[$g] .= sprintf "  curve %d linetype solid color %1.2f %1.2f %1.2f label : %s\n",
    				cache_total, (purple), "Cache Total";
    $g_p[$g] .= sprintf "  curve %d linetype solid color %1.2f %1.2f %1.2f label : %s\n",
    				time_takes, (turquoise), "Time till Stopped";

    for (my $i = 0; $i < $slabtop; $i++)
    {
	my $j = $i * 3;
	$g_p[$g] .= sprintf "  curve %d linetype solid color %2.2f %2.2f %2.2f label : %s\n",
		   slabtops + $i, $slabcolors[$j], $slabcolors[$j+1], $slabcolors[$j+2], $slabs[$i];
    }

    # new graph, start counters over again (for X and Y points).
    $pts = 0;
#--    $slabtop = 0;
#--    undef(@slabs);
    $mem_cnt = 0;		# There are two entries - before running, and after running.
    $swap_cnt = 0;
    $node0_cnt = 0;
    $node1_cnt = 0;
    $cb_cnt = 0;
    $time_cnt = 0;
#--    undef(@slab_cnt);
#=    undef(@pts_x);
#=    undef(@pts_y);

    if (($x_t + X_inc + X_offset) >= X_page)
    {
	$x_t = $x_start;	# Start X over.
	if (($y_t - Y_inc - Y_offset) < 0)
	{
	    $y_t = $y_start;	# Start Y over.
	    $newpage = 1;	# Need a newpage directive for next graph.
	}
	else
	{
	    $y_t -= Y_inc;
	}
    }
    else
    {
	$x_t += X_inc;
    }
}   # end of vm_swappiness

#-----------------------------------------------------------------------------
# Process line 'vm.vfs_cache_pressure = '
sub vm_vfs_cache_pressure($)
{
    my $value = $_[0];
    $x = $value;			# vm.vfs_cache_pressure value.
    if ($value > $maxx) { $maxx = $value; }
    if ($value < $minx) { $minx = $value; }
}   # end of vm_vfs_cache_pressure

#-----------------------------------------------------------------------------
sub save_value($$$)
{
    my ($value, $curve, $where) = @_;

    if ($value <= MINY) { $value = USE_MINY; }
    if ($value > $maxy) { $maxy = $value; }
    if ($value < $miny) { $miny = $value; }
    $pts_x[$g][$curve][$where] = $x;
    $pts_y[$g][$curve][$where] = $value;
}   # end of save_value

#-----------------------------------------------------------------------------
sub mem($$$$)
{
    my ($used, $free, $buffcache, $avail) = @_;

    save_value($used, mem_used, $mem_cnt);
    save_value($free, mem_free, $mem_cnt);
    save_value($buffcache, buff_cache, $mem_cnt);
    save_value($avail, mem_avail, $mem_cnt);
    $mem_cnt++;			# Ready for data from after running, etc.
}   # end of mem

#-----------------------------------------------------------------------------
sub swap($)
{
    my ($used) = @_;

    save_value($used, swap_used, $swap_cnt);
    $swap_cnt++;			# Ready for data from after running, etc.
}   # end of swap

#-----------------------------------------------------------------------------
sub node0free($)
{
    my ($free) = @_;

    save_value($free, node0_free, $node0_cnt);
    $node0_cnt++;			# Ready for data from after running, etc.
}   # end of node0free

#-----------------------------------------------------------------------------
sub node1free($)
{
    my ($free) = @_;

    save_value($free, node1_free, $node1_cnt);
    $node1_cnt++;			# Ready for data from after running, etc.
}   # end of node1free

#-----------------------------------------------------------------------------
sub cachetotal($)
{
    my ($used) = @_;

    $used = $used / 1024;		# convert kilobytes to megabytes
    save_value($used, cache_total, $cb_cnt);
    $cb_cnt++;				# Ready for data from after running, etc.
}   # end of cachetotal

#-----------------------------------------------------------------------------
sub timedrun($)
{
    my ($sec) = @_;

    if (!defined($first_time))
    {
	$first_time = $sec;
	return;
    }
    save_value($sec - $first_time, time_takes, $time_cnt);
    $time_cnt++;			# Ready for data from after running, etc.
    undef($first_time);
}   # end of timedrun

#-----------------------------------------------------------------------------
sub slabtop($$)
{
    my ($size, $name) = @_;

    $size = $size / 1024;		# convert kilobytes to megabytes.
#+    if ($size <= MINY) { $size = USE_MINY; }
#+    if ($size > $maxy) { $maxy = $size; }
#+    if ($size > MINY && $size < $miny) { $miny = $size; }
    for (my $i = 0; $i < $slabtop; $i++)
    {
	if ($name eq $slabs[$i])
	{
#+	    my $j = $i * 3;
#+	    $pts_x[$g][slabtops + $i][$slab_cnt[$i]] = $x;
#+	    $pts_y[$g][slabtops + $i][$slab_cnt[$i]] = $size;
	    save_value($size, slabtops + $i, $slab_cnt[$i]);
	    $slab_cnt[$i]++;
	    return;
	}
    }

    $slabs[$slabtop] = $name;
    my $j = $slabtop * 3;
    $g_p[$g] .= sprintf "  curve %d linetype solid color %2.2f %2.2f %2.2f label : %s\n",
	       slabtops + $slabtop, $slabcolors[$j], $slabcolors[$j+1], $slabcolors[$j+2], $name;
    if (defined($pts_x[$g][slabtops + $slabtop][0]))
    {
	printf STDERR "ERROR, already defined slabtop(%d) value x(%1.2f) size(%1.2f)\n", $slabtop, $x, $size;
	exit 1;
    }
    $slab_cnt[$slabtop] = 0;
#+    $pts_x[$g][slabtops + $slabtop][$slab_cnt[$slabtop]] = $x;
#+    $pts_y[$g][slabtops + $slabtop][$slab_cnt[$slabtop]] = $size;
    save_value($size, slabtops + $slabtop, $slab_cnt[$slabtop]);
    $slab_cnt[$slabtop]++;
    $slabtop++;
}   # end of slabtop

#-----------------------------------------------------------------------------
use Time::Local;

my %Month;
$Month{"Jan"} = 0;
$Month{"Feb"} = 1;
$Month{"Mar"} = 2;
$Month{"Apr"} = 3;
$Month{"May"} = 4;
$Month{"Jun"} = 5;
$Month{"Jul"} = 6;
$Month{"Aug"} = 7;
$Month{"Sep"} = 8;
$Month{"Oct"} = 9;
$Month{"Nov"} = 10;
$Month{"Dec"} = 11;

#-----------------------------------------------------------------------------
sub gettime($)
{
    my $str = $_[0];

    if (!($str =~ /^[^ ]* ([^ ]*)  *([0-9][0-9]*)  *([0-9][0-9]*):([0-9][0-9]*):([0-9][0-9]*)  *[a-zA-Z]*  *([0-9][0-9]*)$/))
    {
	return undef;
    }
    my ($mon, $mday, $hours, $min, $sec, $year) = ($1, $2, $3, $4, $5, $6);
# print STDERR "input='$str'  => $mon $mday $year   $hours:$min:$sec\n";
    $mon = $Month{$mon};

    my $seconds = timegm($sec, $min, $hours, $mday, $mon, $year);
# print STDERR "input='$str'  => $mon $mday $year   $hours:$min:$sec    seconds=$seconds\n";
    return $seconds;
}   # end of gettime

#-----------------------------------------------------------------------------
sub donegraphs()
{
    # Generate y_epts or pts for all curves on graph.
    for (my $i = 0; $i < $graph; $i++)	# Left index of 2 dimensional array.
    {
	if (!defined($pts_x[$i]))
	{
	    next;
	}
	for (my $j = 0; $j < @{$pts_x[$i]}; $j++)	# Left index of 2 dimensional array.
	{
	    if (!defined($pts_x[$i][$j]))
	    {
		next;
	    }
	    my $k = @{$pts_x[$i][$j]}; 		# Limit on right index of array.
	    my $e_or_p = 0;			# No last y_epts (2), nor pts (1).
	    my $last_x = -1;		# No last x value.
	    my $first = 0;			# newline before "pts" or "y_epts"

	    $g_m[$i] .= sprintf "  curve %d\n", $j;
	    for (my $l = 0; $l < $k; $l++)	# Go through right index.
	    {
		if (defined($pts_x[$i][$j][$l]))
		{
		    my $x = $pts_x[$i][$j][$l];
		    my $y = $pts_y[$i][$j][$l];	# Assume no coding problems that make this undefined.

		    if (defined($pts_x[$i][$j][$l+1]) && $pts_x[$i][$j][$l+1] == $x)
		    {
			my $ny = $pts_y[$i][$j][$l+1];
			# If y_epts to be used.
			if ($last_x != 2)
			{
			    if ($first != 0)
			    {
				$g_m[$i] .= sprintf "\n",
			    }
			    $g_m[$i] .= sprintf "  y_epts",
			    $last_x = 2;	# In middle of y_epts now.
			}
			$g_m[$i] .= sprintf "  %1.2f %1.2f %1.2f %1.2f", $x, $ny, $y, $ny;
			$l++;		# Skip this and the next one.
		    }
		    else	
		    {
			# If pts to be used.
			if ($last_x != 1)
			{
			    if ($first != 0)
			    {
				$g_m[$i] .= sprintf "\n",
			    }
			    $g_m[$i] .= sprintf "  pts";
			    $last_x = 1;	# In middle of pts now.
			}
			$g_m[$i] .= sprintf "  %1.2f %1.2f", $x, $y;
		    }
		    $first++;
		}
		else
		{
		    $last_x = -1;		# skipped x value.
		}
	    }
	    if ($first != 0)
	    {
		$g_m[$i] .= sprintf "\n",
	    }
	}
    }

    # Print out the pre-graph info (g_p) followed by points (g_m).
    for (my $i = 0; $i < $graph; $i++)
    {
	$g_p[$i] .= sprintf "  xaxis size %2.2f log\n", X_size;
	$g_p[$i] .= sprintf "    label fontsize 12 : vm.vfs_cache_pressure\n";
	$g_p[$i] .= sprintf "    min %2.2f max %2.2f\n", $minx, $maxx;
	$g_p[$i] .= sprintf "    hash_label at %2.2f : %2d\n", $minx, $minx;
	$g_p[$i] .= sprintf "    hash_label at %2.2f : %2d\n", $maxx, $maxx;

	$g_p[$i] .= sprintf "  yaxis size %2.2f log log_base 2 mhash 8\n", Y_size;
	$g_p[$i] .= sprintf "    label fontsize 12 : Megabytes of memory\n";
	$g_p[$i] .= sprintf "    min %2.2f max %2.2f\n", $miny, $maxy;
	$g_p[$i] .= sprintf "    hash_label at %2.2f : %2d\n", $miny, $miny;
	$g_p[$i] .= sprintf "    hash_label at %2.2f : %2d\n", $maxy, $maxy;

	printf "%s%s", $g_p[$i], $g_m[$i];
    }
}   # end of donegraphs

#-----------------------------------------------------------------------------
# Main program follows.
my $line;
#-----------------------------------------------------------------------------
while ($line = <>)
{
    if ($line =~ /The defaults are:/)
    {
       # Ignore the next 8 lines.
       for (my $i = 0; $i < 9; $i++)
	   { $line = <>; }
    }
    if ($line =~ /message_limiting/ ||
        $line =~ /Starting read threads\.\.\./ ||
	$line =~ /Wait for threads to finish their tasks.../ ||
	$line =~ /user: .* ms  sys: .* ms  real time: .* seconds/ ||
	$line =~ /First time fills cache, etc\./ ||
	$line =~ /vm.drop_caches = / ||
	$line =~ /forks\/sec 7 Seconds=/ ||
	$line =~ /----------------------------------------------/ ||
	$line =~ /Do it the second time, as cache, etc\. exists\./ ||
	$line =~ /^swapoff -a/ ||
	$line =~ /^swapon -a/ ||
	$line =~ /^sleep / ||
	$line =~ /^forks\/sec / ||

	$line =~ /^Start two large programs/ ||
	$line =~ /^large[(]s[)] started, sleep/ ||
	$line =~ /^Size = / ||
	$line =~ /^Starting write loop/ ||
	$line =~ /^start find / ||
	$line =~ /^find \// ||
	$line =~ /^wait / ||
	$line =~ /^Done with first loop / ||
	$line =~ /^kill large program/ ||
	$line =~ /^SWAP USED: / ||
	$line =~ /^\.\/T: line / ||
	$line =~ /^Done with \.\/T/ ||
	$line =~ / no process found/ ||

	$line =~ /^Area/ ||
	$line =~ /^firmware\/hardware/ ||
	$line =~ /^kernel image/ ||
	$line =~ /^kernel dynamic memory/ ||
	$line =~ /^free memory/ ||
	$line =~ /^userspace memory/ ||

	$line =~ /^How Run Did:/ ||
	$line =~ /kill:.*No such process/ ||
	$line =~ /Terminated.*large/ ||
	$line =~ /Terminated.*find/ ||
	$line =~ /^DONE with /
	)
    {
	next;
    }

    $line = chompit($line);
    if ($line =~ /^$/)
    {
	next;
    }

    # Process lines of importance.
    if ($line =~ /vm.swappiness = ([0-9][0-9]*)$/)
    {
	vm_swappiness($1);
    }
    elsif ($line =~ /vm.vfs_cache_pressure = ([0-9][0-9]*)$/)
    {
	vm_vfs_cache_pressure($1);
    }
    elsif ($line =~ /^Mem: *[0-9][0-9]*  *([0-9][0-9]*)  *([0-9][0-9]*)  *[0-9][0-9]*  *([0-9][0-9]*)  *([0-9][0-9]*) *$/)
    {
    	# $1 = used
    	# $2 = free
    	# $3 = buff/cache
    	# $4 = available
	mem($1, $2, $3, $4);
	$pts++;
    }
    elsif ($line =~ /^Swap: *[0-9][0-9]*  *([0-9][0-9]*)  *[0-9][0-9]* *$/)
    {
	swap($1);
	$pts++;
    }
    elsif ($line =~ /^node 0 free: *([0-9][0-9]*) MB *$/)
    {
	node0free($1);
	$pts++;
    }
    elsif ($line =~ /^node 1 free: *([0-9][0-9]*) MB *$/)
    {
	node1free($1);
	$pts++;
    }
    elsif ($line =~ /^ *Active \/ Total Size \(% used\)  *:  *[0-9.][0-9.]*[KMGkmg] \/ ([0-9.][0-9.]*)[KMGkmg].*$/)
    {
	cachetotal($1);
	$pts++;
    }
    #                    OBJS          ACTIVE        USE             OBJ_SIZE     SLABS         OBJ/SLAB     CACHE_SIZE    NAME
    elsif ($line =~ /^ *[0-9.][0-9.]*  *[0-9.][0-9.]*  *[0-9.][0-9.]*%  *[0-9.][0-9.]*[KMGkmg]  *[0-9.][0-9.]*  *[0-9.][0-9.]*  *([0-9.][0-9.]*)[KMGkmg]  *(.*)$/)
    {
    # slabtop_1 => 8,
    # slabtop_2 => 9,
    	# $1 = Cache_size
    	# $2 = Cache_name
	slabtop($1, $2);
	$pts++;
    }
    else
    {
	my $ret = gettime($line);
	if (defined($ret))
	{
# NOTDONEYET
	    timedrun($ret);
	}
	else
	{
	    $g_m[$g] .= sprintf "(* notdoneyet: %s *)\n", $line;
	}
    }
}   # End of main loop.

donegraphs();			# Put out last yaxis max for each graph.

#-----------------------------------------------------------------------------
# End of file working.pl
