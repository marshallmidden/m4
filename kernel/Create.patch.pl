#!/usr/bin/perl -w
use strict;
use warnings;

#-----------------------------------------------------------------------------
sub usage()
{
    print STDERR "\n" .
      "Usage $0 -- has 4 arguments:\n" .
      "    CHANGED_ENTERPRISE_SOURCE = Directory containing changed kernelsource.\n" .
      "    PATCH_NAME                = What to call this patch, use -, not '[_ .]'.\n" .
      "    USER_NAME                 = 'Marshall Midden' -- for example. Note quotes.\n" .
      "    MESSAGE                   = SPECS/lightspeed.spec %changelog comment.\n" .
      "\n" .
      "Current directory is fresh git checkout used after script runs for git commit.\n" .
      "\n" .
      "Example:\n" .
      "    cd ~/clean-clone/enterprise/kernel\n" .
      "    $0 ~/working/enterprise/kernel/BUILD/k*/l* Patch_Example 'M4' Fix something.\n";

    die "\n";
}   # End of usage

#-----------------------------------------------------------------------------
# Argument processing provides the following variables.

my $CHANGED_ENTERPRISE_SOURCE;  # Where changed enterprise/kernel/BUILD/k*/l* source is.
my $PATCH_NAME;                 # Name of 'patch' partial-filename to use/create.
my $USER_NAME;                  # Name of person creating the patch.
my $MESSAGE;                    # Message for lightspeed.spec %changelog section.

#-----------------------------------------------------------------------------
sub parse_arguments()
{
print STDERR "Parsing arguments...\n";

    if ((scalar @ARGV) < 4) {
        usage();
    }
    $CHANGED_ENTERPRISE_SOURCE = shift (@ARGV);
    $PATCH_NAME                = shift(@ARGV);
    $USER_NAME                 = shift(@ARGV);
    $MESSAGE                   = join(' ', @ARGV);
}   # End of parse_arguments

#-----------------------------------------------------------------------------
# Run a system command, check for errors and die, and get rid of trailing new line.

sub run_command($)
{
    my ($cmd) = @_;

print STDERR "  Running '$cmd'\n";
    my $output = `$cmd`;
    my $err = $?;
    chomp($output);
    if ($err != 0)
    {
        $err = $err >> 8;
        print STDERR "run_command: Error exit = $err\n";
        if (defined($output) && $output ne '')
        {
            print STDERR "run_command: output='$output'\n";
        }
        exit $err;
    }
    return $output;
}   # End of run_command

#-----------------------------------------------------------------------------
# Where is files/dirs: code source, SOURCES, SPECS files?

sub find_files()
{
print STDERR "Finding needed directories and files...\n";

    my $CWD = run_command("pwd");               # Current working directory.

    my $FRESH_GIT_UNPACKED_SRC = $CWD . '/BUILD/k*/l*';
    # Expand shell globs
    $CHANGED_ENTERPRISE_SOURCE = run_command("echo $CHANGED_ENTERPRISE_SOURCE");
    $FRESH_GIT_UNPACKED_SRC = run_command("echo $FRESH_GIT_UNPACKED_SRC");

    # Check that directories exist.
    if ( ! -d $CHANGED_ENTERPRISE_SOURCE) {
        die "Changed/edited source directory '$CHANGED_ENTERPRISE_SOURCE' does not exist.\n";
    }
    if ( ! -d $FRESH_GIT_UNPACKED_SRC) {
        die "Directory to merge diffs into '$FRESH_GIT_UNPACKED_SRC' does not exist.\n";
    }

    # Check that the SOURCES and SPECS directories exist.
    my $FRESH_SOURCES = $CWD . '/SOURCES';
    if ( ! -d $FRESH_SOURCES) {
        die "New SOURCES directory '$FRESH_SOURCES' does not exist.\n";
    }
    my $CUR_SPECS_FILE = $CWD . '/SPECS/lightspeed.spec';
    if ( ! -s $CUR_SPECS_FILE) {
        die "Current SPECS/lightspeed.spec file '$CUR_SPECS_FILE' does not exist.\n";
    }

    return ($FRESH_GIT_UNPACKED_SRC, $FRESH_SOURCES, $CUR_SPECS_FILE);
}   # End of find_files

