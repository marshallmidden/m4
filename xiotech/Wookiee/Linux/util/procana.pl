#!/usr/bin/perl -w
# $Id: procana.pl 89676 2009-06-19 14:49:32Z mdr $

###
# procana.pl - Analyze proc crashes by adding symbolic information.
#
# Mark Rustad, 2005/04/13
#
# Copyright (c) 2005 Xiotech Corporation. All rights reserved.
#
# With much thanks to Gene Olson for providing elf2kme as a starting point.

## Use "perldoc crashana.pl" to see documentation

=head1 NAME

procana.pl - Analyze proc crashes by adding symbolic information

=head1 SYNOPSIS

procana.pl [-d] elffile crashfile ...

=head1 OPTIONS

-d          Print debugging info.

elffile     Elf file with debugging symbols.

crashfile   File(s) each holding one or more crash records.

=head1 DESCRIPTION

procana.pl reads symbolic information from an elf file and then
applies that information to stack dumps contained in a crash file.

=head1 BUGS

The program does not know if the given elf file goes with the crash
file that it is given. The results of such a mistake could look
incredibly weird.

=head1 SEE ALSO

readelf(1)

=cut

use strict;
use integer;

## Command line options

my $debug = 0;          # Debug level
my $verbose = 0;        # Verbosity
my $do_typedefs = 0;    # Doing typedefs?

## Symbol table

my %symbol;             # All symbols hashed by readelf(1) "key"

my @sections;           # List of sections


## list of subprogram keys

my @subprograms;

## Some globals for stack access

my @stack;
my $start = 0;
my $end = 0;
my $stacktype = 0;


## get_stack - Access the stack, providing 0 as needed.

sub get_stack($)
{
    my $index = shift;

    return $stack[$index - $start] if (defined($stack[$index - $start]));
    return 0;
}


###
# print_sym - print a symbol

sub print_sym($)
{
    my $key = shift;

    return if !defined $key;

    my $sep = "";

    print "\$symbol[$key] = {";

    for my $v (sort keys %{$symbol{$key}})
    {
        print "$sep$v=" . $symbol{$key}{$v};
        $sep = ", ";
    }

    print "}\n";
}


###
# get_pc_key - Get key to function corresponding to a pc location.

sub get_pc_key($)
{
    my $addr = shift;
    my $key;

    foreach $key (@subprograms)
    {
        my $sym;
        my ($low, $high);

        $sym = $symbol{$key};

        if (exists($sym->{low_pc}) && exists($sym->{high_pc}))
        {
            $low = $sym->{low_pc};
            $high = $sym->{high_pc};
            return $key if ($addr >= $low && $addr <= $high);
        }
    }
    return 0;
}


##
# addr2nearest - Get nearest name from address.

sub addr2nearest($)
{
    my $addr = shift;
    my $sectdiff;

    foreach my $sect (@sections)
    {
        next if (!exists($sect->{addr}));
        next if ($addr < $sect->{addr});
        $sectdiff = $addr - $sect->{addr};
        next if ($sectdiff > $sect->{size});

        my $close = 1000000;
        my $symv = 0;
        foreach my $sym (@{$sect->{symbols}})
        {
            next if (!exists($sym->{addr}));

            my $diff = $addr - $sym->{addr};
            next if ($diff lt 0);
            next if ($sym->{name} =~ /^ct_autolabel_/);

            if ($diff < $close)
            {
                $close = $diff;
                $symv = $sym;
            }
            next;
        }
        if ($symv ne 0)
        {
            return sprintf "%s+0x%x in %s",
                $symv->{name}, $close, $sect->{name};
        }
        return sprintf "+0x%x in %s", $sectdiff, $sect->{name};
    }
    return "\?\?";
}


sub get_name($)
{
    my $key = shift;

    return $symbol{$key}->{name} if (exists $symbol{$key}->{name});
    return "\?\?";
}


###
# Parse elf file

