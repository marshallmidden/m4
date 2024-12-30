#!/usr/bin/perl -w
#====================================================================
#
# FILE NAME:    NightlyBuild.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         03/04/2005
#
# DESCRIPTION:  The Nightly Build / Regression for Wookiee.
#
#====================================================================

use strict;
use	Getopt::Std;
use File::Copy;
use Cwd;

my $startDir = cwd();

# List of those names that get notified of all builds
# Note: all mail recipients must be "@xiotech.com"
my @MAIL_ALL = ("swatosht", "rysavyr", "holtyb", 
                "sneadj", "nigburc", "waggiel",
                "menninc", "williamj");           # named array w/()'s. Note: the
                                                  # MAIL vars below use anonymous 
                                                  # arrays w/[]'s.
                                                  # for example:   MAIL => ["sneadj",
                                                  #            "nigburc", "waggiel"]

my %work = (
    MAIN => 
    {
        BUILD_IT        => "YES",
        WHERE           => "CHEX",
        CONTROLLERS     => ["10.64.100.94", "10.64.100.95"],
        MAIL            => undef,
    },

    WOOKIEE_BACON_GA_BR => 
    {
        BUILD_IT        => "YES",
        WHERE           => "BACON",
#        CONTROLLERS     => ["10.64.100.92"],  # Holty's
        CONTROLLERS     => ["10.64.100.94", "10.64.100.95"],
        MAIL            => undef,
    },
);

#############################################################################
###  Modifications to add/delete branches and the like should only be made 
###  above this line!  

#
# Get the command line args
#
our ($opt_d); 
getopts('d');
 
#
# Print out help/usage info
#
my $script = $0;
$script =~ s/\\/\//g;  # back slashes -> forward slashes
$script =~ s/^.*\///;  # get base name
if (! @ARGV < 1) 
{ 
    print "\nUsage: $script [-d (=> debug)]\n";
    exit 1;
}
my $debug = $opt_d ? "-d" : "";

#
# Run a system call
#
my $runOutput;
sub run
{
    my ($cmd) = @_;
    
    $runOutput = "> $cmd\n";
    print $runOutput;
    
    $runOutput .= ` $cmd 2>&1 `;
    my $runRC = $? >> 8;

    $runOutput .= "> rc = $runRC\n";
    print "> rc = $runRC\n";

    return $runRC;
}

#
# Append data to log file
#
sub AddToLog
{
    my ($file, $pWhat) = @_;
   
    if (defined($$pWhat))
    {
        open LOG, ">>$file" or warn "Couldn't open $file!\n";
        print LOG "$$pWhat";
        close LOG;
    }
}