#-----------------------------------------------------------------------------
sub get_existing_patch_files($$$)
{
print STDERR "Getting and checking existing patches and patch files...\n";

    my ($FRESH_GIT_UNPACKED_SRC, $FRESH_SOURCES, $CUR_SPECS_FILE) = @_;
    my $OUTPUT;                 # Temporary output from backquote run command .

    # Get existing 'PATCH4xxxx: patch-3.4xxxx-' lines in spec file.
    my $cmd = "grep '^PATCH4[0-9][0-9][0-9][0-9]: patch-3\.4[0-9][0-9][0-9][0-9]-.*' $CUR_SPECS_FILE";
    $OUTPUT = run_command($cmd);
    my @patch_lines = split("\n", $OUTPUT);
    if ((scalar @patch_lines) < 1) {
        die "Did not find patch lines in file $CUR_SPECS_FILE\n";
    }

    # Get existing 'ApplyOptionalPatch' lines in spec file.
    $cmd = "grep '^ApplyOptionalPatch patch-3.4[0-9][0-9][0-9][0-9]-.*' $CUR_SPECS_FILE";
    $OUTPUT = run_command($cmd);
    my @apply_patch = split("\n", $OUTPUT);
    if ((scalar @apply_patch) < 1) {
        die "Did not find apply patch lines in file $CUR_SPECS_FILE\n";
    }
    if ((scalar @patch_lines) != (scalar @apply_patch)) {
        die "Did not find same number of patches (" . scalar @patch_lines . ") and apply patches (" .
            scalar @apply_patch . ") in $CUR_SPECS_FILE\n";
    }

    # Get existing 'patch-3.xxx.patch' files in SOURCES directory.
    $cmd = "cd '$FRESH_SOURCES' && eval echo 'patch-3.*'";
    $OUTPUT = run_command($cmd);
    my @patch_files = split(" ", $OUTPUT);
    if ((scalar @patch_files) < 1) {
        die "Did not find any patch files in directory $FRESH_SOURCES\n";
    }
    if ((scalar @patch_lines) != (scalar @patch_files)) {
        die "Did not find same number of patches (" . scalar @patch_lines . ") and patch files (" .
            scalar @patch_files . ") in $FRESH_SOURCES\n\n";
    }

    # Does all patch files exist in the spec file, and in the right order?
    my $pn;                                     # Used after the loop.
    my $patch_lines_filename;
    my $apply_patch_filename;
    for (my $i = 0; $i < scalar @patch_lines; $i++) {
        my $fn;
        my $message;

        ($pn, $patch_lines_filename) = $patch_lines[$i] =~ /^PATCH(4[0-9][0-9][0-9][0-9]): (.*)$/;
        if (!defined($pn) || !defined($patch_lines_filename) ||
            $pn eq '' || $patch_lines_filename eq '') {
            die "Patch number ($pn) or patch file name ($patch_lines_filename) does not exist in line '$patch_lines[$i]'\n";
        }
        ($fn, $message) = $patch_lines_filename =~ /^patch-3\.(4[0-9][0-9][0-9][0-9])-(.*)$/;
        if (!defined($fn) || !defined($message) ||
            $fn eq '' || $message eq '') {
            die "In file name ($patch_lines[$i]), patch number ($fn) or unique name ($message) does not exist in line '$patch_lines[$i]'\n";
        }
        if ($pn ne $fn) {
            die "Patch number ($pn) or patch file name number ($fn) does not match in line '$patch_lines[$i]'\n";
        }

        # Does $apply_patch_filename[$i] match $patch_lines_filename.
        ($apply_patch_filename) = $apply_patch[$i] =~ /^ApplyOptionalPatch (.*)$/;
        if (!defined($apply_patch_filename) || $apply_patch_filename eq '') {
            die "In ApplyOptionalPatch line ($apply_patch[$i]), file name does not exist in line\n";
        }
        if ($apply_patch_filename ne $patch_lines_filename) {
            die "In ApplyOptionalPatch line ($apply_patch[$i]), file name ($apply_patch_filename) does not match ($patch_lines_filename) from line ($patch_lines[$i])\n";
        }

        # Does file name patch match that in directory SOURCES?
        if ($patch_files[$i] ne $apply_patch_filename)
        {
            die "patch file name ($apply_patch_filename) does not match SOURCES directory patch files entry #$i ($patch_files[$i])\n";
        }

        if ($message eq $PATCH_NAME) {
            die "The new patch name ($PATCH_NAME) script argument already exists in line $patch_lines[$i]\n";
        }
    }
    return ($pn, $patch_lines[(scalar @patch_lines) -1], $apply_patch[(scalar @apply_patch) -1]);

}   # End of get_existing_patch_files

