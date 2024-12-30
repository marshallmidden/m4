#! /user/bin/perl -I /opt/xiotech/CCBE -I TestLibs -w
##############################################################################
#
# Magnitude 3D disk updater script
#
# driveCodeLoad.pl - A script to upload code to the drives and drive bays 
# attached to a controller.
#
# drivecodeload.pl <ip addr> <opErrorsFile> <opStatusFile> <cfg file(s)><return>
#
# where:
#    <ip addr> is the IP address of a controller
#    <opErrorsFile> path to the OpErrors.txt file that we write errors into
#    <opStatusFile> path to the OpStatus.txt file that we write % complete and status strings to
#    <cfg file> is the name of a config file that is used to configure the 
#               script.
#
#    All parameters are required. 
#     
# Typical use (with fake ip address):
#
#    drivecodeload.pl 10.64.99.341  ../OpErrors.txt ../OpStatus.txt SampleDCL <return>
#
# Notes:
#    Derived from the SBOD bay updater which supposedly supported drives -- found out
#    that it did NOT work for drives.  Heavily modified to work with drives.
#
#    ** NOT TESTED FOR BAY UPDATES **
#
##############################################################################

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

use FileHandle;
use strict;
use warnings;

#---Autoflush all stdout writes

STDOUT->autoflush(1);
STDERR->autoflush(1);

DriveFwLoader();

exit 0;



##############################################################################
#
#          Name: SendUpdatedProgress
#
#        Inputs: Status string
#
#       Outputs: none
#
#  Globals Used: totalDrives, drivesUpdated
#
#   Description: Outputs a the current status to the OpStatus.txt file
#
##############################################################################
sub SendUpdatedProgress($)
{
  my $statusString = $_[0];
  print STDOUT "SendUpdatedProgress: $statusString\n";
  
  #
  # Calculate this script's percent complete
  #
  
  my $percent_complete = 0;
  if ($totalMatchingPdisks > 0)
  {
    $percent_complete = $currentMatchingPdiskIndex / $totalMatchingPdisks * 100.0;
  }
  
  open (STATUS_FILE, ">$opStatusFile") or print STDERR "SendUpdatedProgress: Can't open file '$opStatusFile' $!\n"; 
  print (STATUS_FILE "PercentComplete=$percent_complete\nStatus=$statusString");
  close (STATUS_FILE);
  
  print STDOUT "SendUpdatedProgress: Total Disks: $totalMatchingPdisks, Current: $currentMatchingPdiskIndex, Percent Complete: $percent_complete\n";
}



##############################################################################
#
#          Name: SendErrorMessage
#
#        Inputs: string to output to a file
#
#       Outputs: none
#
#  Globals Used: none
#
#   Description: Outputs a given string to the OpErrors.txt file
#
##############################################################################
sub SendErrorMessage($)
{
  my $message = $_[0];

  print STDOUT "SendErrorMessage: $message\n";
  open (ERRORS_FILE, ">>$opErrorsFile") or print STDERR "SendErrorMessage: Can't open file '$opErrorsFile' $!\n";
  print (ERRORS_FILE "$message\n");
  close (ERRORS_FILE);
}



