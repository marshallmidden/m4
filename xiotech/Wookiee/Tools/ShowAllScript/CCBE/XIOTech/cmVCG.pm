# $Id: cmVCG.pm 159043 2012-03-08 21:34:48Z m4 $
##############################################################################
# Copyright (c) 2001-2008  Xiotech Corporation
# ======================================================================
# Author: Anthony Asleson
#
# Purpose:
#   Wrapper for all the different XIOTech commands that can be sent
#   to the Xiotech SAN system
##############################################################################
package XIOTech::cmdMgr;

use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;

use XIOTech::bigNums;
use XIOTech::error;

use XIOTech::logMgr;

use strict;

#ifdef ENGINEERING
##############################################################################
# Name:     vcgAddController
#
# Desc:     Add a controller to our group.
#
# Input:    obj - Command manager object that we can use to retrieve
#                   information.
#
#
# Returns:  Hash with the following values:
#               STATUS - Status code for this validation, PI_GOOD or one
#                        of the PI error codes.
#               ERROR_CODE - Error code for this validation.
#               ERROR_MSG - Optional error message.
##############################################################################
sub vcgAddController
{
    my ($self, $obj) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['i'],
                ["vcgAddController"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    bless($obj, "XIOTech::cmdMgr");

    my %rsp;
    my %vcginfo;
    my %slaveSerialNumbers;
    my $slaveNodeID;
    my $vcgid = 0;
    my $masterSN = 0;
    my $masterIP = unpack "L", $self->ip2long($self->{HOST});
    my $slaveIP = unpack "L", $obj->ip2long($obj->{HOST});
    my $slaveSN = 0;
    my $ctrlValid = 0;
    my %pdisks;
    my $ownedDrives = 0;

    $rsp{STATUS} = PI_GOOD;
    $rsp{ERROR_CODE} = 0;

    # Get the VCG info for the group and save the pieces we will need later.
    #   - VCGID
    #   - MASTER SN
    #   - Validate that the controller being added is part of the license
    if (%rsp && $rsp{STATUS} == PI_GOOD)
    {
        %vcginfo = $self->vcgInfo(0);

        if (!%vcginfo || $vcginfo{STATUS} != PI_GOOD)
        {
            if (%rsp)
            {
                $rsp{STATUS} = $vcginfo{STATUS};
                $rsp{ERROR_CODE} = $vcginfo{ERROR_CODE};
            }
            else
            {
                $rsp{STATUS} = PI_ERROR;
            }

            $rsp{ERROR_MSG} = "Unable to retrieve VCG information for the group.";
        }
        else
        {
            if ($vcginfo{VCG_CURRENT_NUM_CONTROLLERS} >= $vcginfo{VCG_MAX_NUM_CONTROLLERS})
            {
                $rsp{STATUS} = PI_ERROR;
                $rsp{ERROR_MSG} = "VCG already contains max controllers.";
            }
            else
            {
                $vcgid = $vcginfo{VCG_ID};

                for (my $i = 0; $i < $vcginfo{VCG_MAX_NUM_CONTROLLERS}; $i++)
                {
                    if ($masterSN == 0 && $vcginfo{CONTROLLERS}[$i]{AM_I_MASTER} == 1)
                    {
                        $masterSN = $vcginfo{CONTROLLERS}[$i]{SERIAL_NUMBER};
                    }
                }
            }
        }
    }

    # Make sure the controller being added is configured to be part
    # of this group and does not have a duplicate CN ID value.
    if (%rsp && $rsp{STATUS} == PI_GOOD)
    {
        print "Checking if the controller is valid to be added...\n";

        %slaveSerialNumbers = $obj->serialNumGet();

        if (%slaveSerialNumbers && $slaveSerialNumbers{STATUS} == PI_GOOD)
        {
            $slaveSN = $slaveSerialNumbers{1}{SERIAL_NUM};
            $slaveNodeID = ($slaveSN & 0xF);

            # Make sure the VCGID is the same as that of the group
            # the controller it is being added to.
            if ($slaveSerialNumbers{2}{SERIAL_NUM} != $vcgid)
            {
                $rsp{STATUS} = PI_ERROR;
                $rsp{ERROR_CODE} = 0;
                $rsp{ERROR_MSG} = "Controller is not configured to be part of the group.";
            }
            elsif ($slaveNodeID >= $vcginfo{VCG_MAX_NUM_CONTROLLERS})
            {
                $rsp{STATUS} = PI_ERROR;
                $rsp{ERROR_CODE} = 0;
                $rsp{ERROR_MSG} = "Controller Node ID is out of range.";
            }
            else
            {
                # Make sure the controller serial number does not already
                # exist in the configured controllers.
                for (my $i = 0; $i < $vcginfo{VCG_MAX_NUM_CONTROLLERS}; $i++)
                {
                    if ($slaveSerialNumbers{1}{SERIAL_NUM} == $vcginfo{CONTROLLERS}[$i]{SERIAL_NUMBER})
                    {
                        $rsp{STATUS} = PI_ERROR;
                        $rsp{ERROR_CODE} = 0;
                        $rsp{ERROR_MSG} = "Controller Node ID is already used in this group.";
                    }
                }
            }
        }
        else
        {
            if (%slaveSerialNumbers)
            {
                $rsp{STATUS} = $slaveSerialNumbers{STATUS};
                $rsp{ERROR_CODE} = $slaveSerialNumbers{ERROR_CODE};
            }
            else
            {
                $rsp{STATUS} = PI_ERROR;
                $rsp{ERROR_CODE} = 0;
            }

            $rsp{ERROR_MSG} = "Unable to retrieve serial numbers.";
        }
    }

    # Check if the virtual controller group currently owns drives (has
    # labeled drives).
    if (%rsp && $rsp{STATUS} == PI_GOOD)
    {
        print "Checking if the VCG owns drives...\n";

        %pdisks = $self->physicalDisks();

        if (%pdisks)
        {
            if ($pdisks{STATUS} != PI_GOOD)
            {
                $rsp{STATUS} = $pdisks{STATUS};
                $rsp{ERROR_CODE} = $pdisks{ERROR_CODE};
                $rsp{ERROR_MSG} = "Failed to retreive physical disks for VCG.";
            }
            else
            {
                for (my $i = 0; $i < $pdisks{COUNT}; $i++)
                {
                    if ($pdisks{PDISKS}[$i]{PD_SSERIAL} == $vcgid)
                    {
                        $ownedDrives++;
                        last;
                    }
                }
                
                if ($ownedDrives == 0)
                {
                    $rsp{STATUS} = PI_ERROR;
                    $rsp{ERROR_CODE} = 0;
                    $rsp{ERROR_MSG} = "VCG does not own any drives, please " .
                                        "label drives and retry the operation.";
                }
            }
        }
        else
        {
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
            $rsp{ERROR_MSG} = "Failed to retreive physical disks for VCG.";
        }
    }


    # Validate the VCG and controller to make sure the controller can be
    # added to the group.
    if (%rsp && $rsp{STATUS} == PI_GOOD)
    {
        print "Validating VCG and slave controller...\n";

        %rsp = $self->vcgValidateController($obj);

        if (!%rsp || $rsp{STATUS} != PI_GOOD)
        {
            print "First pass of controller validation failed..." .
                    "resetting controller.\n";
#            print "\n";

            $obj->resetProcessor(3, 2);

#            print "\n";
            print "Validating VCG and slave controller...\n";

            %rsp = $self->vcgValidateController($obj);
        }
    }

    # Prepare the slave controller
    if (%rsp && $rsp{STATUS} == PI_GOOD)
    {
        print "Preparing slave controller...\n";

        my %prep = $obj->_vcgPrepSlave($vcgid, $masterSN, $masterIP);

        if (!%prep || $prep{STATUS} != PI_GOOD)
        {
            if (%prep)
            {
                $rsp{STATUS} = $prep{STATUS};
                $rsp{ERROR_CODE} = $prep{ERROR_CODE};
            }
            else
            {
                $rsp{STATUS} = PI_ERROR;
                $rsp{ERROR_CODE} = 0;
            }

            $rsp{ERROR_MSG} = "Unable to prepare slave controller.";
        }
    }

    # Add the slave controller
    if (%rsp && $rsp{STATUS} == PI_GOOD)
    {
        print "Adding slave controller...\n";

        my %add = $self->_vcgAddSlave($slaveSN, $slaveIP);

        if (!%add || $add{STATUS} != PI_GOOD)
        {
            if (%add)
            {
                $rsp{STATUS} = $add{STATUS};
                $rsp{ERROR_CODE} = $add{ERROR_CODE};
            }
            else
            {
                $rsp{STATUS} = PI_ERROR;
                $rsp{ERROR_CODE} = 0;
            }

            $rsp{ERROR_MSG} = "Unable to add slave controller.";
        }
    }

    return %rsp;
}

##############################################################################
# Name:     vcgApplyLicense
#
# Desc:     Apply license information to the virtual controller group.
#
# Input:    virtual controller group ID
#           list of controllers licensed for this group
#
# Returns:
##############################################################################
sub vcgApplyLicense
{
    my ($self, $vcgid, $maxctrls) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ['d', 1, 8],
                ["vcgApplyLicense"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VCG_APPLY_LICENSE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("L", $vcgid);
    $data .= pack("C128", 0);
    $data .= pack("CCS", 0, 0, $maxctrls);

    for (my $i = 0; $i < $maxctrls; $i++)
    {
        $data .= pack("L", 0);
    }

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}
#endif
##############################################################################
# Name:     vcgInfo
#
# Desc:     Retrieve virtual controller group information.
#
# Input:
#
# Returns:
##############################################################################
sub vcgInfo
{
    my ($self, $vcgid) = @_;
#    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ["vcgInfo"]];
#    my $args = [['i'],
#                ["vcgInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VCG_INFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_vcgInfoPacket);
}

##############################################################################
# Name:     vcgPing
#
# Desc:     Ping another controller within the control group.
#
# Input:    serial number of controller to ping
#
# Returns:
##############################################################################
sub vcgPing
{
    my ($self, $serialNumber) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 1, 0xFFFFFFFF],
                ["vcgPing"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VCG_PING_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("L", $serialNumber);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}

##############################################################################
# Name:     vcgMPList
#
# Desc:     Retrieve the virtual controller group mirror partner list.
#
# Input:    NONE
#
# Returns:
##############################################################################
sub vcgMPList
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["vcgMPList"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VCG_GET_MP_LIST_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS", 0, 0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_vcgMPListResponsePacket);
}

##############################################################################
# Name:     vcgShutdown
#
# Desc:     Shuts down the VCG, what else did you expect?
#
# Input:    NONE
#
# Returns:
##############################################################################
sub vcgShutdown
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["vcgShutdown"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VCG_SHUTDOWN_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}

##############################################################################
# Name:     vcgActivateController
#
# Desc:     Activate a controller that is part of this group but is currently
#           inactivated.
#
# Input:    controller serial number
#
# Returns:
##############################################################################
sub vcgActivateController
{
    my ($self, $serialNumber) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 1, 0xFFFFFFFF],
                ["vcgActivateController"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VCG_ACTIVATE_CONTROLLER_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("L", $serialNumber);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}

##############################################################################
# Name:     vcgInactivateController
#
# Desc:     Inactivate a controller that is part of this group
#
# Input:    controller serial number
#
# Returns:
##############################################################################
sub vcgInactivateController
{
    my ($self, $serialNumber) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 1, 0xFFFFFFFF],
                ["vcgInactivateController"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VCG_INACTIVATE_CONTROLLER_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("L", $serialNumber);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}

##############################################################################
# Name:     vcgUnfailController
#
# Desc:     Unfail a controller that is part of this group but is currently
#           failed.
#
# Input:    controller serial number
#
# Returns:
##############################################################################
sub vcgUnfailController
{
    my ($self, $serialNumber) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 1, 0xFFFFFFFF],
                ["vcgUnfailController"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VCG_UNFAIL_CONTROLLER_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("L", $serialNumber);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}

##############################################################################
# Name:     vcgFailController
#
# Desc:     Fail a controller that is part of this group
#
# Input:    controller serial number
#
# Returns:
##############################################################################
sub vcgFailController
{
    my ($self, $serialNumber) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 1, 0xFFFFFFFF],
                ["vcgFailController"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VCG_FAIL_CONTROLLER_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("L", $serialNumber);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}

#ifdef ENGINEERING
##############################################################################
# Name:     vcgRemoveController
#
# Desc:     Remove a controller from this group.
#
# Input:    controller serial number
#
# Returns:
##############################################################################
sub vcgRemoveController
{
    my ($self, $obj) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['i'],
                ["vcgRemoveController"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    bless($obj, "XIOTech::cmdMgr");

    my %rsp;
    $rsp{STATUS} = PI_GOOD;
    $rsp{ERROR_CODE} = 0;

    my $sn = 0;

    if (%rsp && $rsp{STATUS} == PI_GOOD)
    {
        my %ctrl = $obj->serialNumGet();

        if (%ctrl && $ctrl{STATUS} == PI_GOOD)
        {
            $sn = $ctrl{1}{SERIAL_NUM};
        }
        else
        {
            if (%ctrl)
            {
                $rsp{STATUS} = $ctrl{STATUS};
                $rsp{ERROR_CODE} = $ctrl{ERROR_CODE};
            }
            else
            {
                $rsp{STATUS} = PI_ERROR;
                $rsp{ERROR_CODE} = 0;
            }

            $rsp{ERROR_MSG} = "Unable to retrieve controller serial numbers.";
        }
    }

    if (%rsp && $rsp{STATUS} == PI_GOOD)
    {
        %rsp = $self->_removeController($sn);
    }

    if (%rsp && $rsp{STATUS} == PI_GOOD)
    {
        %rsp = $obj->_removeController($sn);
    }

    return %rsp;
}

##############################################################################
# Name:     vcgValidation
#
# Desc:     Starts group redundancy validation on this group.
#
# Input:    $flags      Refer to CCB code
#
# Returns:  Hash with the following values:
#               STATUS - Status code for this validation, PI_GOOD or one
#                        of the PI error codes.
#               ERROR_CODE - Error code for this validation.
#               ERROR_MSG - Optional error message.
##############################################################################
sub vcgValidation
{
    my ($self, $flags) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["vcgValidation"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    # Send request using PI protocol
    my $cmd = PI_VCG_VALIDATION_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("Sa2", $flags, 0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                      $packet,
                                      \&_genericResponsePacket);
}


##############################################################################
# Name:     vcgValidateController
#
# Desc:     Validate a controller can be added to our group.
#
# Input:    obj - Command manager object that we can use to retrieve
#                   information.
#
# Returns:  Hash with the following values:
#               STATUS - Status code for this validation, PI_GOOD or one
#                        of the PI error codes.
#               ERROR_CODE - Error code for this validation.
#               ERROR_MSG - Optional error message.
##############################################################################
sub vcgValidateController
{
    my ($self, $obj) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['i'],
                ["vcgValidateController"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    bless($obj, "XIOTech::cmdMgr");

    my %rsp;
    $rsp{STATUS} = PI_GOOD;
    $rsp{ERROR_CODE} = 0;

    # Verify that the VCG has been established (serial numbers are different).
    if (%rsp && $rsp{STATUS} == PI_GOOD)
    {
        print "Verifying serial numbers are different (VCG is established)...\n";
        logMsg("Verifying serial numbers are different (VCG is established)...\n");
        %rsp = $self->_validateVCG();
    }

    # Verify that the controller is in the correct state to be added to a group.
    if (%rsp && $rsp{STATUS} == PI_GOOD)
    {
        print "Verifying controller state...\n";
        logMsg("Verifying controller state...\n");
        %rsp = $obj->_validateController();
    }

    # Verify that the firmware versions on the VCG and controller are the same.
    if (%rsp && $rsp{STATUS} == PI_GOOD)
    {
        print "Verifying firmware versions are the same...\n";
        logMsg("Verifying firmware versions are the same...\n");
        %rsp = $self->_validateFWV($obj);
    }

    if (%rsp && $rsp{STATUS} == PI_GOOD)
    {
        print "Validating physical disks...\n";
        logMsg("Validating physical disks...\n");
        %rsp = $self->_validatePhysicalDisks($obj);
    }

###    if (%rsp && $rsp{STATUS} == PI_GOOD)
###    {
###        print "Validating FE deivce list...\n";
###        logMsg("Validating FE deivce list...\n");
###        %rsp = $self->_validateDeviceListFE($obj);
###    }

    return %rsp;
}
#endif

##############################################################################
# Name:     vcgElectionState
#
# Desc:     retrieves the election state from the controller
#
# Input:    none
#
# Return:   _modeDataResponsePacket hash
##############################################################################
sub vcgElectionState
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["electionState"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }
    
    my $cmd = PI_DEBUG_GET_ELECTION_STATE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_electionStateResponsePacket);
}

##############################################################################
# Name: displayVCGInfo
#
# Desc: Create a string from the virtual controller group information.
#
# In:   Virtual Controller Group Information Hash
##############################################################################
sub displayVCGInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $i;
    my @controllers = $info{CONTROLLERS};

    my $msg = "";

    $msg .= "Virtual Controller Group Information:\n";
    $msg .= sprintf "  VCG_ID:                %lu (0x%8.8x)\n", $info{VCG_ID}, $info{VCG_ID};
    $msg .= sprintf "  VCG_IP_ADDRESS:        %s\n", $info{VCG_IP_ADDRESS};
    $msg .= sprintf "  VCG_MAX_NUM_CTRLS:     %hu\n", $info{VCG_MAX_NUM_CONTROLLERS};
    $msg .= sprintf "  VCG_CURRENT_NUM_CTRLS: %hu\n", $info{VCG_CURRENT_NUM_CONTROLLERS};
    $msg .= sprintf "\n";

    $msg .= sprintf "  SERIAL# (HEX)            IP_ADDRESS        FAILURE_STATE              RANK\n";
    $msg .= sprintf "  -----------------------  ----------------  -------------------------  ------\n";

    for ($i = 0; $i < $info{VCG_MAX_NUM_CONTROLLERS}; $i++)
    {
        my $failureState = "";

        if ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_UNUSED)
        {
            $failureState = "UNUSED";
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_FAILED)
        {
            $failureState = "FAILED";
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_OPERATIONAL)
        {
            $failureState = "OPERATIONAL";
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_POR)
        {
            $failureState = "POWER ON READY"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_ADD_CONTROLLER_TO_VCG)
        {
            $failureState = "ADDING CTRL TO VCG"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_STRANDED_CACHE_DATA)
        {
            $failureState = "STRANDED CACHE DATA"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_FIRMWARE_UPDATE_INACTIVE)
        {
            $failureState = "FIRMWARE UPDATE INACTIVE"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_FIRMWARE_UPDATE_ACTIVE)
        {
            $failureState = "FIRMWARE UPDATE ACTIVE"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_UNFAIL_CONTROLLER)
        {
            $failureState = "UNFAIL CONTROLLER"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_VCG_SHUTDOWN)
        {
            $failureState = "VCG SHUTDOWN"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_INACTIVATED)
        {
            $failureState = "INACTIVATED"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_ACTIVATE)
        {
            $failureState = "ACTIVATE"
        }
        elsif ($info{CONTROLLERS}[$i]{FAILURE_STATE} == FD_STATE_DISASTER_INACTIVE)
        {
            $failureState = "DISASTER INACTIVE"
        }
        else
        {
            $failureState = "UNKNOWN";
        }

        my $master = "MASTER";

        if ($info{CONTROLLERS}[$i]{AM_I_MASTER} == 0)
        {
            $master = "SLAVE";
        }

        $msg .= sprintf "  %-10lu (0x%8.8x)  %-16s  %-25s  %-11s\n",
                        $info{CONTROLLERS}[$i]{SERIAL_NUMBER},
                        $info{CONTROLLERS}[$i]{SERIAL_NUMBER},
                        $info{CONTROLLERS}[$i]{IP_ADDRESS},
                        $failureState,
                        $master;
    }

    $msg .= "\n";

    return $msg;
}

##############################################################################
# Name: displayVCGMPList
#
# Desc: Print the virtual controller group mirror partner list.
#
# In:   Virtual Controller Group Mirror Partner List Hash
##############################################################################
sub displayVCGMPList
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $i;
    my @mplist = $info{MPLIST};

    my $msg = "";

    $msg .= "Virtual Controller Group Mirror Partner List:\n";
    $msg .=  "\n";

    $msg .=  "    SOURCE SERIAL# (HEX)     DEST SERIAL# (HEX)   \n";
    $msg .=  "  -----------------------  -----------------------\n";

    for ($i = 0; $i < $info{COUNT}; $i++)
    {
        $msg .= sprintf "  %-10lu (0x%8.8x)  %-10lu (0x%8.8x)\n",
                $info{MPLIST}[$i]{SOURCE},
                $info{MPLIST}[$i]{SOURCE},
                $info{MPLIST}[$i]{DEST},
                $info{MPLIST}[$i]{DEST};
    }

    $msg .= "\n";

    return $msg;
}