#-----------------------------------------------------------------------------
sub get_ready_for_diff($)
{
print STDERR "Get rid of .o and other files from the two directories.\n";

    my ($FRESH_GIT_UNPACKED_SRC) = @_;

    # in this directory source...
    run_command("cd ${FRESH_GIT_UNPACKED_SRC} && make clean distclean");
    # in unchanged 'original' directory source...
    run_command("cd ${CHANGED_ENTERPRISE_SOURCE} && make clean distclean");

    my $cmd = "cd $FRESH_GIT_UNPACKED_SRC && find . -name '*.mod' -o -name Module.symvers -o -name tags -o -name cscope.out -o -name drivers.undef -o -name hdrwarnings.txt -o -name '*.gz'";
    my $OUTPUT = run_command($cmd);
    if (defined($OUTPUT) && $OUTPUT ne '') {
        my @lo = split("\n", $OUTPUT);
        $OUTPUT = join(' ', @lo);
        $cmd = "cd ${FRESH_GIT_UNPACKED_SRC} && rm -f $OUTPUT";
        run_command($cmd);
    }

    $cmd = "cd $CHANGED_ENTERPRISE_SOURCE && find . -name '*.mod' -o -name Module.symvers -o -name tags -o -name cscope.out -o -name drivers.undef -o -name hdrwarnings.txt -o -name '*.gz'";
    $OUTPUT = run_command($cmd);
    if (defined($OUTPUT) && $OUTPUT ne '') {
        my @lo = split("\n", $OUTPUT);
        $OUTPUT = join(' ', @lo);
        $cmd = "cd ${CHANGED_ENTERPRISE_SOURCE} && rm -f $OUTPUT";
        run_command($cmd);
    }

    print STDERR "Set up before doing diff command to create the patch file.\n";
    # The patch command is called with -p1, and 'git diff' uses a and b for old/new.
    run_command("rm -f a b");
    run_command("ln -s ${FRESH_GIT_UNPACKED_SRC} a");
    run_command("ln -s ${CHANGED_ENTERPRISE_SOURCE} b");
}   # End of get_ready_for_diff

#-----------------------------------------------------------------------------
sub check_toss_or_keep($$)
{
    my ($data_Makefile, $o) = @_;

    my @match_pat = (
"diff -Nrp -U 7 --no-ignore-file-name-case --strip-trailing-cr '--exclude=\\.git\\*' '--exclude=objtool' '--exclude=cpupower' a/Makefile b/Makefile\$" ,
"--- a/Makefile .*\$" ,
"\\+\\+\\+ b/Makefile   .*\$" ,
"\\@\\@ -1,12 \\+1,12 \\@\\@\$" ,
" # SPDX-License-Identifier: GPL-2\\.0\$" ,
" VERSION = [0-9]+\$" ,
" PATCHLEVEL = [0-9]+\$" ,
" SUBLEVEL = [0-9]+\$" ,
"-EXTRAVERSION = -[0-9.]+.lightspeed\\.x86_64\$" ,
"\\+EXTRAVERSION = -[0-9.]+.lightspeed\\.x86_64\$" ,
" NAME = .*\$" ,
" \$" ,
" # \\*DOCUMENTATION\\*\$" ,
" # To see a list of typical targets execute \"make help\"\$" ,
" # More info can be located in \\./README\$" ,
" # Comments in this file are targeted only to the developer, do not\$" ,
" # expect to learn how to build the kernel reading this file\\.\$" );

    my @lines = split("\n", $data_Makefile);
    if ((scalar @lines) == (scalar @match_pat)) {
        for (my $i = 0; $i < (scalar @lines); $i++)
        {
            if ($lines[$i] !~ $match_pat[$i]) {
print STDERR " $i -   failed '$match_pat[$i]'\n";
                print $o $data_Makefile;        # Put out Makefile, because it don't match.
                return;
            }
        }
    } else {
        printf STDERR "number of match lines (%d) does not equal number of diff Makefile lines (%d)\n", scalar @lines, scalar @match_pat;
        print $o $data_Makefile;        # Put out Makefile, because it don't match.
    }

print STDERR "  a/Makefile diff b/Makefile determined as tossable!\n";
}   # End of check_toss_or_keep