##############################################################################
#
#          Name: DriveFwLoader
#
#        Inputs: none
#
#       Outputs: none
#
#  Globals Used: none
#
#   Description: The front end for use with config files
#
##############################################################################
sub DriveFwLoader
{
  my $i;
  my $ipAddr;        # master's IP
  my $ret;           # a returned value
  my $master;        # object for controller
  my $inf;
  my $cfgFile;
  my $task; 
  my @driveList;
  my @fromFwList;
  my @fwList;
  my @codeList;
  my @codeList2;

  my $fileParseError = FALSE;

  # passed on the cmdline or not.
  
  # The call should look like this:
  # DriveCodeLoad.pl <ip addr> <opErrorsFile> <opStatusFile> <cfg file> 

  my $numElements = $#ARGV + 1;
  if ($numElements >= 4)
  {
    # Get the command line parm and see if an IP address
    ($inf) = $ARGV[0];

    # If its an IP address, we will connect to it automatically
    if ($inf =~ /^\d{1,3}?\.\d{1,3}?\.\d{1,3}?\.\d{1,3}?:*\d*$/)
    {
      $ipAddr = $inf;
    }
    else        # parm not an IP address
    {
      print "DriveFwLoader: $inf\n";
      SendErrorMessage("Unable to determine master controller IP address from file.");
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
    # @driveList, @fromFwList, @fwList, @codeList, @codeList2, $task
    #
    #
    #  @driveList is a list of the drive identifier strings eg. "ST336752FC"
    #
    #  @fromFwList is a list of valid firmware versions we can upgrading from.
    #
    #  @FwList is a list of code revisions eg.  "F103"
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
    
    print "DriveFwLoader: Parsing config files...\n";
    
    no strict 'refs';

    #
    # Process the argument list of files along with any includes.
    #
    foreach $cfgFile (@ARGV) 
    {
      if (ProcessConfigFile($cfgFile, 'fh00', \$task, \@driveList, \@fromFwList, \@fwList, \@codeList, \@codeList2) == ERROR)
      {
        $fileParseError = TRUE;
      }
    }

    if ($fileParseError == TRUE)
    {
      SendErrorMessage("Error parsing bay-update configuration file.");
      return ERROR;
    }            
  }
  else # wrong number of parms
  {
    SendErrorMessage("Insufficient arguements to DriveCodeLoad.pl");
    return ERROR;;
  }

  #
  # NOTE: Currently only EXACTLY one IP address is supported
  #

  #
  # establish controller connections
  #

  $master = XIOTech::cmdMgr->new(\*STDOUT);

  $ret =  $master->login( $ipAddr, CCB_PORT);       # connect to controller
  if ( ! $ret  )
  {
    print "DriveFwLoader: >>>>>>>> Failed to connect to $ipAddr  <<<<<<<<\n";
    return (ERROR);
  }

  # Check to see if this is really the master.  Otherwise return GOOD for slave.  Only needs to run once
    
  ( my $rc, my $masterIndex, my $dscId) = FindMasterIndex( $master );
  $g_dsc_id = $dscId;
  SendUpdatedProgress("DSC $dscId: Checking back-end device paths...");
    
  if( $rc == ERROR )
  {
    # This is not the Master controller.  Return Good to not run this twice by mistake.
    return (GOOD);
  }
    
  #---Set TimeOut
  $master->{TIMEOUT} = 700;

  # Send start message
  ForceLog("Mag 3D disk updater started", LOG_TYPE_WARNING, $master);
  sleep 10;

  # Check the BE Device paths prior to starting the main process.
    
  $ret = CheckBeDevice($master);
    
  if ( GOOD != $ret )
  {
    ForceLog("Mag 3D disk updater failed, back-end path issue detected. Please contact service", 
             LOG_TYPE_ERROR, $master);
    SendErrorMessage("Back-end path issue detected. Please contact customer service.");
    return ERROR;
  }
  else
  {
    ForceLog("Mag 3D disk updater BEDevice check, all paths operational", LOG_TYPE_INFO, $master);
  }       

  #
  # now do the update
  #

  #    my @driveList = ( "ST3146855FC",        "ST3300655FC",        "ST373455FC" );
  #
  #    my @fromFwList =( "XR50,XR52,XR55",     "XR50,XR52,XR55",     "XR50,XR52,XR55" );
  #
  #    my @fwList =    ( "XR56",               "XR56" ,              "XR56" );
  #
  #    my @codeList =  ( "XR56_C591_146G.ima", "XR56_B591_300G.ima", "XR56_D591_73G.ima" );
  #    
  #    my @codeList2=  ( "none",               "none",               "none" );
  #

  #
  # Toggle into a mode which doesn't perform any updates, but checks to see how much work
  # there is to do
  #
  
  print STDOUT "DriveFwLoader: Setting 'CALCULATE_ONLY' mode\n";
  SendUpdatedProgress("DSC $dscId: Analyzing physical disks (this may take a while)...");
  $executionMode = CALCULATE_ONLY;
  $ret = UpdateDriveCode($master, $task, \@driveList, \@fromFwList, \@fwList, \@codeList, \@codeList2);
  
  #
  # Toggle into a mode where we'll actually perform the updates, and run it again
  #
      
  print STDOUT "DriveFwLoader: Setting 'UPDATE_CODE' mode\n";
  $executionMode = UPDATE_CODE;
  $ret = UpdateDriveCode($master, $task, \@driveList, \@fromFwList, \@fwList, \@codeList, \@codeList2);
  if ( GOOD != $ret )
  {
    ForceLog("Mag 3D disk updater failed", LOG_TYPE_ERROR, $master);
    SendErrorMessage("Mag 3D disk updater failed.");
    return ERROR;
  }
  else
  {
    ForceLog("Mag 3D disk updater complete", LOG_TYPE_WARNING, $master);
  }    

  #
  # terminate controller connection
  #

  $master->logout();

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
##############################################################################
sub UpdateDriveCode
{
  my ( $ctlr, $type ,$drivePtr, $fromFwPtr, $fwPtr, $fnPtr, $scPtr) = @_;

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
  my @fromFwList;
  my @fwList;
  my $wwnLo;
  my $wwnHi;
  my $wwnString;
  my $lun;
  my $flag;
  my @fnList;
  my @scList;
  my $drvList;
  my $msg;
  my $type_mess;
  my %rspext;
  my %infoext;

  $flag = GOOD;

  @driveList = @$drivePtr; 
  @fromFwList = @$fromFwPtr; 
  @fwList = @$fwPtr; 
  @scList = @$scPtr; 
  @fnList = @$fnPtr;

  use strict;
  
  # need a longer timeout for this
  my $value = 1200;
  %rsp = $ctlr->timeoutMRP("MRP", $value);
  if (!%rsp)                        # no response
  {
    print "UpdateDriveCode: >>>>>>>> Failed to receive a response from timeoutMRP <<<<<<<<\n";
    return ERROR;
  }

  if ($rsp{STATUS} == 1)            # 1 is bad
  {
    print "UpdateDriveCode: >>>>>>>> Failed: timeoutMRP returned an error <<<<<<<<\n";
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
    print "UpdateDriveCode: >>>>>>>> Failed to get response from physicalDiskList <<<<<<<<\n";
    return ERROR;
  }
  if ($rsp{STATUS} != PI_GOOD)
  {
    print "UpdateDriveCode: >>>>>>>> Unable to physicalDiskList <<<<<<<<\n";
    PrintError(%rsp);
    return ERROR;
  }

  print "UpdateDriveCode: Getting physical disk list.\n";

  for $i (0..$#{$rsp{LIST}})
  {
    print "UpdateDriveCode: Examining device " . $rsp{LIST}[$i] . "\n";
    $id =  $rsp{LIST}[$i];

    #############################
    #  PDISKINFO or DISKBAYINFO
    #############################
    print "UpdateDriveCode: Retrieving physical information for each disk.\n";

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
      print "UpdateDriveCode: >>>>>>>> Failed to get response from physicalDisk/bayInfo <<<<<<<<\n";
      SendErrorMessage("Failed to get bay and physical disk information.");
      return ERROR;
    }

    if ($info{STATUS} != PI_GOOD)
    {
      print "UpdateDriveCode: >>>>>>>> Unable to get physicalDisk/bayInfo <<<<<<<<\n";
      PrintError(%info);
      $flag = ERROR;
      next;
    }

    $drv = $info{PS_PRODID};
    $drv = join ('', split(/\s+/,$drv));     # Remove any spaces
    $drv = lc($drv);
    $rev = $info{PD_REV};
    $wwnLo = $info{WWN_LO};
    $wwnHi = $info{WWN_HI};
    $lun = $info{PD_LUN};

    print  "UpdateDriveCode: PS_PRODID: $info{PS_PRODID}\n";
    printf "UpdateDriveCode: WWN:       %8.8x%8.8x\n", $info{WWN_LO}, $info{WWN_HI};
    printf "UpdateDriveCode: PD_LUN:    %hu\n", $info{PD_LUN};
    
    $wwnString = sprintf("%8.8x%8.8x", $info{WWN_LO}, $info{WWN_HI});

    for ( $j = 0; $j < scalar(@fwList); $j++ )
    {
      my $drvMatch = 0;
        
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
        $drvMatch = 1;
      }

      if ( $drvMatch == 1 )
      {
        print "UpdateDriveCode: Device ID matched, index = $j, old rev is $rev, needs to be $fwList[$j]\n";

        #
        # now we need to update the code
        #

        my $updateRequired = 0;

        if (!( $rev =~ /$fwList[$j]/i ))
        {
          my @validFromFw = split(/,/,$fromFwList[$j]);
          foreach my $fw (@validFromFw)
          {
            if ( $rev =~ /$fw/i )
            { 
              $updateRequired = 1;
              last;
            }
          }  
        }

        if ( $updateRequired == 1 )
        {
          %rspext = $ctlr->physicalDiskList();
          if ( ! %rspext  )
          {
            print "UpdateDriveCode: >>>>>>>> Failed to get response from physicalDiskList <<<<<<<<\n";
            SendErrorMessage("Could not retrieve physical disk list from controller. Empty response");
            return ERROR;
          }

          if ($rspext{STATUS} != PI_GOOD)
          {
            print "UpdateDriveCode: >>>>>>>> Unable to physicalDiskList <<<<<<<<\n";
            SendErrorMessage("Could not retrieve physical disk list from controller. Command failed.");
            PrintError(%rspext);
            return ERROR;
          }

          for $i (0..$#{$rspext{LIST}})
          {
            my $idlst = $rspext{LIST}[$i];

            ##############
            #  PDISKINFO
            ##############

            %infoext = $ctlr->physicalDiskInfo($idlst);
            if ( ! %infoext  )
            {
              print "UpdateDriveCode: >>>>>>>> Failed to get response from physicalDiskInfo <<<<<<<<\n";
              SendErrorMessage("Could not retrieve physical disk information from controller. Empty response.");
              return ERROR;
            }
            if ($infoext{STATUS} != PI_GOOD)
            {
              print "UpdateDriveCode: >>>>>>>> Unable to physicalDiskInfo <<<<<<<<\n";
              SendErrorMessage("Could not retrieve physical disk information from controller. Command failed.");
              PrintError(%infoext);
              return ERROR;
            }
          }
          
          # We're executing in "calculate only" mode, so we dont' want to actually perform
          # any updates, we just want to see what work needs to be done
          if ($executionMode == CALCULATE_ONLY)
          {
            $totalMatchingPdisks = $totalMatchingPdisks+1;
            print "UpdateDriveCode: Found disk $wwnString with firmware: $fwList[$j]. Current upgradable disk count: $totalMatchingPdisks\n\n";
            
            # Jump to the next item in the loop (skipping updating the disk)
            next;
          }
          
          SendUpdatedProgress("Applying $fwList[$j] to disk $wwnString...");
          $currentMatchingPdiskIndex = $currentMatchingPdiskIndex + 1;
           
          #
          # Run our generic status-checker just to make sure things are good
          #
          # Note: not passing opStatusFile on purpose.  The percentage gets lost otherwise.
          #

          my $healthChecker = `./SystemStatusChecker -d $g_dsc_id -e $opErrorsFile`;
          my $healthCheckResult = $?;
          if ($healthCheckResult != 0) 
          {
            print STDERR "UpdateDriveCode: Updater aborting due to system health issues.\n";
            $flag = ERROR;
            return ERROR;
          } else {
            print STDERR "UpdateDriveCode: System Status check complete. Systems is OK to proceed.\n";
          }

          ForceLog("Mag 3D disk updater on disk $wwnString with firmware $fwList[$j]", LOG_TYPE_WARNING, $ctlr); 
          $ret = SendCodeToDrive( $ctlr, $id, $fnList[$j], $wwnLo, $wwnHi, $lun ) ;
          if ( $ret != GOOD )
          {
            # First attempt failed.  Try again
            SendUpdatedProgress("Applying $fwList[$j] to disk $wwnString (second attempt)...");
            ForceLog("Mag 3D disk updater on disk $wwnString, first attempt failed. Trying again...", 
                     LOG_TYPE_WARNING, $ctlr);
            sleep 10;

            $ret = SendCodeToDrive( $ctlr, $id, $fnList[$j], $wwnLo, $wwnHi, $lun ) ;
            if ( $ret != GOOD )
            {
              # Second attempt failed.  Try again
              SendUpdatedProgress("Applying $fwList[$j] to disk $wwnString (third attempt)...");
              ForceLog("Mag 3D disk updater on disk $wwnString, second attempt failed. Trying again...", 
                       LOG_TYPE_WARNING, $ctlr);
              sleep 10;

              $ret = SendCodeToDrive( $ctlr, $id, $fnList[$j], $wwnLo, $wwnHi, $lun ) ;
              if ( $ret != GOOD )
              {
                # Third attempt failed.  Try again
                SendUpdatedProgress("Applying $fwList[$j] to disk $wwnString (last attempt)...");
                ForceLog("Mag 3D disk updater on disk $wwnString, third attempt failed. Last try...", 
                         LOG_TYPE_WARNING, $ctlr);
                sleep 10;

                $ret = SendCodeToDrive( $ctlr, $id, $fnList[$j], $wwnLo, $wwnHi, $lun ) ;
                if ( $ret != GOOD )
                {
                  ForceLog("Mag 3D disk updater failed on disk $wwnString after 4 attempts", LOG_TYPE_ERROR, $ctlr);
                  SendErrorMessage("Failed to update code on disk $wwnString after 4 attempts.");
                  $flag = ERROR;
                }
              }
            }
          }

          # handle a second file if present (only for disk)
          if ( $type eq DRIVE )
          {
            if ( "none" ne $scList[$j] )
            {
              $ret = SendCodeToDrive( $ctlr, $id, $scList[$j], $wwnLo, $wwnHi, $lun ) ;
              if ( $ret != GOOD )
              {
                print "UpdateDriveCode: >>>>>>>> Failure updating code on disk $wwnString <<<<<<<<\n";
                ForceLog("Mag 3D disk updater failed on disk $wwnString", LOG_TYPE_ERROR, $ctlr);
                SendErrorMessage("Failed to update code on disk $wwnString.");
                $flag = ERROR;
              }
            }
          }
        
          $ret = RescanBE( $ctlr );
          if( $ret == ERROR )
          {
            print "UpdateDriveCode: >>>>>>>> Failed to rescan devices. <<<<<<<<\n";
          }

          # Check the BE Device paths prior to starting the main process.
  
          $ret = CheckBeDevice($ctlr);
  
          if ( GOOD != $ret )
          {
            ForceLog("Mag 3D disk updater failed, back-end path issue detected. Please contact service", LOG_TYPE_ERROR, $ctlr);
            SendErrorMessage("Back-end path issue detected. Please contact customer service.");
            return ERROR;
          }
          else
          {
            ForceLog("Mag 3D disk updater BEDevice check, all paths operational", LOG_TYPE_INFO, $ctlr);
          } 
        }
        else
        {
          if ($executionMode != UPDATE_CODE) #Don't log during calculation-mode (would cause double-logging)
          {
            ForceLog("Mag 3D disk updater on disk $wwnString, already at firmware $fwList[$j]. Skipping", LOG_TYPE_INFO, $ctlr); 
#            last;
          }
        }
      }
      
      # If we're in "CALCULATE_ONLY", then we can ignore the rest of this for-loop
      if ($executionMode == CALCULATE_ONLY)
      {
        next;
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
        print "UpdateDriveCode: >>>>>>>> Failed to get response from physicalDisk/bayInfo <<<<<<<<\n";
        SendErrorMessage("Failed to get a response when querying for disk information.");
        return ERROR;
      }

      if ($info{STATUS} != PI_GOOD)
      {
        print "UpdateDriveCode: >>>>>>>> Unable to get disk information <<<<<<<<\n";
        PrintError(%info);
        $flag = ERROR;
        next;
      }

      $drv = $info{PS_PRODID};
      $drv = join ('', split(/\s+/,$drv));     # Remove any spaces
      $drv = lc($drv);
      $rev = $info{PD_REV};
      $wwnLo = $info{WWN_LO};
      $wwnHi = $info{WWN_HI};
      $wwnString = sprintf("%8.8x%8.8x", $info{WWN_LO}, $info{WWN_HI});
      $lun = $info{PD_LUN};

      my $updateFwRequired = 0;

      if (!( $rev =~ /$fwList[$j]/i ))
      {
        my @validFromFw = split(/,/,$fromFwList[$j]);
        foreach my $fw (@validFromFw)
        {
          if ( $rev =~ /$fw/i )
          {
            $updateFwRequired = 1;
            last;
          }
        }
      }

      
      # Check product type and firmware match
      $drvMatch = 0;    
          
      if ($drv eq $drvList)
      {
        $drvMatch = 1;
      }
      
      if ( $drvMatch == 1 )
      {
        if ( $updateFwRequired == 1 )
        {
          ForceLog("Mag 3D disk updater on disk $wwnString, firmware after load does not match: current rev is $rev, needs to be $fwList[$j]. ABORTING", LOG_TYPE_ERROR, $ctlr);
          SendErrorMessage("Disk $wwnString: Firmware does not match after load. Current revision is $rev, needs to be $fwList[$j]. ABORTING");
          return ERROR; 
        }
      }
              
      # Check the BE paths again        
      $ret = CheckBeDevice($ctlr);

      if ( GOOD != $ret )
      {
        ForceLog("Mag 3D disk updater failed, back-end path issue detected. Please contact service", LOG_TYPE_ERROR, $ctlr);
        SendErrorMessage("Disk update failed, back-end path issue detected. Please contact customer service.");
        return ERROR;
      }
      else
      {
        ForceLog("Mag 3D disk updater BEDevice check, all paths operational", LOG_TYPE_INFO, $ctlr);
      } 
    }
  }

  # If we're in "CALCULATE_ONLY" mode, don't do things like rescanning disks, etc.
  if ($executionMode == CALCULATE_ONLY)
  {
    return GOOD;
  }    

  # gotta force a rescan here since the labels may have changed since 
  # the last scan...

  $ret = RescanBE( $ctlr );          
  if( $ret == ERROR )
  {
    print "UpdateDriveCode: >>>>>>>> Failed to rescan disks. <<<<<<<<\n";
  }

  if ( $flag == ERROR )
  {
    ForceLog("Mag 3D disk updater, some disks were not updated. Please Contact Service", LOG_TYPE_ERROR, $ctlr);
    SendErrorMessage("Some disks were not updated. Please contact customer service.");
    return ERROR;
  }

  return GOOD;
}



##############################################################################
#
#          Name: SendCodeToDrive
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
##############################################################################
sub SendCodeToDrive
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
    print "SendCodeToDrive: >>>>>>>> Missing or unreadable file ($filename) <<<<<<<<\n";
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
  
  printf "SendCodeToDrive: Writing %s to %8.8x%8.8x\n", $filename, $wwnLo, $wwnHi;
  
  my %rsp = $ctlr->writeBuffer(1, $filename, @arr);
  
  if ( ! %rsp  )
  {
    print "SendCodeToDrive: >>>>>>>> Failed to get response from writeBuffer <<<<<<<<\n";
    return ERROR;
  }

  if ($rsp{STATUS} != PI_GOOD)
  {
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
##############################################################################
sub PrintError
{
  my (%ret) = @_;

  printf "PrintError: Status Code: 0x%02x\n", $ret{STATUS};
  if (defined($ret{STATUS_MSG}))
  {
    printf "PrintError: \"%s\"\n", $ret{STATUS_MSG};
  }
  printf "PrintError: Error Code: 0x%02x\n", $ret{ERROR_CODE};
 
  if (defined($ret{ERROR_MSG}))
  {
    printf "PrintError: \"%s\"\n", $ret{ERROR_MSG};
  }

  return;
}



#
# This subroutine will be used to process each config file.
#
sub ProcessConfigFile
{                         
  my($filename, $input, $taskPtr, $driveListPtr, $fromFwListPtr, $fwListPtr, $codeListPtr, $codeList2Ptr) = @_;
  my $badCheck;
  my $Section;
  my @typeValues = ("disk" , "bay");
  no strict 'refs';

  $input++;                           # this is a string increment
  unless (open($input, $filename))
  { 
    print "ProcessConfigFile: Can't Open Config File $filename: $!\n";
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
      print "ProcessConfigFile: Including Config File: $1\n"; 
      if (ProcessConfigFile($1, $input) == ERROR)
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
        print "ProcessConfigFile: Parse error - The specified section [$1] is unrecognized\n";
        print "ProcessConfigFile: Filename: $filename, Line: $lineNumber\n";
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
            print "ProcessConfigFile: The specified control parameter ($inputValues[1]) must be either 'drive' or 'bay'\n";
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
          push @$fromFwListPtr, $inputValues[1];
          push @$fwListPtr, $inputValues[2];
          push @$codeListPtr, $inputValues[3];
          push @$codeList2Ptr, $inputValues[4];
        }
        else
        {
          print "ProcessConfigFile: Type is not drive, skipping drive configuration.\n";
        }
        last Section_Case;
      };

      # Act on Bay Keywords
      $Section eq "bay"  && do       
      {
        if ($$taskPtr eq "bay")
        {
          push @$driveListPtr, $inputValues[0];
          push @$fromFwListPtr, $inputValues[1];
          push @$fwListPtr, $inputValues[2];
          push @$codeListPtr, $inputValues[3];
          push @$codeList2Ptr, $inputValues[4];
        }
        else
        {
          print "ProcessConfigFile: Type is not Bay, skipping Bay configuration.\n";
        }
        last Section_Case;
      };

    } # End of Section Case

  }
  unless (close $input)
  { 
    print "ProcessConfigFile: Error Closing Config File $filename: $!\n";
  }

  #
  # Check to see if at least one parse error occurred.
  #
  if ($parseError == TRUE)
  {
    return ERROR;
  }
}



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
##############################################################################
sub RescanBE
{
  my ($ctlr) = @_;
  my %rsp;

  print "RescanBE: Scanning the disks....\n";

  %rsp = $ctlr->rescanDevice("LIST");
  
  if ( ! %rsp  )
  {
    print "RescanBE: >>>>>>>> No response rescanning the BE <<<<<<<<\n";
    return ERROR;
  }

  if ( $rsp{STATUS} != PI_GOOD )
  {
    print "RescanBE: >>>>>>>> Failed to rescan the drives <<<<<<<<\n";
    PrintError(%rsp);
    return ERROR;
  }

  return GOOD;
}