##############################################################################
# Name: displayCpuCount
#
# Desc: Print the Cpu count information.
#
# In:   
##############################################################################
sub displayCpuCount
{
    my ($self, %info) = @_;

    logMsg("begin\n");
                my @configlist = $info{CONFIGLIST};

                my $msg = "";
                $msg .= sprintf " CPU COUNT:       %d\n",$info{CPUCOUNT};
                $msg .= "\n";
                return $msg;
}


##############################################################################
# Name: displayBEType
#
# Desc: Print the be type information.
#
# In:   
##############################################################################
sub displayBEType
{
    my ($self, %info) = @_;

    logMsg("begin\n");
                my @configlist = $info{CONFIGLIST};

                my $msg = "";

                if ($info{BETYPE} == 0)
                {
                                $msg .= " BACKEND TYPE IS LOOP";
                }
                else
                {
                                $msg .= " BACKEND TYPE IS FABRIC";
                }

                $msg .= "\n";
                return $msg;
}



#ifdef ENGINEERING
##############################################################################
# Name:     rollingUpdatePhase
#
# Desc:     Executes the rolling update phase for use before/after rolling
#           code updates.
#
# Input:    none
#
# Return:   _genericResponsePacket
##############################################################################
sub rollingUpdatePhase
{
    my ($self, $sn, $phase) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 1, 0xFFFFFFFF],
                ['d', 1, 2],
                ["rollingUpdatePhase"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }
    
    my $cmd = PI_ROLLING_UPDATE_PHASE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LL", $sn, $phase);
    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}