#-----------------------------------------------------------------------------
sub run_diff_check_reasonable_patch($$)
{
print STDERR "Run the diff command and check output reasonable\n";

    my ($FRESH_SOURCES, $new_file_name) = @_;

# Very close to what 'git diff' does, but use -U7 because m4 wants it that way.
    my $diff_cmd = "diff -Nrp -U 7 --no-ignore-file-name-case --strip-trailing-cr" .
              " '--exclude=.git*' '--exclude=objtool' '--exclude=cpupower'";
    my $pat = $diff_cmd;
    $pat =~ s/\*/\\*/;
    my $cmd = "${diff_cmd} " . "a b || true";
#-- print STDERR "\n";
#-- print STDERR "file='${FRESH_SOURCES}/${new_file_name}'\n";
#-- print STDERR "diff='$cmd\n";
#-- print STDERR "\n";

    open (my $o, ">", "${FRESH_SOURCES}/${new_file_name}")    or die "Cannot open new patch file '${FRESH_SOURCES}/${new_file_name}'\n";
    open (my $i, $cmd . '|')                or die "Cannot start diff command '$cmd'\n";

    # Now we want to get rid of the a/Makefile b/Makefile if it matches correctly.

    my $in_Makefile = 0;                # false
    my $data_Makefile;
    my $line;
    while ($line = <$i>) {
        # If we are not already in the Makefile diff...
        if ($in_Makefile == 0) {
            my ($a, $b) = $line =~ /^${pat} (.*) (.*)$/m;
            if (defined($a) && defined($b) &&
                $a eq "a/Makefile" &&
                $b eq "b/Makefile") {
                $data_Makefile = $line; # First line -- in case need write later.
                $in_Makefile = 1;       # Flag in possible Makefile to delete area.
                next;
            }

            # Check for lines that are do not start with [d@ +-] ...
            if ($line =~ /^[^d@ +-]/) {
                print STDERR "Unexpected line in diff file:\n$line";
            }
            print $o $line;             # Put out line just read.
            next;
        }

        # We are inside a/Makefile b/Makefile diff.
        my ($a, $b) = $line =~ /^${pat} (.*) (.*)$/m;
        # If another diff, check if the accumulating one should be tossed.
        if (defined($a) && defined($b)) {
            check_toss_or_keep($data_Makefile, $o);
            $in_Makefile = 0;           # out of it now.
            print $o $line;              # Put out line just read.
            next;
        }
        $data_Makefile .= $line;        # Add in subsequent lines.
    }
    if ($in_Makefile != 0) {            # In case we hit end of file.
        check_toss_or_keep($data_Makefile, $o);
    }
    close($i);
    close($o);
}   # End of run_diff_check_reasonable_patch

#-----------------------------------------------------------------------------
#  Put into the right place in lightspeed.spec file.
#      a) # Light specification file. 2018-11-15
#      b) %define pkgrelease 2018.11.15
#      c) Patch40007: patch-3.40007-nvmetcp-1.patch
#      d) ApplyOptionalPatch patch-3.40007-nvmetcp-1.patch
#      e) %changelog -- Add message.

sub make_new_lightspeed_spec_file($$$$$$$$)
{
print STDERR "Copy old spec file to .new, then rename old to .old and new to it.\n";

    my ($CUR_SPECS_FILE, $patch_lines_last, $apply_patch_last,
        $new_comment_date, $new_pkgrelease_date,
        $new_Patch, $new_ApplyOptionalPatch, $new_changelog) = @_;

    open (my $o, ">", "${CUR_SPECS_FILE}.new")  or die "Cannot open new spec file '${CUR_SPECS_FILE}.new'\n";
    open (my $i, "<", "${CUR_SPECS_FILE}")      or die "Cannot open old spec file '$CUR_SPECS_FILE'\n";

    my $line;

    # Copy first part of file (up to "a" above).
    while ($line = <$i>) {
        if ($line =~ /^# Light specification file\. /) {
            print $o $new_comment_date;
            last;
        }
        print $o $line;         # Put out line just read.
    }

    # Copy second part of file (up to "b" above).
    while ($line = <$i>) {
        if ($line =~ /^\%define pkgrelease /) {
            print $o $new_pkgrelease_date;
            last;
        }
        print $o $line;         # Put out line just read.
    }

    # Copy third part of file (up to "c" above).
    while ($line = <$i>) {
        if ($line =~ /^$patch_lines_last/) {
            print $o $line;             # Put out line just read.
            print $o $new_Patch;
            last;
        }
        print $o $line;         # Put out line just read.
    }

    # Copy fourth part of file (up to "d" above).
    while ($line = <$i>) {
        if ($line =~ /^$apply_patch_last/) {
            print $o $line;             # Put out line just read.
            print $o $new_ApplyOptionalPatch;
            last;
        }
        print $o $line;         # Put out line just read.
    }

    # Copy fifth part of file (up to "e" above).
    while ($line = <$i>) {
        if ($line =~ /^\%changelog/) {
            print $o $line;             # Put out line just read.
            print $o $new_changelog;
            last;
        }
        print $o $line;         # Put out line just read.
    }

    # Copy to end of file.
    while ($line = <$i>) {
        print $o $line;         # Put out line just read.
    }

    close($i);
    close($o);

print STDERR "Rename the files.\n";
    run_command("mv ${CUR_SPECS_FILE} ${CUR_SPECS_FILE}.old");
    run_command("mv ${CUR_SPECS_FILE}.new ${CUR_SPECS_FILE}");

}   # End of make_new_lightspeed_spec_file

