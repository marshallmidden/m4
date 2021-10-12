#!/usr/bin/perl -w
use strict;
use warnings;
# ----------------------------------------------------------------------------
# To do:
# 1) See if faster/slower notes decade at same rate.
#    See if $cc11_start = 80; $cc11_stop = 127; -- works over start to end of note.
# 2) See if faster/slower notes start (loudness) at same rate.
#    OVERLAP remain the same over speed changes.
# 3) See if loudness does decade at same rate -- and OVERLAP. (#1,#2)
# ----------------------------------------------------------------------------
my $QN = 480;                           # Length in MIDI of a quarter note.
my $NOTELTH=$QN;
my $MAXVOICES = 1;
my @NOTES = ( 60, 62, 64, 65 );

#-- my $OVERLAP = 80;
#-- my $OVERLAP = 60;
#-- my $OVERLAP = 40;
#-- my $OVERLAP = 30;
#-- my $OVERLAP = 20;
my $OVERLAP = 10;

# my $ENDTRACKS = ($NOTELTH) * ($MAXVOICES) * 8;
my $ENDTRACKS = ($NOTELTH) * ($MAXVOICES) * 12;
# ----------------------------------------------------------------------------
sub print_header($$)
{
    my ($nv, $qn) = @_;
    printf STDOUT "0, 0, Header, 1, %d, %d\n", $nv, $qn;
}   # End of print_header

# ----------------------------------------------------------------------------
sub print_track_end($$)
{
    my ($track, $endtrack) = @_;
    printf STDOUT "%d, %d, End_track\n", $track, $endtrack;
}   # End of print_track_end

# ----------------------------------------------------------------------------
sub print_track_start($$)
{
    my ($track, $endtrack) = @_;
    printf STDOUT "%d, 0, Start_track\n", $track;
    printf STDOUT "%d, 0, Tempo, 555555\n", $track;
    printf STDOUT "%d, 0, Key_signature, -1, \"major\"\n", $track;
}   # End of print_track_start

# ----------------------------------------------------------------------------
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
    printf STDOUT "%d, %d, Control_c, %d, 7, 127\n", $track, $start, $chan;     # Full volume.
}   # End of print_track

#-----------------------------------------------------------------------------
# 2, 480, Control_c, 0, 11, 96

sub print_expression($$$$)
{
    my ($track, $where, $chan, $thing) = @_;
    printf STDOUT "%d, %d, Control_c, %d, 11, %d\n", $track, $where, $chan, $thing;
}   # End of print_expression

#-----------------------------------------------------------------------------
sub print_legato($$$$$$)
{
    my ($track, $start, $stop, $chan, $p_note, $p_stop) = @_;
    my $notelth = $stop - $start + 1;
    my $cc11_start = 80;
    my $cc11_stop = 127;
    my $thing = $cc11_start;
    print_expression($track, $start, $chan, $cc11_start);
    my $x = $cc11_stop - $cc11_start;
    for (my $i = 0; $i < $notelth; $i++)
    {
        my $d = int($cc11_start + (($i / ($notelth - 1)) * ($x + 0.0)));
	if ($i < $OVERLAP)
	{
	    $d = $d - int(($OVERLAP - $i) / 4);
	}
        if ($d != $thing)
        {
            print_expression($track, $start + $i, $chan, $d);
            $thing = $d;
        }
	if ($i == $OVERLAP && $p_note > 0)
	{
	    print_note_stop($track, $p_stop + $OVERLAP, $chan, $p_note);
	}
    }
}   # End of print_legato

#-----------------------------------------------------------------------------
my $track;
my $chan = 0;
my $start = 0;
my $stop = $QN;
my $note;
my $prev_note;
my $prev_stop;
#-----------------------------------------------------------------------------
# Print header
print_header($MAXVOICES + 1, $QN);
$track = 1;
print_track_start($track, $ENDTRACKS);
print_track_end($track, $ENDTRACKS);
#-----------------------------------------------------------------------------
$track = $track + 1;
print_track_start($track, $ENDTRACKS);
print_track($track, $chan, 0, 0, $start, $stop, $note, 64);

#-----------------------------------------------------------------------------
# Do normal first.
for (my $i = 0; $i < 4; $i++)
{
    $note = $NOTES[$i];;
    print_note_start($track, $start, $chan, $note);
    print_note_stop($track, $stop, $chan, $note);
    $note = $note + 2;
    $start = $start + $NOTELTH;
    $stop = $stop + $NOTELTH;
}
#-----------------------------------------------------------------------------
$prev_note = -1;
$prev_stop = -1;

#-----------------------------------------------------------------------------
# Do legato second.
for (my $i = 0; $i < 4; $i++)
{
    $note = $NOTES[$i];;
    print_note_start($track, $start, $chan, $note);
    print_legato($track, $start, $stop, $chan, $prev_note, $prev_stop);
    $prev_note = $note;
    $prev_stop = $stop;
    $note = $note + 2;
    $start = $start + $NOTELTH;
    $stop = $stop + $NOTELTH;
}
print_note_stop($track, $prev_stop, $chan, $prev_note);
#-----------------------------------------------------------------------------
# Do third normal again.
for (my $i = 0; $i < 4; $i++)
{
    $note = $NOTES[$i];;
    print_note_start($track, $start, $chan, $note);
    print_note_stop($track, $stop, $chan, $note);
    $note = $note + 2;
    $start = $start + $NOTELTH;
    $stop = $stop + $NOTELTH;
}
#-----------------------------------------------------------------------------

print_track_end($track, $ENDTRACKS);
print STDOUT "0, 0, End_of_file\n";
exit 0;
# ----------------------------------------------------------------------------
