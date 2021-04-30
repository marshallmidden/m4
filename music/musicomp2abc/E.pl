#!/usr/bin/perl -w
use strict;
use warnings;
#-----------------------------------------------------------------------------
my $QN = 480;				# Length in MIDI of a quarter note.

#-- my @instruments = (    44,3,44,3, 40,3,44,3);
my @instruments = (    73, 71, 70, 68, 60, 48, 40, 41, 42, 43, 44, 45);
my @reverb      = (    52, 57, 57, 55, 62, 60, 60, 60, 60, 72, 72, 72);
my @pan = (0, 8, 16, 32, 48, 64, 80, 96, 112, 120, 127);
#-----------------------------------------------------------------------------

my %in;

my $MAXVOICES = scalar(@instruments);
my $NUM_PAN = scalar(@pan);

my $NOTELTH=$QN;
# my $RESTLTH=$QN;
#-- my $RESTLTH=0;

# End of all tracks (length in MIDI for 2 quarter notes * voices);
my $ENDTRACKS = ($NUM_PAN) * ($NOTELTH) * ($MAXVOICES);

#-----------------------------------------------------------------------------
# c c+/d- d d+/e- e f f+/g- g g+/a- a a+/b- b  c
# 0  1    2  3    4 5  6    7  8    9  10  11 12
#   -11 -10 -9  -8 -7 -6   -5 -4   -3  -2  -1  0
#-- my @NOTE = ( 60-12-12-7, 60-12-12-7, 60-12-12-7, 60-12-12-7,  60-12-12-7, 60-12-12-7, 60-12-12-7, 60-12-12-7);
my $NOTE = 60-12-12-7;
#-----------------------------------------------------------------------------
sub print_header($$)
{
    my ($nv, $qn) = @_;
    printf STDOUT "0, 0, Header, 1, %d, %d\n", $nv, $qn;
}   # End of print_header

#-----------------------------------------------------------------------------
sub print_track_end($$)
{
    my ($track, $endtrack) = @_;
    printf STDOUT "%d, %d, End_track\n", $track, $endtrack;
}   # End of print_track_end

#-----------------------------------------------------------------------------
sub print_track_start($$)
{
    my ($track, $endtrack) = @_;
    printf STDOUT "%d, 0, Start_track\n", $track;
    printf STDOUT "%d, 0, Tempo, 555555\n", $track;
    printf STDOUT "%d, 0, Key_signature, -1, \"major\"\n", $track;
}   # End of print_track_start

#-----------------------------------------------------------------------------
sub print_note_start($$$$)
{
    my ($track, $start, $chan, $note) = @_;
    printf STDOUT "%d, %d, Note_on_c, %d, %d, 127\n", $track, $start, $chan, $note;
}   # End of print_note_start

#-----------------------------------------------------------------------------
sub print_note_stop($$$$)
{
    my ($track, $stop, $chan, $note) = @_;
    printf STDOUT "%d, %d, Note_off_c, %d, %d, 0\n", $track, $stop, $chan, $note;
}   # End of print_note_stop

#-----------------------------------------------------------------------------
sub print_track($$$$$$$$)
{
    my ($track, $chan, $inst, $rev, $start, $stop, $note, $pan) = @_;
    printf STDOUT "%d, %d, Program_c, %d, %d\n", $track, $start, $chan, $inst;
    printf STDOUT "%d, %d, Control_c, %d, 121, 0\n", $track, $start, $chan;
    printf STDOUT "%d, %d, Control_c, %d, 64, 0\n", $track, $start, $chan;

#  91   Reverb Level - Affects: this is usually the reverb or delay level.
    printf STDOUT "%d, %d, Control_c, %d, 91, %d\n", $track, $start, $chan, $rev;  # No reverb.
#  10   Pan position   64 is center, 0 is hard left, and 127 is hard right.
    printf STDOUT "%d, %d, Control_c, %d, 10, %d\n", $track, $start, $chan, $pan;  # where between stereo l->r.
#   7   Volume   maximum is 127 and off.
    printf STDOUT "%d, %d, Control_c, %d, 7, 127\n", $track, $start, $chan;	# Full volume.
}   # End of print_track

#-----------------------------------------------------------------------------
my $track = 1;
#-----------------------------------------------------------------------------
# Print header
print_header($MAXVOICES + 1, $QN);
print_track_start($track, $ENDTRACKS);
print_track_end($track, $ENDTRACKS);
#-----------------------------------------------------------------------------
my $last_instrument = 0;
my $chan;
my $start = 0;
my $stop = $QN;
my $v = 0;
my $note = 60;
for (my $i=0; $i < scalar(@instruments); $i = $i + 1)
{
    $track = $track + 1;
    print_track_start($track, $ENDTRACKS);

    if ($instruments[$i] != $last_instrument)
    {
	$last_instrument = $instruments[$i];
	if (defined($in{$last_instrument}))
	{
	    $chan = $in{$last_instrument};
	}
	else
	{
	    if (defined($chan))
	    {
		$chan = $chan + 1;
		if ($chan == 9)
		{
		    $chan = $chan + 1;
		}
	    }
	    else
	    {
		$chan = 0;
	    }
	    $in{$last_instrument} = $chan;
	}
    }

    for my $p (@pan)
    {
	print_track($track, $chan, $last_instrument, $reverb[$i], $start, $stop, $note, $p);
	print_note_start($track, $start, $chan, $note);
	print_note_stop($track, $stop, $chan, $note);
	$start = $start + $NOTELTH;
	$stop = $stop + $NOTELTH;
    }
    print_track_end($track, $ENDTRACKS);

    # Next voice/track.
    $v = $v + 1
}

print STDOUT "0, 0, End_of_file\n";

exit 0;
