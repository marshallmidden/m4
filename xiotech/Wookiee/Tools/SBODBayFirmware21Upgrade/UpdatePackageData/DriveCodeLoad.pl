#! /user/bin/perl -I CCBE -I TestLibs -w
# $Header$

##############################################################################
#
#   CCBE Drive firmware update script
#
#   6/10/2002  XIOtech   Craig Menning
#
#   A small test file for updating drive fw
#
#   Copyright 2002 XIOtech, A Seagate Company
#
#   For XIOtech internal use only.
#
##############################################################################
=head1 NAME

driveCodeLoad.pl - A script to upload code to the drives and drive bays 
attached to a controller.

$Id: DriveCodeLoad.pl 149462 2010-10-22 19:32:41Z tom_marlin $

=head1 SUPPORTED PLATFORMS

=begin html

 <UL> 
     <LI>Linux</LI> 
     <LI>Windows</LI> 
 </UL>

=end html

=head1 SYNOPSIS

This is a utility to upload new firmware.


=head1 DESCRIPTION

Invocation:

drivecodeload.pl <ip addr> <opErrorsFile> <opStatusFile> <cfg file(s)><return>

 where:
    <ip addr> is the IP address of a controller
    <opErrorsFile> path to the OpErrors.txt file that we write errors into
    <opStatusFile> path to the OpStatus.txt file that we write % complete and status strings to
    <cfg file> is the name of a config file that is used to configure the 
               script.

    All parameters are required. 
     
 Typical use (with fake ip address):

    drivecodeload.pl 10.64.99.341  ../OpErrors.txt ../OpStatus.txt SampleDCL <return>

 Notes:
    The configuration file contains information about the drives and bays.
    Specifically, the inquiry response (model) and desired firmware revision.
    It also contains the name and path to the firmware code files that are
    to be used. The path may be either relative or absolute. If relative, 
    they need to be relative to the working directory when the script is 
    launched.

    The script can do  the drives, or the bays, but not both at the same 
    time. The config file determines which will be done. 
    
    If the script fails to complete it can be run an additional time. Drives
    that are at the correct revision will not have the code loaded a 
    second time. This is also a means to verify the code levels on a system.

    There should be no IO to the system when this is run.

    The config file can contain information for different models, so a 
    system with different models drives can have all drives updated by 
    running the script once.

=cut

#
# - other modules used
#

#  Service Notes 8/21
#  
#  
# 1.	 Priority for 3000 customers for 08 bay code upgrades.
#  a.	Prereq exists for 3000 to 4000 upgrades be upgraded to 08 before the 4000 upgrade.
#  b.	Bay adds to a 3000 - how many do we anticipate?
# 2.	4000 Bay code upgrades for SATA bays - what are our options for an uptime upgrade?   
#


use lib "CCBE";


use XIOTech::cmdMgr;
use XIOTech::cmUtils;
use XIOTech::constants;
use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::logMgr;
use TestLibs::Logging;
use TestLibs::Constants qw(:DEFAULT  :CCBE);


#
# - some global variables/constant
#

my $currentConnection = -1;
my $currentIP = "";
my $currentPort = CCB_PORT;
my $currentMgr;
my $numNextConnection = 0;
my %connections;
my $g_dsc_id = 0;

use constant DRIVE => "drive";     # for drive code load
use constant BAY => "bay";         # for bay code load

use constant CALCULATE_ONLY => 0;  #Flag telling the script to not perform updates
use constant UPDATE_CODE => 1;     #Flag telling the script to perform updates

#use constant GOOD => 0;     #define GOOD 0
#use constant ERROR => 1;    #define ERROR 1
#use constant FALSE => 0;    #define GOOD 0
#use constant TRUE => 1;     #define ERROR 1
#use constant INVALID => -999;  # must be negative (shouldn't need to change)


#
# Globals for reporting proper status
#

my $opErrorsFile = "";    # To be filled in later
my $opStatusFile = "";    # To be filled in later
my $totalMatchingPdisks = 0; # To be filled in later
my $currentMatchingPdiskIndex = 0; # Counting how many pdisks we've done
my $executionMode = CALCULATE_ONLY; #Initial mode to run in to calculate info


#
# - perl compiler/interpreter flags 'n' things
#

#use Getopt::Std;
use FileHandle;
#use Text::Abbrev;
#use Cwd;
#use IO::Handle;

use strict;
use warnings;



#----Start Logging File

#my $logFile = XIOTech::logMgr::logStart("craigTest.txt", "TS");

#---Autoflush all stdout writes

STDOUT->autoflush(1);
STDERR->autoflush(1);

#
#   ---   Use one or the other of the two lines that follow
#

#MyTestMain();        # this one uses embedded configuration info
MyTestMain2();      # this one reads config files

#
#   ---  
#



exit 0;



##############################################################################
#
#          Name: displayError
#
#        Inputs: ErrorMsg, ErrorValue
#
#       Outputs: none
#
#  Globals Used: none
#
#   Description: Outputs a message to the log file
#
#
##############################################################################

sub displayError($$)
{
  print STDOUT "ERR: '$_[0]' Val: '$_[1]'";
}


##############################################################################
#
#          Name: send_updated_progress
#
#        Inputs: Status string
#
#       Outputs: none
#
#  Globals Used: totalDrives, drivesUpdated
#
#   Description: Outputs a the current status to the OpStatus.txt file
#
#
##############################################################################
sub send_updated_progress($)
{
    my $statusString = $_[0];
    print STDOUT "Outputing status: $statusString";
    
    #
    # Calculate this script's percent complete
    #
    
    my $percent_complete = 0;
    if ($totalMatchingPdisks > 0)
    {
      $percent_complete = $currentMatchingPdiskIndex / $totalMatchingPdisks * 100.0;
    }
    
    open (STATUS_FILE, ">$opStatusFile") or print STDERR "Can't open file '$opStatusFile' $!"; 
    print (STATUS_FILE "PercentComplete=$percent_complete\nStatus=$statusString");
    close (STATUS_FILE);
    
    print STDOUT "PercentCompleteDebug:  totalPdisk: $totalMatchingPdisks  current: $currentMatchingPdiskIndex percent: $percent_complete\n";

}   # End of send_updated_progress