#-----------------------------------------------------------------------------
#--     Do git status?
#----------------------------------------------------------------------------

# Main program starts here.
{
    my ($FRESH_GIT_UNPACKED_SRC, $FRESH_SOURCES, $CUR_SPECS_FILE);
    my ($LAST, $patch_lines_last, $apply_patch_last);
    my $NEXT;
    my $DATE;
    my $new_file_name;
    my $mod_date;
    my $per_date;
    my $new_comment_date;
    my $new_pkgrelease_date;
    my $new_Patch;
    my $new_ApplyOptionalPatch;
    my $new_changelog;
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

    parse_arguments();

    #   new source code  ,  SOURCES dir, SPECS/lightspeed.spec
    ($FRESH_GIT_UNPACKED_SRC, $FRESH_SOURCES, $CUR_SPECS_FILE) = find_files();
#-- print STDERR "\n";
#-- print STDERR "FRESH_GIT_UNPACKED_SRC=$FRESH_GIT_UNPACKED_SRC\n";
#-- print STDERR "FRESH_SOURCES=$FRESH_SOURCES\n";
#-- print STDERR "CUR_SPECS_FILE=$CUR_SPECS_FILE\n";
#-- print STDERR "\n";

    # NOTE: $LAST is last number found/used in lightspeed.spec file.
    ($LAST, $patch_lines_last, $apply_patch_last) =
              get_existing_patch_files($FRESH_GIT_UNPACKED_SRC, $FRESH_SOURCES, $CUR_SPECS_FILE);
#-- print STDERR "\n";
#-- print STDERR "LAST=$LAST\n";
#-- print STDERR "patch_lines_last=$patch_lines_last\n";
#-- print STDERR "apply_patch_last=$apply_patch_last\n";
#-- print STDERR "\n";

    get_ready_for_diff($FRESH_GIT_UNPACKED_SRC);

    # Create the values needed for putting into the spec file, and the name of patch
    # file creating, etc.

    $NEXT = $LAST + 1;

    $DATE = run_command("/bin/date '+%a %b %d %Y'");

    $new_file_name = "patch-3.${NEXT}-${PATCH_NAME}";

    $mod_date = run_command("/bin/date '+%Y-%m-%d'");
    $per_date = run_command("/bin/date '+%Y.%m.%d'");

    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    $new_comment_date = "# Light specification file. $mod_date\n";
    $new_pkgrelease_date = "%define pkgrelease $per_date\n";
    $new_Patch = "PATCH${NEXT}: ${new_file_name}\n";
    $new_ApplyOptionalPatch = "ApplyOptionalPatch ${new_file_name}\n";

    $new_changelog = "* ${DATE} ${USER_NAME}\n" .
                        "- [kernel] rhel-kernel: ${MESSAGE}\n" .
                        "\n";
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    run_diff_check_reasonable_patch($FRESH_SOURCES, $new_file_name);

    make_new_lightspeed_spec_file($CUR_SPECS_FILE, $patch_lines_last, $apply_patch_last,
                                  $new_comment_date, $new_pkgrelease_date,
                                  $new_Patch, $new_ApplyOptionalPatch,
                                  $new_changelog);

    print STDERR "------------------------------------------------------------------------------\n";
    print STDERR "CHANGED_ENTERPRISE_SOURCE = '${CHANGED_ENTERPRISE_SOURCE}'\n";
    print STDERR "FRESH_GIT_UNPACKED_SRC    = '${FRESH_GIT_UNPACKED_SRC}'\n";
    # print STDERR "PATCH_NAME                = '${PATCH_NAME}'\n";
    # print STDERR "USER_NAME                 = '${USER_NAME}'\n";
    # print STDERR "MESSAGE                   = '${MESSAGE}'\n";
    print STDERR "LAST                      = '${LAST}'\n";
    print STDERR "NEXT                      = '${NEXT}'\n";
    # print STDERR "DATE                      = '${DATE}'\n";

    print STDERR "new_file_name             = '$new_file_name'\n";
    print STDERR "new_ApplyOptionalPatch    = $new_ApplyOptionalPatch";
    print STDERR "new_Patch                 = $new_Patch";
    print STDERR "new_changelog             = \n***\n$new_changelog***\n";

    #-----------------------------------------------------------------------------
    exit 0;
}

#-----------------------------------------------------------------------------
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#-----------------------------------------------------------------------------
# End of file Create.patch.pl