sub ForceLog
{
  my ($msg, $type_mess, $currentMgr) = @_;

  my %rsp = $currentMgr->logTextMessage($msg, $type_mess);

  if (%rsp)
  {
    if ($rsp{STATUS} != PI_GOOD)
    {
      print "ForceLog: Unable to send log message.\n";
    }
  }
  else
  {
    print "ForceLog: Error, did not receive a response packet.\n";
    logout();
  }

  return GOOD;       
}



#
# Check the BE device paths.  
#

sub CheckBeDevice
{
  my ($dscs) = @_;
  my @controller_ips = split(/\s+/, $dscs);
  my $ccbe_output;

  use strict;
    
  # need a longer timeout for this
  my $value = 1200;
  my %rsp = $dscs->timeoutMRP("MRP", $value);
  if (!%rsp)                        # no response
  {
    print "CheckBeDevice: >>>>>>>> Failed to receive a response from timeoutMRP <<<<<<<<\n";
    return ERROR;
  }

  if ($rsp{STATUS} == 1)            # 1 is bad
  {
    print "CheckBeDevice: >>>>>>>> Failed: timeoutMRP returned an error <<<<<<<<\n";
    PrintError(%rsp);
    return ERROR;
  }

  $ccbe_output = $dscs->PATH_PHYSICAL_DISK("PDISK", "BP");
    
  # Check for N=1 or N=2
    
  # Paths are 3 or C for dual connection on N=2
  my @ccbe_output = split(/\n/, $ccbe_output);
  my $bad_pids;
  my $count = 0;
  foreach my $i (@ccbe_output) 
  {
    if ($i =~ /^ *(0[0-9A-F]*) *000([0-9A-F])/) 
    {
      my $pid = $1;
      my $ports = $2;
      if ($ports ne '3' && $ports ne 'C') 
      {
        $count++;
        if (!defined($bad_pids)) 
        {
          $bad_pids = "$pid";
        } 
        else 
        {
          $bad_pids .= ",$pid";
        }
      }
    }
  }

  if ($count == 0) 
  {
    return GOOD;
  }    
    
  return ERROR;
}



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
  print "FindMasterIndex: $msg\n";

  print "FindMasterIndex: $nodeArray->{HOST}\n";
  my $modVCGIP = "$vcgInfoRsp{VCG_IP_ADDRESS}:3000";
  print "FindMasterIndex: $modVCGIP\n";
  
  $dscId = "$vcgInfoRsp{VCG_ID}";
  print "FindMasterIndex: $dscId\n";

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
  if(($masterIndex < 0) || ($masterIndex > scalar($nodeArray)))
  {
    print "FindMasterIndex: This is the slave\n";
    $rc = ERROR;
  }

  return($rc, $masterIndex, $dscId);
}