##############################################################################
#
#          Name: send_error_message
#
#        Inputs: string to output to a file
#
#       Outputs: none
#
#  Globals Used: none
#
#   Description: Outputs a given string to the OpErrors.txt file
#
#
##############################################################################
sub send_error_message($)
{
  my $message = $_[0];

  print STDOUT "Outputing error: $message";
  open (ERRORS_FILE, ">>$opErrorsFile") or print STDERR "Can't open file '$opErrorsFile' $!";
  print (ERRORS_FILE "$message\n");
  close (ERRORS_FILE);

}   # End of send_error_message



##############################################################################
#
#          Name: TestMain
#
#        Inputs: none
#
#       Outputs: none
#
#  Globals Used: none
#
#   Description: The old front end for embedded config info
#
#
##############################################################################
sub MyTestMain
{

    my $i;

    my $ipAddr;        # master's IP

    my $ret;           # a returned value

    my $master;        # object for controller

    my $inf;



    # passed on the cmdline or not.

    if (@ARGV==1)
    {
#       print "--------> Parsing command line for next IP address <--------- \n";
        # Get the command line parm and see if an IP address
        ($inf) = shift @ARGV;

        # If its an IP address, we will connect to it automatically
        if ($inf =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?:*\d*$/)
        {
            $ipAddr = $inf;
        }
        else        # parm not an IP address
        {
            printHelp();
            return ERROR;;
        }
            
    }
    else            # wrong number of parms
    {
        printHelp();
        return ERROR;;
    }

    #
    # NOTE: Currently only EXACTLY one IP address is supported
    #

    print "\n";


    #
    # establish controller connections
    #

    #print " new on $ipList[$i] is ";

    $master = XIOTech::cmdMgr->new(\*STDOUT);


    print "logging in to controller  \n";

    $ret =  $master->login( $ipAddr, CCB_PORT);       # connect to controller
    if ( ! $ret  )
    {
        print ">>>>>>>> Failed to connect to $ipAddr  <<<<<<<<\n";
        return (ERROR);
    }

    #print " login complete, setting timeout \n";

    #---Set TimeOut
    $master->{TIMEOUT} = 700;



    #
    # now do the update
    #

  
    my @driveList = ( "ST336752FC", "ST336605FC", "st3146807fc", 
                      "st318304fc", "st373405fc", "st373453fc"  );

    my @FWlist =    ( "0005",       "0005" ,      "0004",
                      "0005",       "0005",       "F103"  );

    my @codeList =  ( "ST336752FC_0005.ima", "ST33605FC_0005.ima", "146GB_FC_0004.ima", 
                      "st318304fc_0005.ima", "st373405fc_0005.ima", "st373453fc_F103.ima"     );
    
    my @codeList2=  ( "none", "none", "none", 
                      "none", "none", "none"     );



    $ret = UpdateDriveCode($master, DRIVE, \@driveList, \@FWlist, \@codeList, \@codeList2 );
    
    if ( GOOD != $ret )
    {
        print ">>>>>>>> Unable to UpdateDriveCode <<<<<<<<\n";
        return ERROR;
    }
        

    #
    # terminate controller connection
    #

    $master->logout();


    return GOOD;
}

##############################################################################
#
#          Name: TestMain2
#
#        Inputs: none
#
#       Outputs: none
#
#  Globals Used: none
#
#   Description: The new front end for use with config files
#
#
##############################################################################
sub MyTestMain2
{

    my $i;

    my $ipAddr;        # master's IP

    my $ret;           # a returned value

    my $master;        # object for controller

    my $inf;

    my $cfgFile;

    my $task; 
    my @driveList;
    my @FWlist;
    my @codeList;
    my @codeList2;

    my $fileParseError = FALSE;


    # passed on the cmdline or not.
    
    # The call should look like this:
    # drivecodeload.pl <ip addr> <opErrorsFile> <opStatusFile> <cfg file> 

    my $numElements = $#ARGV + 1;
    if ($numElements >= 4)
    {
#       print "--------> Parsing command line for next IP address <--------- \n";
        # Get the command line parm and see if an IP address
        ($inf) = $ARGV[0];

        # If its an IP address, we will connect to it automatically
        if ($inf =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?:*\d*$/)
        {
            $ipAddr = $inf;
        }
        else        # parm not an IP address
        {
            print "$inf\n";
            printHelp();
            send_error_message "Unable to determine master controller IP address from file.";
            return ERROR;;
        }

        #
        # get the rest of the inputs
        #
        
        $opErrorsFile = $ARGV[1];
        $opStatusFile = $ARGV[2];
        
        #
        # shift all arguements except the config files
        #
        
        my $temp = shift @ARGV; #shift off IP addr
        $temp = shift @ARGV;    #shift off errors file
        $temp = shift @ARGV;    #shift off status file
        

        ################################################
        #  Add code here to chop up the config file
        ################################################
        
        # the following arrays/items are filled in
        #
        # @driveList, @FWlist, @codeList, @codeList2, $task
        #
        #
        #  @driveList is a list of the drive identifier strings eg. "ST336752FC"
        #
        #  @FWList is a list of code revisions eg.  "F103"
        #
        #  @codeList is a list of code file names, with path
        #     eg. "..\Drives\Seagate\ST33605FC_0005.ima" 
        #     paths can be relative or absolute
        #  
        #  @codeList2 is a list of secondary files
        #     eg. "none" or "st373405fc_0005.ima"
        #     paths can be relative or absolute
        #
        #  $task is "drive" or "bay" to indicate which is being updated
        #
        #  

        
        print("Parsing config files...\n");
    
        no strict 'refs';

        #
        # Process the argument list of files along with any includes.
        #
        foreach $cfgFile (@ARGV) 
        {
            if (process($cfgFile, 'fh00', \$task, \@driveList, \@FWlist, \@codeList, \@codeList2) == ERROR)
            {
                $fileParseError = TRUE;
            }
        }

        if ($fileParseError == TRUE)
        {
            send_error_message "Error parsing bay-update configuration file.";
            return ERROR;
        }            
    }
    else            # wrong number of parms
    {
        printHelp();
        send_error_message "Insufficient arguements to DriveCodeLoad.pl";
        return ERROR;;
    }

    #
    # NOTE: Currently only EXACTLY one IP address is supported
    #

    print "\n";

#return GOOD;

    #
    # establish controller connections
    #

    #print " new on $ipList[$i] is ";

    $master = XIOTech::cmdMgr->new(\*STDOUT);

    print "logging in to controller  \n";

    $ret =  $master->login( $ipAddr, CCB_PORT);       # connect to controller
    if ( ! $ret  )
    {
        print ">>>>>>>> Failed to connect to $ipAddr  <<<<<<<<\n";
        return (ERROR);
    }

    # Check to see if this is really the master.  Otherwise return GOOD for slave.  Only needs to run once
    
    ( my $rc, my $masterIndex, my $dscId) = FindMasterIndex( $master );
    $g_dsc_id = $dscId;
    send_updated_progress("DSC $dscId: Checking back-end device paths...");
    
    if( $rc == ERROR )
    {
      # This is not the Master controller.  Return Good to not run this twice by mistake.
      return (GOOD);
    }
    
    #print " login complete, setting timeout \n";

    #---Set TimeOut
    $master->{TIMEOUT} = 700;

    # Send start message
    forceLog("Drive/Bay Code Script Started", LOG_TYPE_WARNING, $master);
    sleep 10;

    # Check the BE Device paths prior to starting the main process.
    
    $ret = check_bedevice($master);
    
    if ( GOOD != $ret )
    {
        forceLog("Drive/Bay Code Failed:  Back End Paths are not good - Please contact service", LOG_TYPE_ERROR, $master);
        send_error_message("Back-end paths are not good - Please contact customer service.");
        return ERROR;
    }
    else
    {
        forceLog("Drive/Bay Code BEDevice check:  All paths operational", LOG_TYPE_INFO, $master);
    }       



    #
    # now do the update
    #

  
#    my @driveList = ( "ST336752FC", "ST336605FC", "st3146807fc", 
#                      "st318304fc", "st373405fc", "st373453fc"  );
#
#    my @FWlist =    ( "0005",       "0005" ,      "0004",
#                      "0005",       "0005",       "F103"   );
#
#    my @codeList =  ( "ST336752FC_0005.ima", "ST33605FC_0005.ima", "146GB_FC_0004.ima", 
#                      "st318304fc_0005.ima", "st373405fc_0005.ima", "st373453fc_F103.ima"     );
#    
#    my @codeList2=  ( "none", "none", "none", 
#                      "none", "none", "none"     );
#



    #
    # Toggle into a mode which doesn't perform any updates, but checks to see how much work
    # there is to do
    #
    
    print STDOUT "Setting 'CALCULATE_ONLY' mode\n";
    send_updated_progress("DSC $dscId: Analyzing physical disks (this may take a while)...");
    $executionMode = CALCULATE_ONLY;
    $ret = UpdateDriveCode($master, $task, \@driveList, \@FWlist, \@codeList, \@codeList2);
    
    
    
    #
    # Toggle into a mode where we'll actually perform the updates, and run it again
    #
        
    print STDOUT "Toggling into 'UPDATE_CODE' mode\n";
    $executionMode = UPDATE_CODE;
    $ret = UpdateDriveCode($master, $task, \@driveList, \@FWlist, \@codeList, \@codeList2);
    if ( GOOD != $ret )
    {
        forceLog("Drive/Bay Code Failed:  Error during update", LOG_TYPE_ERROR, $master);
        send_error_message "Drive/Bay code update failed.";
        return ERROR;
    }
    else
    {
        forceLog("Drive/Bay Code Upgrade Complete", LOG_TYPE_WARNING, $master);
    }    

    #
    # terminate controller connection
    #

    $master->logout();


    return GOOD;
}