sub read_elf($)
{
    my $file = shift;
    my $key;
    my $sym = {};
    my $func;
    my $struct;
    my $array;
    my $newfile = 0;
    my $tag;
    my $filekey;

    open(ELFFILE, "readelf -wi $file |") or die "Cannot open file: $file";

    # Read the readelf output

    loop: while (<ELFFILE>)
    {
        if (m/^\s*<(\w+)><(\w+)>:.*Number:\s*(\w+)\s*\(DW_TAG_(\w+)\)/i)
        {
            print_sym($key) if $debug >= 3;
            $key = $2;
            $tag = $4;

            warn "Duplicate symbol $2" if (defined($symbol{$key}));

            if ($tag eq "compile_unit")
            {
                $newfile = 1;
                $filekey = $key;
                $symbol{$key} = $sym = { tag => $tag };
                next;
            }
            $newfile = 0;

            $symbol{$key} = $sym = { tag => $tag, file => $filekey };
            if ($tag eq "subprogram" || $tag eq "inlined_subroutine")
            {
                $func = $key;
                push @subprograms, $key;
                next;
            }
#            if ($tag eq "structure_type" || $tag eq "union_type")
#            {
#                $struct = $key;
#                next;
#            }
#            if ($tag eq "array_type")
#            {
#                $array = $key;
#                next;
#            }
#            if ($tag eq "member")
#            {
#                push @{$symbol{$struct}->{member}}, $key;
#                next;
#            }
#            if ($tag eq "subrange_type")
#            {
#                push @{$symbol{$array}->{subrange}}, $key;
#                next;
#            }
            if ($tag eq "formal_parameter")
            {
                push @{$symbol{$func}->{parameters}}, $key;
                next;
            }
            while (<ELFFILE>)
            {
                redo loop if (m/\s*</);
            }
            next;
        }
        #if (! exists $sym->{tag}) { next; }
        if (m/^\s*DW_AT_(\w+)\s*:\s*(.*?)\s*$/)
        {
            my $item = $1;
            my $body = $2;

            if ($item eq "name")
            {
                # Struct/union/typedef/function/file/formal parameter name
                if ($body =~ m/([^ :][^:]*?)$/)
                {
                    print "Processing $1\n" if ($verbose && $newfile eq 1);
                    $sym->{name} = $1;
                } else
                {
                    warn "Unparseable $item $key $body";
                }

#                if ($newfile eq 0)
#                {
#                    if ($sym->{tag} eq "subprogram" ||
#                        $sym->{tag} eq "inlined_subroutine" ||
#                        $sym->{tag} eq "structure_type" ||
#                        $sym->{tag} eq "union_type" ||
#                        ($sym->{tag} eq "typedef" && $do_typedefs &&
#                        !defined $generate{$sym->{name}}))
#                    {
#                        $generate{$sym->{name}} = $key;
#                    }
#                }
                next;
            }
#            if ($item eq "type")
#            {
#                # Points to another symbol
#                if ($body =~ /^<(\w+)>$/)
#                {
#                    $sym->{$item} = $1;
#                } else
#                {
#                    warn "Unparseable: $item $key $body";
#                }
#                next;
#            }
#            if ($item eq "encoding")
#            {
#                # Basic encoding: char, int, long, etc.
#                if ($body =~ /\(([a-z_ ]+)\)/)
#                {
#                    $sym->{$item} = $1;
#                } else
#                {
#                    warn "Unparseable: $item $key $body";
#                }
#                next;
#            }
#            if ($item eq "byte_size" || $item eq "upper_bound")
#            {
#                # Symbol size or array element size
#                if ($body =~ /^(\w+)$/)
#                {
#                    $sym->{$item} = $1;
#                } else
#                {
#                    warn "Unparseable: $item $key $body";
#                }
#                next;
#            }
#            if ($item eq "data_member_location")
#            {
#                # Structure offset
#                if ($body =~ /plus_uconst:\s*(\w+)\)/)
#                {
#                    $sym->{offset} = $1;
#                } else
#                {
#                    warn "Unparseable: $item $key $body";
#                }
#                next;
#            }
            if ($item eq "low_pc" || $item eq "high_pc")
            {
                # Code range
                if ($body =~ /^\s*(\w+)/)
                {
                    $sym->{$item} = oct $1;
                } else
                {
                    warn "Unparseable: $item $key $body";
                }
                next;
            }
            next;
        }
        next;
    }

    print_sym($key) if $debug >= 3;

    close(ELFFILE);
}


###
# read_syms - Get section and symbol information.

sub read_syms($)
{
    my $file = shift;
    my $key;
    my $sym = {};
    my $newfile = 0;
    my $tag;
    my $filekey;

    open(ELFFILE, "readelf -Ss $file |") or die "Cannot open file: $file";

    # Read the readelf output

    while (<ELFFILE>)
    {
        if (/^\s*\[\s*([0-9]+)\]\ (\S+)\s+ # $1 section no., $2 section name
            (\S+)\s+([0-9a-f]+)\s+  # $3 section type, $4 section addr.
            ([0-9a-f]+)\s+([0-9a-f]+)/x)   # $5 section offset, $6 section size
        {
            my $addr = oct("0x" . $4);

            next if ($addr eq 0);
            $sections[$1] =
                { name => $2, addr => $addr, size => oct("0x" . $6) };
            next;
        }
        last if (/^Symbol table/);
    }

    while (<ELFFILE>)
    {
        if (/\s*([0-9]+)\:\ ([0-9a-f]+)\s+ # $1 symbol no., $2 value
            (\S+)\s+(\S+)\s+(\S+)\s+    # $3 size, $4 type, $5 Bind
            (\S+)\s+([0-9]+)\s+(\S+)/x)    # $6 Vis, $7 Ndx, $8 Name
        {
            my $size = $3;
            my $ix = $7;
            my $name = $8;
            my $addr = oct("0x" . $2);

            next if ($addr eq 0);

            $size = oct $size if $size =~ /^0/;
            push @{$sections[$ix]->{symbols}},
                { name => $name, addr => $addr, size => $size };
            next;
        }
        next;
    }
}