#endif

##############################################################################
#
#   All commands that start with a _ should not be used and are considered
#   private.
#
##############################################################################

##############################################################################
# Name:     _vcgAddSlave
#
# Desc:     Add the slave into virtual controller group.
#
# Input:    IP Address of the controller being added
#
# Returns:
##############################################################################
sub _vcgAddSlave
{
    my ($self, $controllerSN, $ipAddress) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 1, 0xFFFFFFFF],
                ['d', 1, 0xFFFFFFFF],
                ["vcgAddSlave"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VCG_ADD_SLAVE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LL", $controllerSN, $ipAddress);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}

##############################################################################
# Name:     _vcgPrepSlave
#
# Desc:     Prepare slave for addition into virtual controller group.
#
# Input:    virtual controller group ID
#
# Returns:
##############################################################################
sub _vcgPrepSlave
{
    my ($self, $vcgid, $controllerSN, $ipAddress) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 1, 0xFFFFFFFF],
                ['d', 1, 0xFFFFFFFF],
                ['d', 1, 0xFFFFFFFF],
                ["vcgPrepSlave"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VCG_PREPARE_SLAVE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LLL",
                    $vcgid,
                    $controllerSN,
                    $ipAddress);

    $data .= pack("CCCCCCCCCCCCCCCC", 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}

##############################################################################
# Name:     _removeController
#
# Desc:     Removes a controller from the group.
#
# Input:    $self - Our command manager object.
#           $serialNumber - Serial number of the controller to remove.
#
# Returns:  Hash with the following values:
#               STATUS - Status code for this validation, PI_GOOD or one
#                        of the PI error codes.
#               ERROR_CODE - Error code for this validation.
#               ERROR_MSG - Optional error message.
##############################################################################
sub _removeController
{
    my ($self, $serialNumber) = @_;

    my %rsp;
    my %rmv;

    logMsg("begin\n");

    $rsp{STATUS} = PI_GOOD;
    $rsp{ERROR_CODE} = 0;

    my $cmd = PI_VCG_REMOVE_CONTROLLER_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("L", $serialNumber);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    %rmv = $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);

    if (!%rmv || $rmv{STATUS} != PI_GOOD)
    {
        if (%rmv)
        {
            $rsp{STATUS} = $rmv{STATUS};
            $rsp{ERROR_CODE} = $rmv{ERROR_CODE};
        }
        else
        {
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
        }

        $rsp{ERROR_MSG} = "Unable to remove controller.";
    }

    logMsg("end\n");

    return %rsp;
}

