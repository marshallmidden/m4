#!/usr/bin/perl -w
# $Id$
#
# makeinc.pl
#
# Make assembly and C include files from a common source.
#
# Copyright 2007 Xiotech Corporation. All rights reserved.
#
# Mark D. Rustad, 12/20/2007

use strict;
use Getopt::Std;
use File::Basename;

# Process command line args

our $opt_h; 
our $opt_d;
our $opt_o;
getopts('dho:');

my $script;
($script = $0) =~ s/^.*\\//;
unless (@ARGV >= 1) { die "\nUsage: ${script} [-d] [-h] [-o dir ] files...\n\n" }

if ($opt_h)
{
    print "Here is some help\n";
    exit;
}

my $file;
my $path;
my $ext;
my $lineno;


##
# get_offset returns the effective offset expression string.

sub get_offset($$)
{
    my ($offset, $offset_expr) = @_;

    if ("${offset_expr}" eq "")
    {
        return "${offset}";
    }
    return "${offset}+${offset_expr}"
}


##
# quantity returns the number in the argument, or 1 if null or undefined

sub quantity($)
{
    my $value = shift;

    if (! defined($value) || "${value}" eq "")
    {
        return 1;
    }
    return $value;
}


##
# linechk

sub linechk(*$)
{
    my $fh = $_[0];
    my $num = $_[1];

    if ($lineno != $num)
    {
        print $fh "#line ${lineno} \"${path}${file}${ext}\"\n";
        $_[1] = $lineno;
    }
}

my $parm;

foreach $parm (@ARGV)
{
    my $field_offset;
    my $field_offset_expr;
    my $mark_offset;
    my $mark_offset_expr;
    my $alines = 0;
    my $clines = 0;
    my $aneed = 1;
    my $cneed = 1;

    $lineno = 0;
    ($file, $path, $ext) = fileparse("${parm}", '\..*');
    print "parm=${parm}, file=${file}, path=${path}, ext=${ext}\n" if $opt_d;

    # Open the input file

    open IN, "<${parm}" or die "\nAbort: Can't open $parm...\n";

    if ($opt_o)
    {
        print "Changing directory to ${opt_o}\n" if $opt_d;
        chdir $opt_o;
    }

    # Open the output files

    open OUTINC, ">${file}.inc" or die "\nAbort: Can't open ${file}.inc...\n";
    open OUTH, ">${file}.h" or die "\nAbort: Can't open ${file}.h...\n";
    print "Output being written to ${file}.inc and ${file}.h...\n" if $opt_d;

    # Process the data

    while (<IN>)
    {
        ++$lineno;
        next if /^\s*#/;        # Skip comment lines.
        next if not /@/;        # Skip lines without @

        chomp;
        my ($asm, $delim1, $c, $delim2, $comment, $delim3) = split /(@)/, $_;

        $asm =~ s/\s*$//;
        $c = "" if ! defined $c;
        $c =~ s/\s*$//;
        $comment = "" if ! defined $comment;
        $comment =~ s/\s*$//;

        if (defined $asm)
        {
            my ($f1, $f2, $f3) = split " ", $asm;

            $f1 = "" if ! defined $f1;
            if ($f1 =~ /^\s*\.def$/)
            {
                $asm = ".set ${f2},${f3}";
                $c .= " ${f3}";
            }
            elsif ($f1 =~ /^\s*\.b$/)
            {
                $asm = ".set ${f2}," . get_offset($field_offset, $field_offset_expr);
                $field_offset += 1 * quantity($f3);
            }
            elsif ($f1 =~ /^\s*\.struct$/)
            {
                $field_offset = 0;
                $field_offset_expr = "";
                $mark_offset = 0;
                $field_offset_expr = "";
                $asm = "";
            }
            elsif ($f1 =~ /^\s*\.s$/)
            {
                $asm = ".set ${f2}," . get_offset($field_offset, $field_offset_expr);
                $field_offset += 2 * quantity($f3);
            }
            elsif ($f1 =~ /^\s*\.w$/)
            {
                $asm = ".set ${f2}," . get_offset($field_offset, $field_offset_expr);
                $field_offset += 4 * quantity($f3);
            }
            elsif ($f1 =~ /^\s*\.l$/)
            {
                $asm = ".set ${f2}," . get_offset($field_offset, $field_offset_expr);
                $field_offset += 8 * quantity($f3);
            }
            elsif ($f1 =~ /^\s*\.t$/)
            {
                $asm = ".set ${f2}," . get_offset($field_offset, $field_offset_expr);
                $field_offset += 12 * quantity($f3);
            }
            elsif ($f1 =~ /^\s*\.q$/)
            {
                $asm = ".set ${f2}," . get_offset($field_offset, $field_offset_expr);
                $field_offset += 16 * quantity($f3);
            }
            elsif ($f1 =~ /^\s*\.f$/)
            {
                $asm = ".set ${f2}," . get_offset($field_offset, $field_offset_expr);
                if ($f3 =~ /^\d+$/)
                {
                    $field_offset += $f3;
                }
                elsif ("${field_offset_expr}" eq "")
                {
                    $field_offset_expr = "(${f3})";
                }
                else
                {
                    $field_offset_expr .= "+(${f3})";
                }
            }
            elsif ($f1 =~ /^\s*\.m$/)
            {
                $mark_offset = $field_offset;
                $mark_offset_expr = $field_offset_expr;
                $asm = "";
            }
            elsif ($f1 =~ /^\s*\.r$/)
            {
                $field_offset = $mark_offset;
                $field_offset_expr = $mark_offset_expr;
                $asm = "";
            }
        }

        if ("${asm}" ne "")
        {
            linechk(\*OUTINC, ++$alines);
            if (defined $comment && "${comment}" ne "")
            {
                print OUTINC "${asm}\t#${comment}\n";
            }
            else
            {
                print OUTINC "${asm}\n";
            }
        }

        if ("${c}" ne "")
        {
            linechk(\*OUTH, ++$clines);
            if (defined $comment && "${comment}" ne "")
            {
                print OUTH "${c}\t//${comment}\n";
            }
            else
            {
                print OUTH "${c}\n";
            }
        }
        if (defined $comment && "${asm}" eq "" && "${c}" eq "")
        {
            linechk(\*OUTINC, ++$alines);
            print OUTINC "#${comment}\n";
            linechk(\*OUTH, ++$clines);
            print OUTH "//${comment}\n";
        }
    }

    close IN;
    close OUTINC;
    close OUTH;
}

# vi:sw=4 ts=4 expandtab
