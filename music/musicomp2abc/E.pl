#!/usr/bin/perl -w
use strict;
use warnings;

#-----------------------------------------------------------------------------
my $QN = 480;				# Length in MIDI of a quarter note.

my @instruments = (    40,3,44,3, 40,3,44,3);

#+ my @instruments = (    74,74,74,74,	# 1,2,3,4	Flute
#+ 		       69,69,69,69,	# 5,6,7,8	Oboe
#+ 		       72,72,72,72,	# 9,10,11,12	Clarinet
#+ 		       71,71,71,71,	# 13,14,15,16	Bassoon
#+ 		       61,61,61,	# 17,18,19	French Horn
#+ 		       41,41,41,	# 20,21,22	Violin
#+ 		       41,41,41,	# 23,24,25	Violin
#+ 		       42,42,42,	# 26,27,28	Viola
#+ 		       43,43,		# 29,30		Cello
#+ 		       44,44);		# 31,32		Contrabass
my $MAXVOICES = scalar(@instruments);

my $NOTELTH=$QN;
# my $RESTLTH=$QN;
my $RESTLTH=0;

my $ENDTRACKS = ($NOTELTH + $RESTLTH) * $MAXVOICES;	# End of all tracks (length in MIDI for 2 quarter notes * voices);

print STDERR "MAXVOICES=$MAXVOICES\n";

#-----------------------------------------------------------------------------
#- my $NOTE = 60;		# 3c
#- my $NOTE = 60-12;		# 2c
#- my $NOTE = 60-12-12;		# 1c
#- my $NOTE = 60-12-12 -7 ;	# 0f
# c c+/d- d d+/e- e f f+/g- g g+/a- a a+/b- b  c
# 0  1    2  3    4 5  6    7  8    9  10  11 12
#   -11 -10 -9  -8 -7 -6   -5 -4   -3  -2  -1  0
#- my @NOTE = ( 60-12-12-7, 60-12-12-12-7, 60-12-12-7, 60-12-12-12-7,  60-12-12-7, 60-12-12-12-7, 60-12-12-7, 60-12-12-12-7);
my @NOTE = ( 60-12-12-7, 60-12-12-7, 60-12-12-7, 60-12-12-7,  60-12-12-7, 60-12-12-7, 60-12-12-7, 60-12-12-7);
#-----------------------------------------------------------------------------
# Print header
printf STDOUT "0, 0, Header, 1, %d, %d\n", $MAXVOICES + 1, $QN;
print STDOUT "1, 0, Start_track\n";
print STDOUT "1, 0, Tempo, 555555\n";
print STDOUT "1, 0, Key_signature, -1, \"major\"\n";
printf STDOUT "1, %d, End_track\n", $ENDTRACKS;
#-----------------------------------------------------------------------------

my $last_instrument = 0;
my $track = 1;
my $chan = 0;
my $start = 1;
my $stop = $QN;

my $v = 1;
for my $instr (@instruments)
{
    if ($instr != $last_instrument)
    {
	$last_instrument = $instr;
	$chan = $chan + 1;
	if ($chan == 9)
	{
	    $chan = $chan + 1;
	}
    }
    $track = $track + 1;
    printf STDOUT "%d, 0, Start_track\n", $track;
    printf STDOUT "%d, 1, Program_c, %d, %d\n", $track, $chan, $last_instrument;
    printf STDOUT "%d, %d, Note_on_c, %d, %d, 127\n", $track, $start, $chan, $NOTE[$v-1];
    printf STDOUT "%d, %d, Note_off_c, %d, %d, 0\n", $track, $stop, $chan, $NOTE[$v-1];
    printf STDOUT "%d, %d, Text_t, \" --- voice %d  ---\"\n", $track, $stop, $v;
    printf STDOUT "%d, %d, End_track\n", $track, $ENDTRACKS;

    # NOTE
    $start = $start + $NOTELTH;
    $stop = $stop + $NOTELTH;
    # REST
    $start = $start + $RESTLTH;
    $stop = $stop + $RESTLTH;
    # Next voice/track.
    $v = $v + 1
}

print STDOUT "0, 0, End_of_file\n";

exit 0;