##############################################################################
# Name:     configController
#
# Desc:     Configures controller.
#
# Input:    
#           IP Address
#           Subnet Mask
#           Default Gateway
#           Replacement Status
#
# Returns:  Hash with the following values:
#               STATUS - Status code for this validation, PI_GOOD or one
#                        of the PI error codes.
#               ERROR_CODE - Error code for this validation.
#               ERROR_MSG - Optional error message.
##############################################################################
sub configController
{
    my ($self, $ipAddr, $subnet, $gateway, $dscId, $nodeId, $replacement) = @_;

    my %rsp;
    my %rmv;

    logMsg("begin\n");

    $rsp{STATUS} = PI_GOOD;
    $rsp{ERROR_CODE} = 0;

    my $cmd = PI_VCG_CONFIGURE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LLLLLCCS", $ipAddr, $subnet, $gateway, $dscId, $nodeId, $replacement, 0, 0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT});

    %rmv = $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);

    if (!%rmv || $rmv{STATUS} != PI_GOOD)
    {
        if (%rmv)
        {
            $rsp{STATUS} = $rmv{STATUS};
            $rsp{ERROR_CODE} = $rmv{ERROR_CODE};
        }
        else
        {
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
        }

        $rsp{ERROR_MSG} = "Unable to configure controller.";
    }

    logMsg("end\n");

    return %rsp;
}

##############################################################################
# Name:     _getCpuCountPacket
#
# Desc:     Extract the information from the receive packet.
#
# Input:    
#
# Returns:  
#           
##############################################################################

sub _getCpuCountPacket
{

    my ($self,
        $seq,  
        $recvPacket) = @_;

    logMsg("begin\n");
    my %info;
                my $rsvd;

        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

    # Load into hash
    ( $info{CPUCOUNT},
                        $rsvd) = unpack ("Ca19", $parts{DATA});
      
    return %info;
}

##############################################################################
# Name:     GetCPUCOUNT
#
# Desc:     Get the cput count information.
#
# Input:    
#
# Returns:  
#           
##############################################################################
sub GetCPUCount
{

    my ($self) = @_;

    logMsg("begin\n");

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $packet = assembleXiotechPacket(PI_GET_CPUCOUNT_CMD,
                                        $seq,
                                        $ts,
                                                                                                                                                             undef,
                                        $self->{PORT}, VERSION_1);


    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_getCpuCountPacket);

}

##############################################################################
# Name:     _getBackendTypePacket
#
# Desc:     Extract the information from the receive packet.
#
# Input:    
#
# Returns:  
#           
##############################################################################

sub _getBackendTypePacket
{

    my ($self,
        $seq,  
        $recvPacket) = @_;

    logMsg("begin\n");
    my %info;
                my $rsvd;

        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

    # Load into hash
    ( $info{BETYPE},
                        $rsvd) = unpack ("Ca3", $parts{DATA});
      
    return %info;
}



##############################################################################
# Name:     GetBEType
#
# Desc:     Get the backend type whether fabric or loop.
#
# Input:    
#
# Returns:  
#           
##############################################################################
sub GetBEType
{

    my ($self) = @_;

    logMsg("begin\n");

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $packet = assembleXiotechPacket(PI_GET_BACKEND_TYPE_CMD,
                                        $seq,
                                        $ts,
                                                                                                                                                             undef,
                                        $self->{PORT}, VERSION_1);


    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_getBackendTypePacket);

}