#
# Go do the builds
#
my @workKeys = keys(%work);
while (@workKeys)
{
    my $branch = shift @workKeys;
    next if ($work{$branch}{BUILD_IT} !~ /YES/i);
    
    # If BUILD_IT == YES, go do it
    my $mailList;
    my $name;
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
    $mday-- if ($opt_d); # Use yesterday if debug mode
    my $version = sprintf "N%x%02u", $mon+1, $mday;
    my $logfile = "/tmp/Build_"."$branch"."_"."$version".".log";
    my $logfiletgz = "$logfile".".tgz";
    my $buildDir = $logfile; $buildDir =~ s/\.log/.dir/;
    my $sendfile;
    my @controllers = defined (@{ $work{$branch}{CONTROLLERS} }) ? 
                        @{ $work{$branch}{CONTROLLERS} } : ();
    my $result = "PASSED";
    my $attachment = "";
    my $rc = 0;
    my $errorTxt = undef;
    
    #
    # This loop construct exists only to give us a convenient 'goto' point
    # at the bottom of it (when we call 'last').
    #
    while (1)
    {
        chdir $startDir;
        
        #
        # Go run the build
        #
        $rc = run "./BuildRelease $debug -l $logfile -b $buildDir -cfmx ".
                  "$branch $version NIGHTLY $work{$branch}{WHERE}";
        
        #
        # Check for successful build
        #
        if ($rc)
        {
            # Failure!
            $errorTxt = "\nBuildRelease Failed! Aborting.\n\n";
            $rc = 1;
            last;
        }
        
        #
        # Check if we can run a regression test
        #
        if (@controllers == 0)
        {
            # No regression (not a failure)
            $errorTxt = "\nNo controllers were listed to run RegressionNWay on.\n\n";
            $rc = 0; # not a failure
            last;
        }

        #
        # Go get the load file
        #
        my @buildFile = glob("$buildDir/Wookiee/YetiApps/*.rpm2");

        if (@buildFile != 1)
        {
            # Failure!
            $errorTxt = "\nMore than one 'rpm2' file found! Aborting.\n\n";
            $rc = 1;
            last;
        }

        #
        # Load the code on the controllers and reset them
        #
        chdir "$buildDir/Wookiee/CCBE";
        run "ccbcl @controllers -e all:fwu $buildFile[0]\\; all:reset all 1";
        AddToLog $logfile, \$runOutput;

        #
        # Count the successful FW loads
        # 
        my $count = 0;
        while ($runOutput =~ /Firmware update successful/ig) 
        {
            $count++;
        }

        #
        # count should == num controllers
        #
        if ($count != @controllers)
        {
            # Failure!
            $errorTxt = "\nOne or more FW updates apparently didn't succeed. ".
                        "Aborting.\n\n";
            $rc = 1;
            last;
        }

        #
        # Wait here for the controllers to come back online.  Normally this
        # isn't necessary since RegressionNWay will do a reboot anyway, but it
        # does help the case where the BE is hung up and an MFGCLEAN can't get
        # through.  If they don't come back up, RegressionNWay will catch
        # that.
        #
        print "Sleeping 5 minutes while controllers reboot...\n";
        sleep (60 * 5); 
        
        #
        # RegressionNWay does a mfgclean 
        # 
        chdir "$buildDir/Wookiee/Test";
        unlink "/tmp/RegressionNWay.log";

        $rc = run "perl RegressionNWay.pl @controllers -v -l 1";
        AddToLog $logfile, \$runOutput;

        #
        # Check for successful run of RegressionNWay
        #
        if ($rc)
        {
            # Failure!
            $errorTxt = "\nRegressionNWay Failed! Aborting.\n\n";
            $rc = 1;
            last;
        }
        
        #
        # Must exit the loop here!
        #
        last;
    }

    #
    # Log the failure message if it was set.
    #
    AddToLog($logfile, \$errorTxt) if defined($errorTxt);
    
    #
    # Compress the logfile
    #
    if (run("tar -czf $logfiletgz $logfile") == 0)
    {
        $sendfile = "$logfiletgz";
    }
    else
    {
        $sendfile = "$logfile";
    }
    # Not much use in logging anything after this point.

    #
    # Copy the other stuff that we want to save to YetiApps
    #
    chdir "$buildDir/Wookiee/YetiApps";
    
    copy "$buildDir/Wookiee/Proc/src/obj/Front.nm",  ".";
    copy "$buildDir/Wookiee/Proc/src/obj/Front.map", "."; 
    copy "$buildDir/Wookiee/Proc/src/obj/Back.nm",   ".";
    copy "$buildDir/Wookiee/Proc/src/obj/Back.map",  ".";
    copy "$buildDir/Wookiee/CCB/Src/obj/ccbrun.map", ".";
    copy "$buildDir/Wookiee/CCB/Src/obj/ccbrun.ima", ".";
    copy "$sendfile",                                ".";

    #
    # Now copy it all up to the K: or Y: drive
    #
    use constant K_DRIVE => "//Rststore/Private";
    use constant Y_DRIVE => "//labstore/labdata";
    my $drive = Y_DRIVE;
    run "tar cf - * | smbclient $drive -A ~/.smbrc ".
        "-c \"cd Release ; ".
        "mkdir Yeti ; cd Yeti ; ".
        "mkdir $work{$branch}{WHERE} ; cd $work{$branch}{WHERE} ; ".
        "mkdir Nightly ; cd Nightly ; ". 
        "mkdir $version ; cd $version\" -Tqx - ";
        
    #
    # $result is used in the email subject line
    # Note: $rc is either the result of 'BuildRelease' or of 'RegressionNWay'
    # Attach the log on failure, otherwise don't.
    #
    if ($rc)
    {
       $result = "FAILED"; # defaults to "PASSED"
       $attachment = "-a $sendfile";
    }
    
    #
    # Build up the 'To:' email list
    # 
    foreach $name (@MAIL_ALL, @{ $work{$branch}{MAIL} })
    {
        $mailList .= "$name\@xiotech.com,";
    }
    $mailList =~ s/,$//;
    $mailList = "rysavyr\@xiotech.com" if ($opt_d);
    
    #
    # Send out the results email
    #
    my $path = "$drive/Release/Yeti/$work{$branch}{WHERE}/".
              "Nightly/$version/";
    $path =~ s/\//\\/g;
    my $msg = "This is an automated message -- do not reply.\n\n".
              "The nightly build/test of $branch $result.\n\n".
              "Results can be found at:\n$path\n";
              
    run "echo '$msg' | ".
        "mail $attachment -r 'Nightly Build<Nightly_Build\@xiotech.com>' ".
            "-s 'Nightly Build/Test of $branch => $result' $mailList";

    #
    # Clean up the log files
    #
    next if ($opt_d); # don't delete anything in debug mode
    
    unlink "$logfile";
    system "rm -r $buildDir";
    unlink "$logfiletgz";
    unlink "/tmp/RegressionNWay.log";
}

#sub TagAndDirCleanup
#{
#    my ($branch, $buildDir) = @_;
#
#
#    
#}

exit 0;
