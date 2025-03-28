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
#
# 4) See if instrument not piano does decade at same rate -- and OVERLAP. (#1,#2)
#	Massive difference in instrument volumes, start of notes, etc.
# ----------------------------------------------------------------------------
my $QN = 480;                           # Length in MIDI of a quarter note.
# my $QN = 240;                           # Length in MIDI of a quarter note.

# my $NOTE_TIME = 1;	# quarter note
# my $NOTE_TIME = 2;	# half note
# my $NOTE_TIME = 8;	# whole note
my $NOTE_TIME = 16;	# double whole note

my $NOTELTH = $QN * $NOTE_TIME;
my $MAXVOICES = 1;
my @NOTES = ( 60, 62, 64, 65 );
my $NUM_SEQ = 4;
#++ my $NUM_SEQS = 3;
my $NUM_SEQS = 1;
my $early = $NOTELTH / 2;

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
my $instrument = 0;			# Acoustic Grand Piano
my $OVERLAP = 10;		# 0 - piano
my $cc11_start = 80;		# quarter note after quarter note.
my $cc11_stop = 127;		# quarter note

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# DB scale for double whole note:
# Starting in seconds:	?0.00195		?4.44687		?8.89204		?13.33562
#  Ramp to up seconds:	?0.1		?4.61150		?9.04170		?13.47447
#   Attack Maximum DB:  ?-12db		?-12db		?-12db		?-12db
#   Decay key release:	?2.22846		?6.62474		?11.11395	?15.55621
#        DB @ release:	?-28db		?-37db		?-35db		?-35db
# Stopping in seconds:	?2.61016		?6.96472		?11.42549	?15.89224
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# DB scale for whole note:
# Starting in seconds:	0.00195		4.44687		8.89204		13.33562
#  Ramp to up seconds:	0.1		4.61150		9.04170		13.47447
#   Attack Maximum DB:  -12db		-12db		-12db		-12db
#   Decay key release:	2.22846		6.62474		11.11395	15.55621
#        DB @ release:	-28db		-37db		-35db		-35db
# Stopping in seconds:	2.61016		6.96472		11.42549	15.89224
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# DB scale for half note (pan full left).
# Starting in seconds:	0.00195		2.22501		4.44687		6.66860
#  Ramp to up seconds:	0.1		2.38963		4.59653		6.73302
#   Attack Maximum DB:  -12db		-11db		-11db		-11db
#   Decay key release:	1.15612		3.32156		5.55082		7.77909
#        DB @ release:	-25db		-25db		-25db		-25db
# Stopping in seconds:	1.57206		3.71238		5.90725		8.19980
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# my $instrument = 40;			# Violin
# my $instrument = 41;			# Viola
# my $instrument = 42;			# Cello
# my $instrument = 43;			# Contrabass
# my $instrument = 47;			# Timpani
# my $instrument = 57;			# Trombone
# my $instrument = 60;			# French Horn
# my $instrument = 68;			# Oboe
# my $instrument = 70;			# Bassoon
# my $instrument = 71;			# Clarinet
# my $instrument = 72;			# Flute

#     'Violin'.lower() : 40,
#     'Viola'.lower() : 41,
#     'Cello'.lower() : 42,
#     'Contrabass'.lower() : 43,
#     'Tremolo Strings'.lower() : 44,
#     'Pizzicato Strings'.lower() : 45,
#     'Timpani'.lower() : 47,
#     'Trombone'.lower() : 57,
#     'French Horn'.lower() : 60,
#     'Oboe'.lower() : 68,
#     'Bassoon'.lower() : 70,
#     'Clarinet'.lower() : 71,
#     'Flute'.lower() : 73,


#-- my $OVERLAP = 80;
#-- my $OVERLAP = 60;
#-- my $OVERLAP = 40;
#-- my $OVERLAP = 30;
#-- my $OVERLAP = 20;
#-- my $OVERLAP = 10;

# my $ENDTRACKS = ($NOTELTH) * ($MAXVOICES) * 8;
my $ENDTRACKS = ($NOTELTH) * ($MAXVOICES) * ($NUM_SEQ * $NUM_SEQS);
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
#--    printf STDOUT "%d, %d, Control_c, %d, 64, 0\n", $track, $start, $chan;
    printf STDOUT "%d, %d, Control_c, %d, 0, 0\n", $track, $start, $chan;
#  91   Reverb Level - Affects: this is usually the reverb or delay level.
    printf STDOUT "%d, %d, Control_c, %d, 91, %d\n", $track, $start, $chan, $rev;  # No reverb.
#  10   Pan position   64 is center, 0 is hard left, and 127 is hard right.
#--    printf STDOUT "%d, %d, Control_c, %d, 10, %d\n", $track, $start, $chan, $pan;  # where between stereo l->r.
    printf STDOUT "%d, %d, Control_c, %d, 0, %d\n", $track, $start, $chan, $pan;  # where between stereo l->r.
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
    my $d = $cc11_stop;
    for (my $i = 0; $i < $notelth; $i++)
    {
        $d = int($cc11_start + (($i / ($notelth - 1)) * $x));
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
    if ($d != $cc11_stop)
    {
	print_expression($track, $stop, $chan, $cc11_stop);
    }
}   # End of print_legato

#-----------------------------------------------------------------------------
my $track;
my $chan = 0;
my $start = 0;
my $stop = $NOTELTH;
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
print_track($track, $chan, $instrument, 0, $start, $stop, $note, 64);

#-----------------------------------------------------------------------------
# Do normal first.
for (my $i = 0; $i < $NUM_SEQ; $i++)
{
    $note = $NOTES[$i];;
    print_note_start($track, $start, $chan, $note);
    print_note_stop($track, $stop - $early, $chan, $note);
    $note = $note + 2;
    $start = $start + $NOTELTH;
    $stop = $stop + $NOTELTH;
}
#-----------------------------------------------------------------------------
$prev_note = -1;
$prev_stop = -1;

#-----------------------------------------------------------------------------
#++ # Do legato second.
#++ for (my $i = 0; $i < $NUM_SEQ; $i++)
#++ {
#++     $note = $NOTES[$i];;
#++     print_note_start($track, $start, $chan, $note);
#++     print_legato($track, $start, $stop - $early, $chan, $prev_note, $prev_stop);
#++     $prev_note = $note;
#++     $prev_stop = $stop;
#++     $note = $note + 2;
#++     $start = $start + $NOTELTH;
#++     $stop = $stop + $NOTELTH;
#++ }
#++ print_note_stop($track, $prev_stop, $chan, $prev_note);
#++ #-----------------------------------------------------------------------------
#++ # Do third normal again.
#++ for (my $i = 0; $i < $NUM_SEQ; $i++)
#++ {
#++     $note = $NOTES[$i];;
#++     print_note_start($track, $start, $chan, $note);
#++     print_note_stop($track, $stop - $early, $chan, $note);
#++     $note = $note + 2;
#++     $start = $start + $NOTELTH;
#++     $stop = $stop + $NOTELTH;
#++ }
#-----------------------------------------------------------------------------

print_track_end($track, $ENDTRACKS);
print STDOUT "0, 0, End_of_file\n";
exit 0;
# ----------------------------------------------------------------------------