##############################################################################
# Name:     _vcgInfoPacket
#
# Desc:     Parses the virtual controller group information packet and
#           places the information in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
##############################################################################
sub _vcgInfoPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;

    if (commandCode($recvPacket) == PI_VCG_INFO_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        my $rsvd;

        # Unpack the data
        (
        $info{VCG_ID},
        $info{VCG_IP_ADDRESS},
        $info{VCG_MAX_NUM_CONTROLLERS},
        $info{VCG_CURRENT_NUM_CONTROLLERS}
        ) = unpack("LNSS", $parts{DATA});

        $info{VCG_IP_ADDRESS} = long2ip(0, $info{VCG_IP_ADDRESS});
        
        my @controllers;
        for ($i = 0; $i < $info{VCG_MAX_NUM_CONTROLLERS}; $i++)
        {
            my $start = 12 + (16 * $i);
            (
            $controllers[$i]{SERIAL_NUMBER},
            $controllers[$i]{IP_ADDRESS},
            $controllers[$i]{FAILURE_STATE},
            $rsvd,
            $controllers[$i]{AM_I_MASTER}
            ) = unpack("LNLa3C", substr($parts{DATA}, $start));

            $controllers[$i]{IP_ADDRESS} = long2ip(0, $controllers[$i]{IP_ADDRESS});
        }

        $info{CONTROLLERS} = [@controllers];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual controller group info packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _vcgMPListResponsePacket
#
# Desc:     Parses the virtual controller group get mirror partner list
#           packet and places the information in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
##############################################################################
sub _vcgMPListResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;

    if (commandCode($recvPacket) == PI_VCG_GET_MP_LIST_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        my $rsvd;

        # Unpack the data
        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{COUNT},
        $rsvd
        ) = unpack("a3CLSa2", $parts{DATA});

        my @mplist;

        for ($i = 0; $i < $info{COUNT}; $i++)
        {
            my $start = 12 + (8 * $i);
            (
            $mplist[$i]{SOURCE},
            $mplist[$i]{DEST}
            ) = unpack("LL", substr($parts{DATA}, $start));
        }

        $info{MPLIST} = [@mplist];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual controller group info packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _validateVCG
#
# Desc:     Validates a VCG has been established.
#
# Input:    $self - Our command manager object.
#
# Returns:  Hash with the following values:
#               STATUS - Status code for this validation, PI_GOOD or one
#                        of the PI error codes.
#               ERROR_CODE - Error code for this validation.
#               ERROR_MSG - Optional error message.
##############################################################################
sub _validateVCG
{
    my ($self) = @_;
    my %rsp;
    my %vcg;

    logMsg("begin\n");

    $rsp{STATUS} = PI_GOOD;
    $rsp{ERROR_CODE} = 0;

    %vcg = $self->serialNumGet();

    if (%vcg && $vcg{STATUS} == PI_GOOD)
    {
        if ($vcg{2}{SERIAL_NUM} == 0)
        {
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
            $rsp{ERROR_MSG} = "VCG is not established, please run VCGApplyLicense.";
        }
    }
    else
    {
        if (%vcg)
        {
            $rsp{STATUS} = $vcg{STATUS};
            $rsp{ERROR_CODE} = $vcg{ERROR_CODE};
        }
        else
        {
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
        }

        $rsp{ERROR_MSG} = "Unable to retrieve serial numbers.";
    }

    logMsg("end\n");

    return %rsp;
}

##############################################################################
# Name:     _validateController
#
# Desc:     Validates a controller is single and has not configuration.
#
# Input:    $self - Our command manager object.
#
# Returns:  Hash with the following values:
#               STATUS - Status code for this validation, PI_GOOD or one
#                        of the PI error codes.
#               ERROR_CODE - Error code for this validation.
#               ERROR_MSG - Optional error message.
##############################################################################
sub _validateController
{
    my ($self) = @_;
    my %rsp;
    my %powerupResponse;

    logMsg("begin\n");

    $rsp{STATUS} = PI_GOOD;
    $rsp{ERROR_CODE} = 0;

    while ($rsp{STATUS} == PI_GOOD)
    {
        %powerupResponse = $self->powerUpState();

        if (%powerupResponse && $powerupResponse{STATUS} == PI_GOOD)
        {
            #
            # If the controller is still in the power-up start state it has
            # not yet reached one of the final power-up states so just wait.
            #
            if ($powerupResponse{STATE} == POWER_UP_START)
            {
                sleep(5);
                next;
            }

            #
            # If the state is waiting for a license the controller is
            # valid to be added to the group so complete the controller
            # validation.
            #
            # Otherwise, the controller is in an invalid state and it
            # cannot be added to a group.
            #
            if ($powerupResponse{STATE} == POWER_UP_WAIT_LICENSE)
            {
                last;
            }
            else
            {
                $rsp{STATUS} = PI_ERROR;
                $rsp{ERROR_CODE} = 0;
                $rsp{ERROR_MSG} = "Controller is not waiting for a license, " .
                                    "unable to add controller.";
            }
        }
        else
        {
            if (%powerupResponse)
            {
                $rsp{STATUS} = $powerupResponse{STATUS};
                $rsp{ERROR_CODE} = $powerupResponse{ERROR_CODE};
            }
            else
            {
                $rsp{STATUS} = PI_ERROR;
                $rsp{ERROR_CODE} = 0;
            }

            $rsp{ERROR_MSG} = "Unable to retrieve powerup state.";
        }
    }

    logMsg("end\n");

    return %rsp;
}

##############################################################################
# Name:     _validateFWV
#
# Desc:     Validates firmware versions from the controller match the VCGs.
#
# Input:    $self - Our command manager object.
#           $obj - The command manager object for the controller to validate.
#
# Returns:  Hash with the following values:
#               STATUS - Status code for this validation, PI_GOOD or one
#                        of the PI error codes.
#               ERROR_CODE - Error code for this validation.
#               ERROR_MSG - Optional error message.
##############################################################################
sub _validateFWV
{
    my ($self, $obj) = @_;
    my %rsp;
    my $i;
    my $j;
    my %fwv1;
    my %fwv2;
    my @fwtypes;
    
    logMsg("begin\n");
    
    if ($self->{CONTROLLER_TYPE} == CTRL_TYPE_BIGFOOT)
    {
        @fwtypes = ( FW_VER_CCB_RUNTIME,
                     FW_VER_CCB_BOOT,
                     FW_VER_BE_RUNTIME,
                     FW_VER_BE_BOOT,
                     FW_VER_BE_DIAG,
                     FW_VER_FE_RUNTIME,
                     FW_VER_FE_BOOT,
                     FW_VER_FE_DIAG);
    }
    else
    {
        @fwtypes = ( FW_VER_CCB_RUNTIME,
                     FW_VER_BE_RUNTIME,
                     FW_VER_FE_RUNTIME);
    }

    $rsp{STATUS} = PI_GOOD;
    $rsp{ERROR_CODE} = 0;

    # Get the firmware version for the VCG
    if ($rsp{STATUS} == PI_GOOD)
    {
        %fwv1 = $self->fwVersion();

        if (%fwv1)
        {
            if ($fwv1{STATUS} != PI_GOOD)
            {
                $rsp{STATUS} = $fwv1{STATUS};
                $rsp{ERROR_CODE} = $fwv1{ERROR_CODE};
                $rsp{ERROR_MSG} = "Failed to retreive firmware versions for VCG.";
            }
        }
        else
        {
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
            $rsp{ERROR_MSG} = "Failed to retreive firmware versions for VCG.";
        }
    }

    # Get the firmware version for the controller
    if ($rsp{STATUS} == PI_GOOD)
    {
        %fwv2 = $obj->fwVersion();

        if (%fwv2)
        {
            if ($fwv2{STATUS} != PI_GOOD)
            {
                $rsp{STATUS} = $fwv2{STATUS};
                $rsp{ERROR_CODE} = $fwv2{ERROR_CODE};
                $rsp{ERROR_MSG} = "Failed to retreive firmware versions for controller.";
            }
        }
        else
        {
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
            $rsp{ERROR_MSG} = "Failed to retreive firmware versions for controller.";
        }
    }

    if ($rsp{STATUS} == PI_GOOD)
    {
        for (my $i = 0; $i < scalar(@fwtypes); ++$i)
        {
            my $type = $fwtypes[$i];

            if (!($fwv1{$type}{PRODUCT_ID} eq $fwv2{$type}{PRODUCT_ID}) ||
                !($fwv1{$type}{REVISION}   eq $fwv2{$type}{REVISION}) ||
                !($fwv1{$type}{REV_COUNT}  eq $fwv2{$type}{REV_COUNT}) ||
                !($fwv1{$type}{BUILD_ID}   eq $fwv2{$type}{BUILD_ID}) ||
                !($fwv1{$type}{SYSTEM_RLS} eq $fwv2{$type}{SYSTEM_RLS}) ||
                !($fwv1{$type}{TS_YEAR}    eq $fwv2{$type}{TS_YEAR}) ||
                !($fwv1{$type}{TS_MONTH}   eq $fwv2{$type}{TS_MONTH}) ||
                !($fwv1{$type}{TS_DATE}    eq $fwv2{$type}{TS_DATE}) ||
                !($fwv1{$type}{TS_DAY}     eq $fwv2{$type}{TS_DAY}) ||
                !($fwv1{$type}{TS_HOURS}   eq $fwv2{$type}{TS_HOURS}) ||
                !($fwv1{$type}{TS_MINUTES} eq $fwv2{$type}{TS_MINUTES}) ||
                !($fwv1{$type}{TS_SECONDS} eq $fwv2{$type}{TS_SECONDS}))
            {
                $rsp{STATUS} = PI_ERROR;
                $rsp{ERROR_CODE} = 0;
                $rsp{ERROR_MSG} = "VCG and controller do not have the same firmware versions.";
                last;
            }
        }
    }

    logMsg("end\n");

    return %rsp;
}

##############################################################################
# Name:     _validateDriveBays
#
# Desc:     Validates drive bays from the controller match the VCGs.
#
# Input:    $self - Our command manager object.
#           $obj - The command manager object for the controller to validate.
#
# Returns:  Hash with the following values:
#               STATUS - Status code for this validation, PI_GOOD or one
#                        of the PI error codes.
#               ERROR_CODE - Error code for this validation.
#               ERROR_MSG - Optional error message.
##############################################################################
sub _validateDriveBays
{
    my ($self, $obj) = @_;
    my %rsp;
    my $i;
    my $j;
    my %bays1;
    my %bays2;

    logMsg("begin\n");

    $rsp{STATUS} = PI_GOOD;
    $rsp{ERROR_CODE} = 0;

    # Get the disk bays for the VCG
    if ($rsp{STATUS} == PI_GOOD)
    {
        %bays1 = $self->diskBays();

        if (%bays1)
        {
            if ($bays1{STATUS} != PI_GOOD)
            {
                $rsp{STATUS} = $bays1{STATUS};
                $rsp{ERROR_CODE} = $bays1{ERROR_CODE};
                $rsp{ERROR_MSG} = "Failed to retreive disk bays for VCG.";
            }
        }
        else
        {
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
            $rsp{ERROR_MSG} = "Failed to retreive disk bays for VCG.";
        }
    }

    # Get the disk bays for the controller
    if ($rsp{STATUS} == PI_GOOD)
    {
        %bays2 = $obj->diskBays();

        if (%bays2)
        {
            if ($bays2{STATUS} != PI_GOOD)
            {
                $rsp{STATUS} = $bays2{STATUS};
                $rsp{ERROR_CODE} = $bays2{ERROR_CODE};
                $rsp{ERROR_MSG} = "Failed to retreive disk bays for controller.";
            }
        }
        else
        {
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
            $rsp{ERROR_MSG} = "Failed to retreive disk bays for controller.";
        }
    }

    # Check to make sure the drives that the VCG and controller are the same
    # If the WWNs match they are the same drive and we will check the
    # following values for equality:
    #   PD_DEVTYPE
    #   PD_DEVSTAT
    if ($rsp{STATUS} == PI_GOOD)
    {
        for (my $i = 0; $i < $bays1{COUNT}; $i++)
        {
            my $bay = $bays1{BAYS}[$i];
            my %bay1 = %$bay;

            # If the device does not have a "non-existant" device status
            # we should find out if the slave sees this device.  If it is
            # "non-existant" well, skip it!
            if ($bay1{PD_DEVSTAT} != 0)
            {
                # Assume that the validation is BAD until we find a match
                my $bOK = 0;

                for (my $j = 0; $j < $bays2{COUNT}; $j++)
                {
                    my $bay = $bays2{BAYS}[$j];
                    my %bay2 = %$bay;

                    # Is this the same device?  Check the WWNs and LUNs...
                    if ($bay1{WWN_LO} == $bay2{WWN_LO} &&
                        $bay1{WWN_HI} == $bay2{WWN_HI} &&
                        $bay1{PD_LUN} == $bay2{PD_LUN})
                    {
                        # We have found the device, until we find something
                        # wrong with it assume it is good.
                        $bOK = 1;

                        if ($bay1{PD_DEVTYPE} != $bay2{PD_DEVTYPE})
                        {
                            my $msg = "Device type does not match.  " .
                                        "VCG BID: $bay1{PD_BID}" .
                                        ", Controller BID: $bay2{PD_BID}" .
                                        ", VCG Class: $bay1{PD_DEVTYPE}" .
                                        ", Controller Class: $bay2{PD_DEVTYPE}";
                            $rsp{ERROR_MSG} = $msg;
                            $bOK = 0;
                            last;
                        }

                        if ($bay1{PD_DEVSTAT} != $bay2{PD_DEVSTAT})
                        {
                            my $msg = "Device status does not match.  " .
                                        "VCG BID: $bay1{PD_PID}" .
                                        ", Controller BID: $bay2{PD_PID}" .
                                        ", VCG Device Status: $bay1{PD_DEVSTAT}" .
                                        ", Controller Device Status: $bay2{PD_DEVSTAT}";
                            $rsp{ERROR_MSG} = $msg;
                            $bOK = 0;
                            last;
                        }
                    }
                }

                # If the device was either not found or was found to be
                # different than what the VCG thought it was we need to
                # stop the validation.
                if ($bOK == 0)
                {
                    $rsp{STATUS} = PI_ERROR;
                    $rsp{ERROR_CODE} = 0;

                    if (!defined($rsp{ERROR_MSG}) || $rsp{ERROR_MSG} eq "")
                    {
                        my $msg = "Device was not found on the controller.  " .
                                    "VCG BID: $bay1{PD_BID}";
                        $rsp{ERROR_MSG} = $msg;
                    }

                    last;
                }
            }
        }
    }

    logMsg("end\n");

    return %rsp;
}

##############################################################################
# Name:     _validatePhysicalDisks
#
# Desc:     Validates phsical disks from the controller match the VCGs.
#
# Input:    $self - Our command manager object.
#           $obj - The command manager object for the controller to validate.
#
# Returns:  Hash with the following values:
#               STATUS - Status code for this validation, PI_GOOD or one
#                        of the PI error codes.
#               ERROR_CODE - Error code for this validation.
#               ERROR_MSG - Optional error message.
##############################################################################
sub _validatePhysicalDisks
{
    my ($self, $obj) = @_;
    my %rsp;
    my $i;
    my $j;
    my %pdisks1;
    my %pdisks2;
    my $retryCount;

    logMsg("begin\n");

    $rsp{STATUS} = PI_GOOD;
    $rsp{ERROR_CODE} = 0;

    # Get the physical disks for the VCG
    if ($rsp{STATUS} == PI_GOOD)
    {
        %pdisks1 = $self->physicalDisks();

        if (%pdisks1)
        {
            if ($pdisks1{STATUS} != PI_GOOD)
            {
                $rsp{STATUS} = $pdisks1{STATUS};
                $rsp{ERROR_CODE} = $pdisks1{ERROR_CODE};
                $rsp{ERROR_MSG} = "Failed to retreive physical disks for VCG.";
            }
        }
        else
        {
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
            $rsp{ERROR_MSG} = "Failed to retreive physical disks for VCG.";
        }
    }

    # Set the retry count for retrieving the physical disks
    $retryCount = 600;

    # Get the physical disks for the controller
    while ($rsp{STATUS} == PI_GOOD && $retryCount > 0)
    {
        # Decrement the retry count
        $retryCount--;
        
        %pdisks2 = $obj->physicalDisks();

        if (%pdisks2)
        {
            if ($pdisks2{STATUS} != PI_GOOD)
            {
                $rsp{STATUS} = $pdisks2{STATUS};
                $rsp{ERROR_CODE} = $pdisks2{ERROR_CODE};
                $rsp{ERROR_MSG} = "Failed to retrieve physical disks for controller.";
            }
            else
            {
                if ($pdisks2{COUNT} > 0)
                {
                    # Expire the retry count since we found physical disks.
                    $retryCount = 0;
                }
                else
                {
                    print "...Empty physical disk list, delay and retry...\n";
                    sleep(1);
                }
            }
        }
        else
        {
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
            $rsp{ERROR_MSG} = "Failed to retreive physical disks for controller.";
        }
    }

    # Check to make sure the drives that the VCG and controller are the same
    # If the WWNs match they are the same drive and we will check the
    # following values for equality:
    #   PD_DEVTYPE
    #   PD_DEVSTAT
    #   PD_SSERIAL
    if ($rsp{STATUS} == PI_GOOD)
    {
        for (my $i = 0; $i < $pdisks1{COUNT}; $i++)
        {
            my $pd = $pdisks1{PDISKS}[$i];
            my %pdisk1 = %$pd;

            # If the device does not have a "non-existant" device status
            # we should find out if the slave sees this device.  If it is
            # "non-existant" well, skip it!
            if ($pdisk1{PD_DEVSTAT} != 0)
            {
                # Assume that the validation is BAD until we find a match
                my $bOK = 0;

                for (my $j = 0; $j < $pdisks2{COUNT}; $j++)
                {
                    my $pd = $pdisks2{PDISKS}[$j];
                    my %pdisk2 = %$pd;

                    # Is this the same device?  Check the WWNs and LUNs...
                    if ($pdisk1{WWN_LO} == $pdisk2{WWN_LO} &&
                        $pdisk1{WWN_HI} == $pdisk2{WWN_HI} &&
                        $pdisk1{PD_LUN} == $pdisk2{PD_LUN})
                    {
                        # We have found the device, until we find something
                        # wrong with it assume it is good.
                        $bOK = 1;

                        if ($pdisk1{PD_DEVTYPE} != $pdisk2{PD_DEVTYPE})
                        {
                            my $msg = "Device type does not match.  " .
                                        "VCG PID: $pdisk1{PD_PID}" .
                                        ", Controller PID: $pdisks2{PD_PID}" .
                                        ", VCG Class: $pdisk1{PD_DEVTYPE}" .
                                        ", Controller Class: $pdisks2{PD_DEVTYPE}";
                            $rsp{ERROR_MSG} = $msg;
                            $bOK = 0;
                            last;
                        }

                        if ($pdisk1{PD_DEVSTAT} != $pdisk2{PD_DEVSTAT})
                        {
                            my $msg = "Device status does not match.  " .
                                        "VCG PID: $pdisk1{PD_PID}" .
                                        ", Controller PID: $pdisk2{PD_PID}" .
                                        ", VCG Device Status: $pdisk1{PD_DEVSTAT}" .
                                        ", Controller Device Status: $pdisk2{PD_DEVSTAT}";
                            $rsp{ERROR_MSG} = $msg;
                            $bOK = 0;
                            last;
                        }

                        if ($pdisk1{PD_SSERIAL} != $pdisk2{PD_SSERIAL})
                        {
                            my $msg = "System serial number does not match.  " .
                                        "VCG PID: $pdisk1{PD_PID}" .
                                        ", Controller PID: $pdisk2{PD_PID}" .
                                        ", VCG System Serial Number: $pdisk1{PD_SSERIAL}" .
                                        ", Controller System Serial Number: $pdisk2{PD_SSERIAL}";
                            $rsp{ERROR_MSG} = $msg;
                            $bOK = 0;
                            last;
                        }
                    }
                }

                # If the device was either not found or was found to be
                # different than what the VCG thought it was we need to
                # stop the validation.
                if ($bOK == 0)
                {
                    $rsp{STATUS} = PI_ERROR;
                    $rsp{ERROR_CODE} = 0;

                    if (!defined($rsp{ERROR_MSG}) || $rsp{ERROR_MSG} eq "")
                    {
                        my $msg = "Device was not found on the controller.  " .
                                    "VCG PID: $pdisk1{PD_PID}";
                        $rsp{ERROR_MSG} = $msg;
                    }

                    last;
                }
            }
        }
    }

    logMsg("end\n");

    return %rsp;
}

#-- ##############################################################################
#-- # Name:     _validateDeviceListFE
#-- #
#-- # Desc:     Validates the device list found on the FE fibre on the VCG matches
#-- #           the list found on the controller.
#-- #
#-- # Input:    $self - Our command manager object.
#-- #           $obj - The command manager object for the controller to validate.
#-- #
#-- # Returns:  Hash with the following values:
#-- #               STATUS - Status code for this validation, PI_GOOD or one
#-- #                        of the PI error codes.
#-- #               ERROR_CODE - Error code for this validation.
#-- #               ERROR_MSG - Optional error message.
#-- ##############################################################################
#-- sub _validateDeviceListFE
#-- {
#--     my ($self, $obj) = @_;
#--     my %rsp;
#--     my $i;
#--     my $j;
#--     my %devlist1;
#--     my %devlist2;
#-- 
#--     logMsg("begin\n");
#-- 
#--     $rsp{STATUS} = PI_GOOD;
#--     $rsp{ERROR_CODE} = 0;
#-- 
#--     # Get the FE device list for the VCG
#--     if ($rsp{STATUS} == PI_GOOD)
#--     {
#--         %devlist1 = $self->deviceList("FE", 0);
#-- 
#--         if (%devlist1)
#--         {
#--             if ($devlist1{STATUS} != PI_GOOD)
#--             {
#--                 $rsp{STATUS} = $devlist1{STATUS};
#--                 $rsp{ERROR_CODE} = $devlist1{ERROR_CODE};
#--                 $rsp{ERROR_MSG} = "Failed to retreive FE device list for VCG.";
#--             }
#--         }
#--         else
#--         {
#--             $rsp{STATUS} = PI_ERROR;
#--             $rsp{ERROR_CODE} = 0;
#--             $rsp{ERROR_MSG} = "Failed to retreive FE device list for VCG.";
#--         }
#--     }
#-- 
#--     # Get the FE device list for the controller
#--     if ($rsp{STATUS} == PI_GOOD)
#--     {
#--         %devlist2 = $obj->deviceList("FE", 0);
#-- 
#--         if (%devlist2)
#--         {
#--             if ($devlist2{STATUS} != PI_GOOD)
#--             {
#--                 $rsp{STATUS} = $devlist2{STATUS};
#--                 $rsp{ERROR_CODE} = $devlist2{ERROR_CODE};
#--                 $rsp{ERROR_MSG} = "Failed to retreive FE device list for controller.";
#--             }
#--         }
#--         else
#--         {
#--             $rsp{STATUS} = PI_ERROR;
#--             $rsp{ERROR_CODE} = 0;
#--             $rsp{ERROR_MSG} = "Failed to retreive FE device list for controller.";
#--         }
#--     }
#-- 
#--     # Make sure the VCG and controller have the same number of devices
#--     if ($rsp{STATUS} == PI_GOOD)
#--     {
#--         if ($devlist1{NDEVS} != $devlist2{NDEVS})
#--         {
#--             $rsp{STATUS} = PI_ERROR;
#--             $rsp{ERROR_CODE} = 0;
#--             $rsp{ERROR_MSG} = "VCG and controller do not see the same number of FE devices.";
#--         }
#--     }
#-- 
#--     # Check to make sure the devices that the VCG and controller are the same
#--     # If the WWNs match they are the same device and we will check the
#--     # following values for equality:
#--     if ($rsp{STATUS} == PI_GOOD)
#--     {
#--         for (my $i = 0; $i < $devlist1{NDEVS}; $i++)
#--         {
#--             my $dev = $devlist1{LIST}[$i];
#--             my %dev1 = %$dev;
#-- 
#--             # Assume that the device is BAD until we find a match
#--             my $bOK = 0;
#-- 
#--             for (my $j = 0; $j < $devlist2{NDEVS}; $j++)
#--             {
#--                 my $dev = $devlist2{LIST}[$j];
#--                 my %dev2 = %$dev;
#-- 
#--                 printf "dev1: %8.8x%8.8x\n", $dev1{PORT_WWN_LO}, $dev1{PORT_WWN_HI};
#--                 printf "dev2: %8.8x%8.8x\n", $dev2{PORT_WWN_LO}, $dev2{PORT_WWN_HI};
#--                 printf "\n";
#-- 
#--                 # Is this the same device?  Check the WWNs...
#--                 if ($dev1{PORT_WWN_LO} == $dev2{PORT_WWN_LO} &&
#--                     $dev1{PORT_WWN_HI} == $dev2{PORT_WWN_HI})
#--                 {
#--                     # We have found the drive, until we find something wrong
#--                     # with it assume it is good.
#--                     $bOK = 1;
#--                     last;
#--                 }
#--             }
#-- 
#--             # If the drive was either not found or was found to be different
#--             # than what the VCG thought it was we need to stop the validation.
#--             if ($bOK == 0)
#--             {
#--                 $rsp{STATUS} = PI_ERROR;
#--                 $rsp{ERROR_CODE} = 0;
#--                 my $msg = sprintf("Failed to find a match for (%8.8x%8.8x).",
#--                                     $dev1{PORT_WWN_LO},
#--                                     $dev1{PORT_WWN_HI});
#--                 $rsp{ERROR_MSG} = $msg;
#--                 last;
#--             }
#--         }
#--     }
#-- 
#--     logMsg("end\n");
#-- 
#--     return %rsp;
#-- }

##############################################################################
# Name:  _electionStateResponsePacket
#
# Desc: Handles a generic response packet (no data expected)
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: Empty hash.
#
##############################################################################
sub _electionStateResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %parts;
    
    if (commandCode($recvPacket) == PI_DEBUG_GET_ELECTION_STATE_CMD)
    {
        %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};
        ($info{STATE}) = unpack("L", $parts{DATA});
        
        if ($info{STATE} == DEBUG_ED_STATE_END_TASK)
        {
            $info{STATE_MSG} = "Election State - End Task";
        }
        elsif ($info{STATE} == DEBUG_ED_STATE_BEGIN_ELECTION)
        {
            $info{STATE_MSG} = "Election State - Begin Election";
        }
        elsif ($info{STATE} == DEBUG_ED_STATE_CHECK_MASTERSHIP_ABILITY)
        {
            $info{STATE_MSG} = "Election State - Check Mastership Ability";
        }
        elsif ($info{STATE} == DEBUG_ED_STATE_TIMEOUT_CONTROLLERS)
        {
            $info{STATE_MSG} = "Election State - Timeout Controllers";
        }
        elsif ($info{STATE} == DEBUG_ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE)
        {
            $info{STATE_MSG} = "Election State - Timeout Controllers Complete";
        }
        elsif ($info{STATE} == DEBUG_ED_STATE_CONTACT_ALL_CONTROLLERS)
        {
            $info{STATE_MSG} = "Election State - Contact All Contollers";
        }
        elsif ($info{STATE} == DEBUG_ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE)
        {
            $info{STATE_MSG} = "Election State - Contact All Contollers Complete";
        }
        elsif ($info{STATE} == DEBUG_ED_STATE_WAIT_FOR_MASTER)
        {
            $info{STATE_MSG} = "Election State - Wait For Master";
        }
        elsif ($info{STATE} == DEBUG_ED_STATE_CHECK_MASTER)
        {
            $info{STATE_MSG} = "Election State - Check Master";
        }
        elsif ($info{STATE} == DEBUG_ED_STATE_NOTIFY_SLAVES)
        {
            $info{STATE_MSG} = "Election State - Notify Slaves";
        }
        elsif ($info{STATE} == DEBUG_ED_STATE_FAILED)
        {
            $info{STATE_MSG} = "Election State - Failed";
        }
        elsif ($info{STATE} == DEBUG_ED_STATE_FINISHED)
        {
            $info{STATE_MSG} = "Election State - Finished";
        }
        else
        {
            $info{STATE_MSG} = "Election State - UNDEFINED STATE!";
        }        
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual disk info packet\n");
    }
    
    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    return %info;
}

1;

#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
