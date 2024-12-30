#!/bin/perl -w
#====================================================================
#
# FILE NAME:    FmtCCBDump.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         6/7/02
#
# DESCRIPTION:  Formats CCB / Corelis memory dump files to their
#               equivalent text format.
#
#====================================================================

($script = $0) =~ s/^.*\\//;
unless (@ARGV >= 2) { die "\nUsage: $script CALLSTACK|SERIAL|TRACE|FR file-to-parse [fileID]\n" };
($type, $parsef, $fileID) = @ARGV;

sub dirname
{
    my ($name) = @_;
    $name =~ s/\\/\//g; # chg '\'s to '/'s
    if ($name =~ /^.*\//) {
        ($name = $&) =~ s/\/$//;
    }
    else {
        $name = ".";
    }
    return $name;
}

sub basename
{
    my ($name) = @_;
    $name =~ s/\\/\//g; # chg '\'s to '/'s
    if ($name =~ /^.*\./) {
        ($name = $&) =~ s/\.$//;
    }
    else {
        $name = "";
    }
    return $name;
}

if(!defined($fileID)) {
    $fileID = "";
}

$base = basename($parsef);
$tooldir = dirname($0);
$filedir = dirname($parsef);

$type = uc($type);
if($type !~ /^CALLSTACK$|^SERIAL$|^TRACE$|^FR$|^IOCB$/) {
    print "Don't know how to process $type type files.\n";
    exit;
}

system "$tooldir/Corelis2Bin.pl $parsef";

if($type eq "TRACE") {
    system "$tooldir/TracDec.pl $parsef-bin";
}

if($type eq "SERIAL") {
    system "$tooldir/SerDec.pl $parsef-bin";
}
if($type eq "FR") {
    system "$tooldir/FrDec.pl $parsef-bin";
}
if($type eq "IOCB") {
    system "$tooldir/IOCBDec.pl $parsef-bin";
}

if($type eq "CALLSTACK") {
    if (! -r "$filedir/CCBrun.map") {
        die "$filedir/CCBrun.map doesn't exist. Copy it in and try again.\n";
    }
        
    system "$tooldir/PCBDec.pl $parsef-bin";
}

# Rename input file to input-N.
$N = 1;
while (-r "$parsef-$N") {
    $N++;
}

# Don't die if this rename fails, its not fatal
rename "$parsef", "$parsef-$N" and print "Rename $parsef -> $parsef-$N\n";

# don't die if this unlink fails, its not fatal either
unlink "$parsef-bin";

print "Rename $parsef-bin-out -> $base$fileID.txt\n";
rename "$parsef-bin-out", "$base"."$fileID".".txt" or die;