##
# showargs - Print parameters to called function.

sub showargs($$)
{
    my $funckey = shift;
    my $ebp = shift;
    my $delim = "(";
    my $sym = $symbol{$funckey};
    my $stoff = 8;
    my $name;

    if (!exists($sym->{parameters}))
    {
        print "()\n";
        return;
    }
    foreach my $arg (@{$sym->{parameters}})
    {
        $name = "<noname>";
        $name = $symbol{$arg}->{name} if (exists($symbol{$arg}->{name}));
        printf STDOUT ("%s%s=0x%08x", $delim, $name,
            get_stack($ebp + $stoff));
        $stoff += 4;
        $delim = ", ";
    }
    print ")\n";
}


##
# read_crash - Read output from process crash, adding symbolic info.

sub read_crash($)
{
    my $file = shift;
    my $ebp = 0;
    my $sp = 0;
    my $pc = 0;
    my $addr = 0;
    my $lower = 0;
    my $upper = 0;
    my $mode = 0;

    open(CRASHFILE, $file) or die "Cannot open file: $file";

    # Read the crash output

    while (<CRASHFILE>)
    {
        s/\(nil\)/0x0/g;
        if (/^---status ebp=(\w+), sp=(\w+), pc=(\w+), si_addr=(\w+)/)
        {
            $ebp = oct $1;
            $sp = oct $2;
            $pc = oct $3;
            $addr = oct $4;
            $stacktype = 1;
            chop;
            $_ .= " in " . addr2nearest($pc) . "\n";
            next;
        }
        if (/^---stack from (\w+) to (\w+)/)
        {
            $start = oct("0x" . $1);
            $end = oct("0x" . $2);
            @stack = ();
            next;
        }
        if (/^-- i960 Stack from (\w+) to (\w+)/)
        {
            $start = oct("0x" . $1);
            $end = oct("0x" . $2);
            @stack = ();
            $stacktype = 2;
            next;
        }
        if (/^([0-9a-f]+): (\w+) (\w+) (\w+) (\w+)/)
        {
            my $value;

            next if ($stacktype eq 0);
            $value = oct("0x" . $1);
            $stack[$value - $start] = oct("0x" . $2);
            $stack[$value - $start + 4] = oct("0x" . $3);
            $stack[$value - $start + 8] = oct("0x" . $4);
            $stack[$value - $start + 12] = oct("0x" . $5);
            next;
        }
        next if (/^\s+\<0\.\.0\>\s*$/);
        if (/^ c_frame .* routine\ \@\ (\w+)/ ||
            /^routine\ \@\ (\w+)/)
        {
            chop;
            $_ .= " in " . addr2nearest(oct($1)) . "\n";
            next;
        }
        if ($#stack ne -1)
        {
            if ($stacktype eq 1)
            {
                my $level = 0;
                my $funckey = get_pc_key($pc);

                printf STDOUT ("#%-3d0x%08x in %s",
                    $level, $pc, addr2nearest($pc));
                showargs($funckey, $ebp);
                eval
                {
                    while ($ebp ne 0)
                    {
                        $pc = get_stack($ebp + 4);
                        $funckey = get_pc_key($pc);

                        printf STDOUT ("#%-3d0x%08x in %s",
                            ++$level, $pc, addr2nearest($pc));
                        $ebp = get_stack($ebp);
                        showargs($funckey, $ebp);
                    }
                }
            }
            @stack = ();
            $stacktype = 0;
            next;
        }
    }
    continue
    {
        print "$_";
    }

    close(CRASHFILE);
}


## Main program

while (defined $ARGV[0] && $ARGV[0] =~ /^-(.+)/)
{
    shift;

    my $opt = $1;

    while ($opt =~ s/(.)//)
    {
        if ($1 eq 'd') { ++$debug; next; }
        if ($1 eq 'v') { ++$verbose; next; }
        if ($1 eq 'V')
        {
            print '$Id: procana.pl 89676 2009-06-19 14:49:32Z mdr $' . "\n";
            exit 0;
        }
        print STDERR
            "procana.pl [-dvV] elffile crashfile ...\n";
        print STDERR
            " For a man page do: perldoc crashana.pl\n";
        exit 2;
    }
}

read_elf($ARGV[0]);
read_syms($ARGV[0]);
shift;

read_crash($_) for @ARGV;

exit(0);

# vi:ts=4 sw=4 expandtab