##############################################################################
#
#          Name: printHelp
#
#        Inputs: 
#
#       Outputs: GOOD, if successful, ERROR otherwise
#
#  Globals Used: none
#
#   Description: a help screen 
#
#
##############################################################################
sub printHelp
{
    print "\n";
    print "This script is used to update firmware on the drives in the enclosure\n";
    print "or the firmware on the enclosure.  The IP address of the controller must\n";
    print "be passed when the script is invoked.  A configuration file(s) must be\n";
    print "passed in after the IP address, there can be multiple config files passed in.\n";
    print "\n";
    print "Usage:\n";
    print "  DriveCodeLoad.pl xxx.xxx.xxx.xxx  <cfg file>\n";
    print "\n";                            
    print "  Where xxx.xxx.xxx.xxx is the ip address of your controller\n";
    print "       and <cfg File> is the configuration file name \n";
    print "\n";
    print "\n";


    return GOOD;
}

##############################################################################
#
#          Name: UpdateDriveCode
#
#        Inputs: controller object, code index, ptr to drives array
#                ptr to ident array, ptr to file array
#
#       Outputs: GOOD, if successful, ERROR otherwise
#
#  Globals Used: executionMode
#
#   Description: update code on all pdds 
#
#
##############################################################################
sub UpdateDriveCode
{
    my ( $ctlr, $type ,$drivePtr, $FWptr, $fnPtr, $scPtr) = @_;

    my %info;
    my $ret;
    my %rtn;
    my %rsp;
    my $i;
    my $id;
    my $j;
    my $drv;
    my $rev;
    my @driveList;
    my @FWList;
    my $wwnLo;
    my $wwnHi;
    my $lun;
    my $flag;
    my @fnList;
    my @SCList;
    my $drvList;
    my $msg;
    my $type_mess;
    my $skipupdate = 0;
    my %rspext;
    my %infoext;
    


    $flag = GOOD;

    @driveList = @$drivePtr; 
    @FWList = @$FWptr; 
    @SCList = @$scPtr; 
    @fnList = @$fnPtr;

    use strict;

    
    # need a longer timeout for this
    my $value = 1200;
    print "Setting MRP timeout to $value seconds\n";
    %rsp = $ctlr->timeoutMRP("MRP", $value);
    if (!%rsp)                        # no response
    {
        print ">>>>>>>> Failed to receive a response from timeoutMRP <<<<<<<<\n";
        return ERROR;
    }

    if ($rsp{STATUS} == 1)            # 1 is bad
    {
        
        print ">>>>>>>> Failed: timeoutMRP returned an error <<<<<<<<\n";
        
        PrintError(%rsp);
        
        return ERROR;
    }

    ############################
    #  PDISKLIST or diskBayList
    ############################


    if ( $type eq DRIVE )
    {
        %rsp = $ctlr->physicalDiskList();
    }
    else
    {
        %rsp = $ctlr->diskBayList();
    }
    # end change



    if ( ! %rsp  )
    {
        print ">>>>>>>> Failed to get response from physicalDiskList <<<<<<<<\n";
        return ERROR;
    }
    if ($rsp{STATUS} != PI_GOOD)
    {
        print ">>>>>>>> Unable to physicalDiskList <<<<<<<<\n";
        PrintError(%rsp);
        return ERROR;
    }

    print "\nGetting physical disk list.\n";

    for $i (0..$#{$rsp{LIST}})
    {
        print "Examining device  " . $rsp{LIST}[$i] . "\n";
        $id =  $rsp{LIST}[$i];

        #############################
        #  PDISKINFO or DISKBAYINFO
        #############################
        print "Retrieving physical information for each device.\n";


        if ( $type eq DRIVE )
        {
            %info = $ctlr->physicalDiskInfo($id);
        }
        else
        {
            %info = $ctlr->diskBayInfo($id);
        }
        # end change

        if ( ! %info  )
        {
            logError( ">>>>>>>> Failed to get response from physicalDisk/bayInfo <<<<<<<<");
            send_error_message "Failed to get bay and physical disk information.";
            return ERROR;
        }

        if ($info{STATUS} != PI_GOOD)
        {
            print ">>>>>>>> Unable to get physicalDisk/bayInfo <<<<<<<<\n";
            PrintError(%info);
            $flag = ERROR;
            next;
            #return ERROR;
        }

        $drv = $info{PS_PRODID};
        $drv = join ('', split(/\s+/,$drv));     # Remove any spaces
        $drv = lc($drv);
        $rev = $info{PD_REV};
        $wwnLo = $info{WWN_LO};
        $wwnHi = $info{WWN_HI};
        $lun = $info{PD_LUN};



        logInfo("  PS_PRODID:     $info{PS_PRODID}");
        printf "  WWN:           %8.8x%8.8x\n", $info{WWN_LO}, $info{WWN_HI};
        printf "  PD_LUN:        %hu\n", $info{PD_LUN};
        
        for ( $j = 0; $j < scalar(@FWList); $j++ )
        {
            my $drvmatch = 0;
            
            #
            # Remove the whitespaces
            #
            $drvList =  join ('', split(/\s+/,$driveList[$j]));
            $drvList = lc($drvList);

                        
            #
            # Check to see if there is an exact match
            #
            if ($drv eq $drvList)
            {
              $drvmatch = 1;
            }
            
            if ( $drvmatch == 1 )
            {
                logInfo("  Device ID matched, index = $j, old rev is $rev, needs to be $FWList[$j]");

                #
                # now we need to update the code
                #

                my $codeMatch = ( $rev =~ /$FWList[$j]/i );

                if ( $codeMatch != 1 )
                {
                                        
                    
                    %rspext = $ctlr->physicalDiskList();
                    if ( ! %rspext  )
                    {
                        logInfo(">>>>>>>> Failed to get response from physicalDiskList <<<<<<<<");
                        send_error_message "Could not retrieve physical disk list from controller. Empty response";
                        return ERROR;
                    }
                    if ($rspext{STATUS} != PI_GOOD)
                    {
                        logInfo(">>>>>>>> Unable to physicalDiskList <<<<<<<<");
                        send_error_message "Could not retrieve physical disk list from controller. Command failed.";
                        PrintError(%rspext);
                        return ERROR;
                    }


                    
                    
                    for $i (0..$#{$rspext{LIST}})
                    {
                        my $idlst = $rspext{LIST}[$i];


                        ##############
                        #  PDISKINFO
                        ##############
                        #logInfo("Retrieving physical disk information for each disk");
                        %infoext = $ctlr->physicalDiskInfo($idlst);
                        if ( ! %infoext  )
                        {
                            logInfo(">>>>>>>> Failed to get response from physicalDiskInfo <<<<<<<<");
                            send_error_message "Could not retrieve physical disk information from controller. Empty response.";
                            return ERROR;
                        }
                        if ($infoext{STATUS} != PI_GOOD)
                        {
                            logInfo(">>>>>>>> Unable to physicalDiskInfo <<<<<<<<");
                            send_error_message "Could not retrieve physical disk information from controller. Command failed.";
                            PrintError(%infoext);
                            return ERROR;
                        }

                        #print "INFO:  $infoext{SES}  :  $infoext{SLOT}  :  $infoext{PD_DEVTYPE}  :   \n\n";
                        
                        #print %infoext;
                        #if  ($infoext{SES} == $id)
                        #{
                        #  if ($infoext{SES} == $id && $infoext{SLOT} == 0 && $infoext{PD_DEVTYPE} == 2 || $infoext{SES} == $id && $infoext{SLOT} == 15 && $infoext{PD_DEVTYPE} == 2 ) # SATA
                        #  {
                        #   $skipupdate = 1;
                        #   print "SATA Match Found     $infoext{SES}  :  $infoext{SLOT}  :  $infoext{PD_DEVTYPE} \n";
                        #  }
                        #}

                    }
                    
                    
                    # We're executing in "calculate only" mode, so we dont' want to actually perform
                    # any updates, we just want to see what work needs to be done
                    if ($executionMode == CALCULATE_ONLY)
                    {
                      $totalMatchingPdisks = $totalMatchingPdisks+1;
                      print "Found drive $id with firmware: $FWList[$j]. Current upgradable disk count: $totalMatchingPdisks";
                      
                      # Jump to the next item in the loop (skipping updating the drive)
                      next;
                    }
                    
                    if ($skipupdate == 0)
                    {
                       
                        #
                        # Run our generic status-checker just to make sure things are good
                        #

                        my $healthChecker = `./SystemStatusChecker -d $g_dsc_id -e $opErrorsFile -s $opStatusFile`;
                        my $healthCheckResult = $?;
                        if ($healthCheckResult != 0) {
                            print STDERR "SBOD Upgrader aborting due to system health issues.\n";
                            $flag = ERROR;
                            return ERROR;
                        } else {
                            print STDERR "System Status check complete. Systems is OK to proceed.";
                        }


                        send_updated_progress("Applying bay firmware $FWList[$j] to bay $id...");
                        $currentMatchingPdiskIndex = $currentMatchingPdiskIndex + 1;

                        forceLog("Drive/Bay Code on Device: $id with firmware: $FWList[$j]", LOG_TYPE_WARNING, $ctlr); 
                        $ret = SendCode2Drive( $ctlr, $id, $fnList[$j],
                                    $wwnLo, $wwnHi, $lun ) ;
                        if ( $ret != GOOD )
                        {
                             # First Attempt failed.  Try again
                             send_updated_progress("Applying bay firmware $FWList[$j] to device $id (second attempt)...");
                             forceLog( "   Drive/Bay Code on Device: $id - First Attempt failed.  Trying again...", LOG_TYPE_WARNING, $ctlr);
                             sleep 10;

                             $ret = SendCode2Drive( $ctlr, $id, $fnList[$j],
                                    $wwnLo, $wwnHi, $lun ) ;
                             if ( $ret != GOOD )
                             {
                                 # Second Attempt failed.  Try again
                                 send_updated_progress("Applying bay firmware $FWList[$j] to device $id (third attempt)...");
                                 forceLog( "   Drive/Bay Code on Device: $id - Second Attempt failed.  Trying again...", LOG_TYPE_WARNING, $ctlr);
                                 sleep 10;

                                 $ret = SendCode2Drive( $ctlr, $id, $fnList[$j],
                                        $wwnLo, $wwnHi, $lun ) ;
                                 if ( $ret != GOOD )
                                 {
                                     # Third Attempt failed.  Try again
                                     send_updated_progress("Applying bay firmware $FWList[$j] to device $id (last attempt)...");
                                     forceLog( "   Drive/Bay Code on Device: $id - Third Attempt failed.  Last try...", LOG_TYPE_WARNING, $ctlr);
                                     sleep 10;

                                     $ret = SendCode2Drive( $ctlr, $id, $fnList[$j],
                                            $wwnLo, $wwnHi, $lun ) ;
                                     if ( $ret != GOOD )
                                     {
                                          forceLog( ">>>>>>>> Failure updating code on device $id after 4 attempts<<<<<<<<", LOG_TYPE_ERROR, $ctlr);
                                          send_error_message("Failed to update code on device $id after 4 attempts.");
                                          $flag = ERROR;
                                          #return ERROR;
                                     }
                                      
                                      
                                 }
                             }

                        }

                        # handle a second file if present (only for drives)
                        if ( $type eq DRIVE )
                        {

                            if ( "none" ne $SCList[$j] )
                            {
                                $ret = SendCode2Drive( $ctlr, $id, $SCList[$j],
                                            $wwnLo, $wwnHi, $lun ) ;
                                if ( $ret != GOOD )
                                {
                                    print ">>>>>>>> Failure updating code on drive $j <<<<<<<<\n";
                                    forceLog( ">>>>>>>> Failure updating code on device $id <<<<<<<<", LOG_TYPE_ERROR, $ctlr);
                                    send_error_message("Failed to update code on drive $j.");
                                    $flag = ERROR;
                                    #return ERROR;
                                }
                            }
                        }
                    
                        if ( $type ne DRIVE )
                        {
                            print "  Sleeping 120 seconds...";
                            sleep 120;
                        }
                        # Check the BE Device paths prior to starting the main process.
    
                        $ret = check_bedevice($ctlr);
    
                        if ( GOOD != $ret )
                        {
                            forceLog("Drive/Bay Code Failed:  Back End Paths are not good - Please contact service", LOG_TYPE_ERROR, $ctlr);
                            send_error_message("Back-end paths are not good. Please contact customer service.");
                            return ERROR;
                        }
                        else
                        {
                            forceLog("Drive/Bay Code BEDevice check:  All paths operational", LOG_TYPE_INFO, $ctlr);
                        } 

                    }
                    else
                    {
                      #forceLog( "Drive/Bay Code on Device: $id  SATA Drives Detected - Skipping Upgrade", LOG_TYPE_WARNING, $ctlr);
                    }
                    
                }
                else
                {
                  if ($executionMode != UPDATE_CODE) #Don't log during calculation-mode (would cause double-logging)
                  {
                    forceLog("Drive/Bay Code on Device: $id Already at firmware: $FWList[$j] - Skipping", LOG_TYPE_INFO, $ctlr); 
                  }
                }
            }
            else
            {
              if ($executionMode != CALCULATE_ONLY) #Don't log during calculation-mode (would cause double-logging)
              {
                forceLog(" Drive/Bay Code - Device: $id Not valid bay type for upgrade - Skipping", LOG_TYPE_INFO, $ctlr);
              }
            }
        
            
            # If we're in "CALCULATE_ONLY", then we can ignore the rest of this for-loop
            if ($executionMode == CALCULATE_ONLY)
            {
              next;
            }
            
            $skipupdate = 0;
       
            # gotta force a rescan here since the labels may have changed since 
            # the last scan...

            $ret = RescanBE( $ctlr );          
            if( $ret == ERROR )
            {
                print ">>>>>>>> Failed to rescan devices. <<<<<<<<\n";
                # return (ERROR);
            }

              
            #Check to make sure the code took
            if ( $type eq DRIVE )
            {
                %info = $ctlr->physicalDiskInfo($id);
            }
            else
            {
                %info = $ctlr->diskBayInfo($id);
            }
            # end change

            if ( ! %info  )
            {
                logError( ">>>>>>>> Failed to get response from physicalDisk/bayInfo <<<<<<<<");
                send_error_message("Failed to get a response from physical disk / bay info.");
                return ERROR;
            }

            if ($info{STATUS} != PI_GOOD)
            {
                print ">>>>>>>> Unable to get physicalDisk/bayInfo <<<<<<<<\n";
                PrintError(%info);
                $flag = ERROR;
                next;
                #return ERROR;
            }

            $drv = $info{PS_PRODID};
            $drv = join ('', split(/\s+/,$drv));     # Remove any spaces
            $drv = lc($drv);
            $rev = $info{PD_REV};
            $wwnLo = $info{WWN_LO};
            $wwnHi = $info{WWN_HI};
            $lun = $info{PD_LUN};

            my $codeMatchChk = ( $rev =~ /$FWList[$j]/i );

            
            # Check product type and firmware match
            $drvmatch = 0;    
                
            if ($drv eq $drvList)
            {
              $drvmatch = 1;
            }
            
            if ( $drvmatch == 1 )
            {
                if ( $codeMatchChk != 1 )
                {
                  forceLog(" Drive/Bay Code - Device: $id FW after load does not match: current rev is $rev, needs to be $FWList[$j] - ABORTING", LOG_TYPE_ERROR, $ctlr);
                  send_error_message("Device $id: Firmware does not match after load. Current revision is $rev, needs to be $FWList[$j]. - ABORTING");
                  return ERROR; 
                }
            }
                    
            # Check the BE paths again        
            $ret = check_bedevice($ctlr);

            if ( GOOD != $ret )
            {
                forceLog("Drive/Bay Code Failed:  Back End Paths are not good - Please contact service", LOG_TYPE_ERROR, $ctlr);
                send_error_message("Drive/Bay Code Failed:  Back-end paths are not good. Please contact customer service.");
                return ERROR;
            }
            else
            {
                forceLog("Drive/Bay Code BEDevice check:  All paths operational", LOG_TYPE_INFO, $ctlr);
            } 
        
                
  
        }
        print "\n\n";
    }

    # If we're in "CALCULATE_ONLY" mode, don't do things like rescanning drives, etc.
    if ($executionMode == CALCULATE_ONLY)
    {
      return GOOD;
    }    

    # gotta force a rescan here since the labels may have changed since 
    # the last scan...

    $ret = RescanBE( $ctlr );          
    if( $ret == ERROR )
    {
        print ">>>>>>>> Failed to rescan devices. <<<<<<<<\n";
        # return (ERROR);
    }




    if ( $flag == ERROR )
    {
        print "##################################################### \n";
        print "##                                                 ## \n";
        print "##   One or more devices failed the update. Get    ## \n";
        print "##   them back online/operational and try again.   ## \n";
        print "##                                                 ## \n";
        print "##                                                 ## \n";
        print "##                                                 ## \n";
        print "##################################################### \n";

        forceLog("Error:  some devices were not updated. Please Contact Service", LOG_TYPE_ERROR, $ctlr);
        send_error_message("Some devices were not updated. Please contact customer service.");
        return ERROR;
    }



    return GOOD;
}

##############################################################################
#
#          Name: SendCode2Drive
#
#        Inputs: controller object, PDD number, partial file name
#                drive WWN(in 2 parts), lun number
#
#       Outputs: GOOD, if successful, ERROR otherwise
#
#  Globals Used: none
#
#   Description: sends code file to disk drive 
#
#
##############################################################################
sub SendCode2Drive
{

    my ( $ctlr, $pdd, $file, $wwnLo, $wwnHi, $lun) = @_;

    my @arr;
    my $filename;

    #
    # build the file name
    #

    # we are in the bigfoot\<rel>\test\ directory
    # so we need to go up 1 dir

    # want it to be  "..\$file.ima" 
    
    # changes for cfg file version (take out the hardcoded path info)
    # was:
    #
    #$filename = "..\\Drives\\Seagate\\" . $file;
    #
    # is now:
    $filename =  $file;


    #
    # verify file existence
    #
    if (! -r $filename)
    {
        print ">>>>>>>> Missing or unreadable file ($filename) <<<<<<<<\n";
        return ERROR;
    }

    #
    # build a data array
    #

    $arr[0]{WWN_LO} = $wwnLo;
    $arr[0]{WWN_HI} = $wwnHi;
    $arr[0]{PD_LUN} = $lun;

    #
    # send the file
    #
    
    printf "  Writing %s to %8.8x%8.8x \n", $filename, $wwnLo, $wwnHi;
    
     my %rsp = $ctlr->writeBuffer(1, $filename, @arr);
     
     if ( ! %rsp  )
     {
         print ">>>>>>>> Failed to get response from writeBuffer <<<<<<<<\n";
         return ERROR;
     }
     if ($rsp{STATUS} != PI_GOOD)
     {
         forceLog(">>>>>>>> Error sending code to device <<<<<<<<", LOG_TYPE_ERROR, $ctlr);
         send_error_message("Update failed: Error sending code to device.");
         PrintError(%rsp);
         return ERROR;
     }

     
   
    return GOOD;
}

##############################################################################
#
#          Name: PrintError
#
#        Inputs: ret (hash returned from a command)
#
#       Outputs: none
#
#  Globals Used: none
#
#   Description: Just prints the error fields in a has. This is frequently 
#                used code.   
#
#
##############################################################################
sub PrintError
{
    my (%ret) = @_;

    printf "Status Code:    0x%02x  ", $ret{STATUS};
    if (defined($ret{STATUS_MSG}))
    {
        printf " \"%s\"", $ret{STATUS_MSG};
    }
    printf "  Error Code:     0x%02x  ", $ret{ERROR_CODE};
 
    if (defined($ret{ERROR_MSG}))
    {
        printf " \"%s\"", $ret{ERROR_MSG};
    }
    print " \n";

    return;
}

#
# This subroutine will be used to process each config file.
#
sub process
{                         
my($filename, $input, $taskPtr, $driveListPtr, $FWlistPtr, $codeListPtr, $codeList2Ptr) = @_;
my $badCheck;
my $Section;
my @typeValues        = ("disk" , "bay");
no strict 'refs';

$input++;                           # this is a string increment
unless (open($input, $filename))
{ 
    print("\nCan't Open Config File $filename: $!\n");
    return ERROR;
}

my $parseError = FALSE;                    # Flag to indicate a parse error.
local $_;
my $lineNumber = 0;

LINE: while (<$input>)              # note use of indirection
{
    $lineNumber++;

    if ($_ =~ /^#include\s*"(.*)"/)
    {
        print ("Including Config File: $1\n"); 
        if (process($1, $input) == ERROR)
        {
            $parseError = TRUE;
        }                         
       
        next LINE;
    }
    chomp;

    # Skip blank lines and comment lines.
    if (($_ =~ /^\s*#/) || ($_ =~ /^\s*$/))
    {
        next LINE;
    }

    # convert everthing to lowercase
    #$_ = lc;                        

    # find the tag within brackets
    if ($_ =~ (/^\s*\[\s*(\w+)\s*\]/))
    {
        #
        # The tag has been extracted from the brackets.
        #  
    
            #
            # Check to see if the section is valid.
            #
            if ( lc($1) eq "type"  or 
                 lc($1) eq "drive" or
                 lc($1) eq "bay" )
            {
                $Section = lc($1); 
            }
            else
            {
                print ("Parse error - The specified section [$1] is unrecognized");
                print ("===>Filename: $filename, Line: $lineNumber");
                $parseError = TRUE;
            }
        next LINE;
    }

    ###################################
    # Parse the Config Information...
    ###################################
    my @inputValues = split;

    Section_Case:
    {
        # Act on Type Keywords
        $Section eq "type"  && do       
            {
                if (lc($inputValues[0]) eq "device")
                {
                    if (lc($inputValues[1]) ne "drive" and lc($inputValues[1]) ne "bay")
                    {
                        print("The specified control parameter ($inputValues[1]) must be either 'drive' or 'bay'\n");
                        return ERROR;
                    }
                    else
                    {
                        $$taskPtr = lc($inputValues[1]);
                    }
                }                    
                last Section_Case;
            };

        # Act on Drive Keywords

        $Section eq "drive"  && do       
            {
                if ($$taskPtr eq "drive")
                {
                    push @$driveListPtr, $inputValues[0];
                    push @$FWlistPtr, $inputValues[1];
                    push @$codeListPtr, $inputValues[2];
                    push @$codeList2Ptr, $inputValues[3];
                }
                else
                {
                    print ("Type is not Drive, skipping Drive configuration.\n");
                }
                last Section_Case;
            };

        # Act on Bay Keywords
        $Section eq "bay"  && do       
            {
                if ($$taskPtr eq "bay")
                {
                    push @$driveListPtr, $inputValues[0];
                    push @$FWlistPtr, $inputValues[1];
                    push @$codeListPtr, $inputValues[2];
                    push @$codeList2Ptr, $inputValues[3];
                }
                else
                {
                    print ("Type is not Bay, skipping Bay configuration.\n");
                }
                last Section_Case;
            };
    } # End of Section Case

    }
    unless (close $input)
    { 
        print ("Error Closing Config File $filename: $!");
    }

    #
    # Check to see if at least one parse error occurred.
    #
    if ($parseError == TRUE)
    {
        return ERROR;
    }

} # 

###############################################################################

##############################################################################
#
#          Name: RescanBE
#
#        Inputs: controllerID
#
#       Outputs: GOOD, if successful, ERROR otherwise
#
#  Globals Used: none
#
#   Description: Rescans the back end to see if the status of the drives has 
#                changed, This makes sure we are dealing with current 
#                information.  
#
#
##############################################################################
sub RescanBE
{
    my ($ctlr) = @_;
    my %rsp;

    print "Scanning the devices....\n";

    %rsp = $ctlr->rescanDevice("LIST");
    
    if ( ! %rsp  )
    {
        print ">>>>>>>> No response rescanning the BE <<<<<<<<\n";
        return (ERROR);
    }

    if ( $rsp{STATUS} != PI_GOOD )
    {
        print ">>>>>>>> Failed to rescan the drives <<<<<<<<\n";
        
        PrintError(%rsp);

        return (ERROR);
    }



    return (Wait4Ses($ctlr, 90));
    
    

}


##############################################################################
##############################################################################
#
#          Name: Wait4Ses
#
#        Inputs: controller object pointer timeout
#
#       Outputs: varies
#
#  Globals Used: none
#
#   Description: Waits until all rebuilds are complete
#
##############################################################################
sub Wait4Ses
{
    my ($ctlr, $timeout ) = @_;
    my %pdisks;
    my $noSesCount;
    my $goodCount;
    my $i;
    my $happyCount;

    #############################
    # wait for rebuild to finish
    #############################
    # rebuild is done if no drives are 'degraded'

    print("Polling for good SES data\n");

    $happyCount = 0;

    while ( $timeout-- > 0 )
    {
        
        %pdisks = $ctlr->physicalDisks();
        
        if (! %pdisks )        
        {
            print (">>>>>>>>>> no response from pdisks in Wait4Ses <<<<<<<<<<<\n");
            return ERROR;
        }

        if ( $pdisks{STATUS} != PI_GOOD )
        {
            print (">>>>>>>>>> Error from pdisks in Wait4Ses <<<<<<<<<<<\n");
            PrintError(%pdisks);
            return ERROR;
        }

        # data is good, now scan it

        $noSesCount = 0;
        $goodCount = 0;

        for ( $i = 0; $i <$pdisks{COUNT} ; $i++ )
        {
            if ( $pdisks{PDISKS}[$i]{PD_DEVSTAT} == 0x10 )      # only look at operational drives
            {
                $goodCount++;

                if ( $pdisks{PDISKS}[$i]{SES} == 0xffff )
                {
                    $noSesCount++;
                }
            }
        }

        if ( ($noSesCount == 0) && ($goodCount > 0) )
        {

            # since multiple passes may be done, look for three 
            # consecutive good ones

            $happyCount++;

            if ( $happyCount > 3 )
            {
                print("Good SES poll complete\n");
                
                return GOOD;
            }
            
        }
        else
        {
            # reset good count if we get a bad one
            $happyCount = 0;
        }    
            
        sleep 1;
    }
    
    print(" Wait for SES: timed out. Number good drives = $goodCount. Number with no SES = $noSesCount.\n");
    
    return INVALID;
}

sub forceLog
{
  my ($msg, $type_mess, $currentMgr) = @_;

  my %rsp = $currentMgr->logTextMessage($msg, $type_mess);

    if (%rsp)
    {
        if ($rsp{STATUS} == PI_GOOD)
        {
            print "Log message sent successfully.\n";
        }
        else
        {
            my $msg = "Unable to send log message.";
            displayError($msg, %rsp);
        }
    }
    else
    {
        print "ERROR: Did not receive a response packet.\n";
        logout();
    }
 return GOOD;       
}

#############################
#
#############################
sub getSESDname
{

        trace();
        my ($key) = @_ ;
        my %dName = (1,'B',2,'C',3,'D',4,'E',5,'F',
        6,'G',7,'H',8,'I',9,'J',10,'K',
        11,'L',12,'M',13,'N',14,'O',15,'P',
        16,'Q',17,'R', ); 
        
        return ($dName{$key});

}

#
# Check the BE device paths.  
#

sub check_bedevice
{
    my ($dscs) = @_;
    my @controller_ips = split(/\s+/, $dscs);
    #my $ips = join(':3000 ', @controller_ips) . ':3000';
    my $ccbe_output;


   use strict;

    
    # need a longer timeout for this
    my $value = 1200;
    print "Setting MRP timeout to $value seconds\n";
    my %rsp = $dscs->timeoutMRP("MRP", $value);
    if (!%rsp)                        # no response
    {
        print ">>>>>>>> Failed to receive a response from timeoutMRP <<<<<<<<\n";
        return ERROR;
    }

    if ($rsp{STATUS} == 1)            # 1 is bad
    {
        
        print ">>>>>>>> Failed: timeoutMRP returned an error <<<<<<<<\n";
        
        PrintError(%rsp);
        
        return ERROR;
    }

    $ccbe_output = $dscs->PATH_PHYSICAL_DISK("PDISK", "BP");

    
    # Check for N=1 or N=2
    
    # Paths are 3 or C for dual connection on N=2
    my @ccbe_output = split(/\n/, $ccbe_output);
    my $bad_pids;
    my $count = 0;
    foreach my $i (@ccbe_output) {
        if ($i =~ /^ *(0[0-9A-F]*) *000([0-9A-F])/) {
            my $pid = $1;
            my $ports = $2;
            if ($ports ne '3' && $ports ne 'C') {
                $count++;
                if (!defined($bad_pids)) {
                    $bad_pids = "$pid";
                } else {
                    $bad_pids .= ",$pid";
                }
            }
        }
    }
    if ($count == 0) {
        return GOOD;              # Flag everything ok.
    }    
    
    return ERROR;
}   # End of check_bedevicepaths_ok

##############################################################################
# FindMasterIndex
##############################################################################
sub FindMasterIndex
{
    my ($nodeArray) = @_;
    my $rc = GOOD;
    my $indexCounter = 0;
    my $masterIndex = -1;
    my $dscId = -1;

    ##
    # Ask for this controller's VCG information
    ##
    my %vcgInfoRsp = $nodeArray->vcgInfo(0);
    my $msg = $nodeArray->displayVCGInfo(%vcgInfoRsp);
    print $msg;


    #foreach ( keys(%$nodeArray) ){
    #  print "$_ => %$nodeArray->{$_} \n";


    print "$nodeArray->{HOST} \n";
    my $modVCGIP = "$vcgInfoRsp{VCG_IP_ADDRESS}:3000";
    print "$modVCGIP \n";
    
    $dscId = "$vcgInfoRsp{VCG_ID}";
    print "$dscId\n";


    if( %vcgInfoRsp )
    {
        ##
        # If this node's IP address is the same as the DSC's address,
        # then we've found the master controller node.
        ##
        if($nodeArray->{HOST} eq $modVCGIP)

        {
            $masterIndex = 1;
        }
    }
   

    ##
    # Verify that the master node was located
    ##
    if( ($masterIndex < 0) || 
        ($masterIndex > scalar($nodeArray)) )
    {
        print( "    This is the slave\n" );
        $rc = ERROR;
    }

    return( $rc, $masterIndex, $dscId );
  
}

###################################
#
# $Log$
# Revision 1.4  2006/05/25 02:35:05  EidenN
# no message
#
# Revision 1.3.20.1  2006/05/22 10:27:01  deepakrc
#
# Fix for Tbolt00014402.
#
# Change the include directory from "ccbe" to "CCBE", since the file-system namespace
# is case-sensitive in linux.
#
# Revision 1.3  2005/08/05 18:23:59  WilliamsJ
# TBolt00000000 - add conditional on the sleep.
#
# Revision 1.2  2005/08/05 16:11:07  WilliamsJ
# TBolt00000000 - Added 60 second delay between loads to avoid CRC errors
# on Xyratex boxes.
#
# Revision 1.1.1.1  2005/05/04 18:53:51  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.22  2005/03/11 16:58:11  EidenN
# TBolt00012542:  Removed the pattern search, replace was an exact match search.  Removed whitespaces from the drvlist. Reviewed by:  MenningC
#
# Revision 1.21  2005/01/17 17:24:18  EidenN
# Tbolt0000:  No change training Bruce
#
# Revision 1.20  2003/04/15 14:36:47  MenningC
# tbolt00000000: move 600 and 3100 to constants; reviewed by JW
#
# Revision 1.19  2003/03/26 21:23:47  WerningJ
# Removed calls to unused modules
# Reviewed by Craig M
#
# Revision 1.18  2003/02/12 23:41:04  WerningJ
# Removed spaces from model name
# reviewed by Craig M
#
# Revision 1.17  2003/02/12 21:22:41  MenningC
# Tbolt00000000 put in the CRs
#
# Revision 1.16  2003/02/12 21:16:25  MenningC
# Tbolt00000000 change loginfo to print
#
# Revision 1.15  2003/02/07 17:24:07  MenningC
# Tbolt00000000: updates for SES changes. reveiwed by Jeff Werning.
#
# Revision 1.14  2003/02/05 22:45:55  MenningC
# Tbolt00000000:added delay after rescan. reviewed by Jeff Werning
#
# Revision 1.13  2003/02/05 22:23:26  MenningC
# Tbolt00000000:added pod. reviewed by Max
#
# Revision 1.12  2002/12/04 15:55:36  MenningC
# tbolt00000000 Debugged the changes. reviewed by  Jeff W.
#
# Revision 1.11  2002/12/03 22:12:02  WerningJ
# Added parsing of config file for Drive and Bay code input
# Reviewed by Craig M
#
# Revision 1.10  2002/10/10 21:28:01  MenningC
# tbolt00000000  new 73G code per Neal, Olga reviewed it.
#
# Revision 1.9  2002/10/09 15:12:30  MenningC
# tbolt00000000 update 146 GB , reviewed by J Werning
#
# Revision 1.8  2002/09/16 14:55:59  MenningC
# tbolt00000000 chenged 73 GB version; reviewed by J Werning
#
# Revision 1.7  2002/08/16 18:31:28  MenningC
# Tbolt00000000 change 146 GB drive FW
#
# Revision 1.6  2002/07/31 19:44:17  HouseK
# Result of merge from tag LOGGING_CHANGES
#
# Revision 1.5.2.2  2002/07/30 15:18:05  HouseK
# Reverted back to 1.5
#
# Revision 1.5  2002/07/24 18:10:51  MenningC
# Tbolt00000000 updated for new 36 gb drivecode. Reviewed by Brett.
#
# Revision 1.4  2002/06/19 14:12:48  MenningC
# Tbolt00000000 updates for M500 license changes. reviewed by Chris.
#
# Revision 1.3  2002/06/12 20:58:21  WerningJ
# TBOLT00000000: added new files for other drives, files in new directory. Reviewed by J Werning.
#
# Revision 1.2  2002/06/12 15:14:10  WerningJ
# TBOLT00000000: handle errors differently, rescan when done. Reviewed by J Werning.
#
# Revision 1.1  2002/06/10 19:44:46  WerningJ
# TBOLT00000000: script for updating drive code, reviewed by J Werning
#
#
#
# 
#
