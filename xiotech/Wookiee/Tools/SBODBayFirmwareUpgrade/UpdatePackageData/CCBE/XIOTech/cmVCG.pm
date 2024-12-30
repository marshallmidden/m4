# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Anthony Asleson
#
# Purpose:
#   Wrapper for all the different XIOTech commands that can be sent
#   to the XIOtech SAN system
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
#           $protocol   0 = PI      1 = X1
#
# Returns:  Hash with the following values:
#               STATUS - Status code for this validation, PI_GOOD or one
#                        of the PI error codes.
#               ERROR_CODE - Error code for this validation.
#               ERROR_MSG - Optional error message.
##############################################################################
sub vcgValidation
{
    my ($self, $flags, $protocol) = @_;

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

    # Handle command based on the protocol
    if ($protocol == 0)
    {
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
    else
    {
        # Send request using X1 protocol
        my $rc;
        my $data = pack("S", $flags);
        my $packet = assembleX1Packet(X1PKT_VCG_VALIDATION, $data);

        my %rsp = $self->_handleX1SyncResponse($packet,
                                               X1RPKT_VCG_VALIDATION,
                                               \&_handleResponseX1StatusOnlyGood0);
        if (%rsp)
        {
            logMsg("X1VCGValidation successful\n");
            $rc = 1;
        }
        else
        {
            logMsg("X1VCGValidation failed\n");
            $rc = 0;
        }
    
        return %rsp;
    }                                        
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

                    # Is this the same device?  Check the WWNs...
                    if ($bay1{WWN_LO} == $bay2{WWN_LO} &&
                        $bay1{WWN_HI} == $bay2{WWN_HI})
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
    $retryCount = 30;

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
                $rsp{ERROR_MSG} = "Failed to retreive physical disks for controller.";
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

                    # Is this the same device?  Check the WWNs...
                    if ($pdisk1{WWN_LO} == $pdisk2{WWN_LO} &&
                        $pdisk1{WWN_HI} == $pdisk2{WWN_HI})
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

##############################################################################
# Name:     _validateDeviceListFE
#
# Desc:     Validates the device list found on the FE fibre on the VCG matches
#           the list found on the controller.
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
sub _validateDeviceListFE
{
    my ($self, $obj) = @_;
    my %rsp;
    my $i;
    my $j;
    my %devlist1;
    my %devlist2;

    logMsg("begin\n");

    $rsp{STATUS} = PI_GOOD;
    $rsp{ERROR_CODE} = 0;

    # Get the FE device list for the VCG
    if ($rsp{STATUS} == PI_GOOD)
    {
        %devlist1 = $self->deviceList("FE", 0);

        if (%devlist1)
        {
            if ($devlist1{STATUS} != PI_GOOD)
            {
                $rsp{STATUS} = $devlist1{STATUS};
                $rsp{ERROR_CODE} = $devlist1{ERROR_CODE};
                $rsp{ERROR_MSG} = "Failed to retreive FE device list for VCG.";
            }
        }
        else
        {
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
            $rsp{ERROR_MSG} = "Failed to retreive FE device list for VCG.";
        }
    }

    # Get the FE device list for the controller
    if ($rsp{STATUS} == PI_GOOD)
    {
        %devlist2 = $obj->deviceList("FE", 0);

        if (%devlist2)
        {
            if ($devlist2{STATUS} != PI_GOOD)
            {
                $rsp{STATUS} = $devlist2{STATUS};
                $rsp{ERROR_CODE} = $devlist2{ERROR_CODE};
                $rsp{ERROR_MSG} = "Failed to retreive FE device list for controller.";
            }
        }
        else
        {
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
            $rsp{ERROR_MSG} = "Failed to retreive FE device list for controller.";
        }
    }

    # Make sure the VCG and controller have the same number of devices
    if ($rsp{STATUS} == PI_GOOD)
    {
        if ($devlist1{NDEVS} != $devlist2{NDEVS})
        {
            $rsp{STATUS} = PI_ERROR;
            $rsp{ERROR_CODE} = 0;
            $rsp{ERROR_MSG} = "VCG and controller do not see the same number of FE devices.";
        }
    }

    # Check to make sure the devices that the VCG and controller are the same
    # If the WWNs match they are the same device and we will check the
    # following values for equality:
    if ($rsp{STATUS} == PI_GOOD)
    {
        for (my $i = 0; $i < $devlist1{NDEVS}; $i++)
        {
            my $dev = $devlist1{LIST}[$i];
            my %dev1 = %$dev;

            # Assume that the device is BAD until we find a match
            my $bOK = 0;

            for (my $j = 0; $j < $devlist2{NDEVS}; $j++)
            {
                my $dev = $devlist2{LIST}[$j];
                my %dev2 = %$dev;

                printf "dev1: %8.8x%8.8x\n", $dev1{PORT_WWN_LO}, $dev1{PORT_WWN_HI};
                printf "dev2: %8.8x%8.8x\n", $dev2{PORT_WWN_LO}, $dev2{PORT_WWN_HI};
                printf "\n";

                # Is this the same device?  Check the WWNs...
                if ($dev1{PORT_WWN_LO} == $dev2{PORT_WWN_LO} &&
                    $dev1{PORT_WWN_HI} == $dev2{PORT_WWN_HI})
                {
                    # We have found the drive, until we find something wrong
                    # with it assume it is good.
                    $bOK = 1;
                    last;
                }
            }

            # If the drive was either not found or was found to be different
            # than what the VCG thought it was we need to stop the validation.
            if ($bOK == 0)
            {
                $rsp{STATUS} = PI_ERROR;
                $rsp{ERROR_CODE} = 0;
                my $msg = sprintf("Failed to find a match for (%8.8x%8.8x).",
                                    $dev1{PORT_WWN_LO},
                                    $dev1{PORT_WWN_HI});
                $rsp{ERROR_MSG} = $msg;
                last;
            }
        }
    }

    logMsg("end\n");

    return %rsp;
}

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

##############################################################################
# Change log:
# $Log$
# Revision 1.2  2006/07/17 20:38:32  RustadM
# TBolt00014770
# Move 750 branch onto main.
#
# Revision 1.1.1.1.30.6  2006/06/14 03:02:03  BharadwajS
# TBolt00014440
# Validation to prevent VCG_CONFIGURE_CMD after configuration
#
# Revision 1.1.1.1.30.5  2006/06/13 11:57:40  BharadwajS
# TBolt00014440
# Code to restart PI Server after VCG_CONFIGURE_CMD
#
# Revision 1.1.1.1.30.4  2006/05/18 04:36:31  BharadwajS
# Adding Node ID to PI_VCG_CONFIGURE_REQ
#
# Revision 1.1.1.1.30.3  2006/05/02 07:56:52  BharadwajS
# PI changes for initial controller configuration
#
# Revision 1.1.1.1.30.2  2006/04/28 05:49:44  BharadwajS
# PI for VCG_CONFIGURE_CMD
#
# Revision 1.1.1.1.30.1  2006/04/26 09:06:20  BharadwajS
# CCBE changes for PI Versioning
#
# Revision 1.1.1.1  2005/05/04 18:53:54  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.55  2004/12/29 22:29:25  NigburC
# TBolt00000000 - Added code to retry the retrieval of the physical disks for
# the controller being added upto 30 times (separated by a one second delay).
# Reviewed by Craig Menning.
#
# Revision 1.54  2004/12/29 19:16:28  NigburC
# TBolt00000000 - Added in some additional controller validation to be used
# when adding a controller to a group.  This code checks to make sure the
# controller is in the POWER UP WAIT FOR LICENSE state before allowing
# it to be added to a group.
# Reviewed by Craig Menning.
#
# Revision 1.53  2004/12/16 19:12:02  SchibillaM
# TBolt00011891: Implement PI_StatsServers - stats for all valid servers on a  controller.
# Reviewed by Chris.
#
# Revision 1.52  2004/09/21 15:24:30  WilliamsJ
# TBolt00011344 - Merge of resync into main.
#
# Revision 1.50.6.1  2004/09/20 14:57:55  WilliamsJ
# TBolt00000000 - Merge from Main (ASYNC_TAG2).  Reviewed by Chris.
#
# Revision 1.51  2004/09/08 14:41:35  RysavyR
# TBolt00011233: Add FW compatibility checking back in to Wookiee powerup.
# Rev by Bryan Holty.
#
# Revision 1.50  2004/02/19 22:16:37  NigburC
# TBolt00010073 - Added code to the add controller function to check if the
# controller being added has a node ID that is less than the maximum number
# of controllers limit.  The ICON has a similar restriction by only showing the
# controllers to be configured up to the maximum limit.
# Reviewed by Mark Schibilla.
#
# Revision 1.49  2004/01/23 20:54:05  RysavyR
# TBolt00000000: Remove some extraneous crlf's.
#
# Revision 1.48  2003/11/11 20:19:17  SchibillaM
# TBolt00000000: Add support for FID 300 - VCG Info.
#
# Revision 1.47  2003/10/01 12:09:00  NigburC
# TBolt00000000 - Removed the target field from the items checked when
# verifying firmware compatibility before adding a controller.
# Reviewed by Randy Rysavy.
#
# Revision 1.46  2003/08/25 21:43:36  McmasterM
# TBolt00008602: GeoRAID: Add "disaster mode" recovery state to CCB startup
# Changed the way the disaster safeguard was being set and reset by the system.
# This work is done to support Chris while he tests the powerup changes, but does
# not completely finish off this defect.
#
# Revision 1.45  2003/07/14 20:34:02  MenningC
# TBOLT00008741: added MP list to failover, added xtc loop count logging to failover. Reviewed by Chris
#
# Revision 1.44  2003/06/09 20:48:54  HoltyB
# TBolt00008278:  Removed the single path log messages, since these are
# handled by validation anyway.  Now the single path event will kick off a
# storage validation.  Added checks in the path validation to see if a whole
# fibrebay has lost a path(s).  If this is the case, only one validation log message
# will be seen indicating the fibrebay has bad paths.
# Reviewed by Mark Schibilla.
#
# Revision 1.43  2003/05/07 19:52:14  NigburC
# TBolt00008251 - Modified the _vcgAddSlave private function to accept the
# serial number of the controller being added.  This function is only called from
# within the cmVCG.pm module.
# Modified the vcgApply function to accept max controller values from 1 to 8.
# Reviewed by Jeff (PERL God) Williams.
#
# Revision 1.42  2003/04/08 21:21:12  MenningC
# TBOLT00000000 contoller to controller; reviewed by Bryan
#
# Revision 1.41  2003/01/16 20:49:13  RysavyR
# Changed AM_I_MASTER = TRUE/FALSE to RANK = MASTER/SLAVE for
# clarity.
#
# Revision 1.40  2003/01/02 17:28:31  NigburC
# TBolt00006588 - Added a packet and the corresponding functions to retrieve
# the VCG mirror partner list.
# Reviewed by Bryan Holty.
#
# Revision 1.39  2002/12/03 12:57:17  NigburC
# TBolt00006389 - Added additional checking in the VCGADDCONTROLLER
# command to ensure that the controller being added has the same CNC ID as
# the group and the CN ID is not already being used in the group.
# Reviewed by Mark Schibilla (virtually).
#
# Revision 1.38  2002/11/20 23:01:20  NigburC
# TBolt00006355 - Fixed a bug that Craig made me introduce.
# Reviewed by Craig Menning.
#
# Revision 1.37  2002/11/19 19:54:48  NigburC
# TBolt00000000 - VCGAddController was allowing controllers beyond the
# license limit to be added to the group.
# Reviewed By Craig Menning.
#
# Revision 1.36  2002/11/15 20:35:39  NigburC
# TBolt00005791 - Added new command handler functions for the VCG activate
# and inactivate commands.
# Reviewed by Mark Schibilla.
#
# Revision 1.35  2002/11/13 17:21:00  NigburC
# TBolt00006310 - Changes to force the CNC and CN ID values to be entered
# through the serial console before the Bigfoot controller really powers up.  This
# includes changes for the serial console menu system and the cntlSetup
# structure.  It also changes the way a controller is considered to be licensed.
# Reviewed by Tim Swatosh.
#
# Revision 1.34  2002/10/30 13:19:06  NigburC
# TBolt00006072 - Serial number changes to generate the controller serial
# numbers from the VCGID.  This includes changes in the applying of licenses,
# power-up sequencing and controller configuration.
# Reviewed by Steve Howe and Mark Schibilla.
#
# Revision 1.33  2002/09/18 20:32:40  NigburC
# TBolt00006022, TBolt00004962 - Added a retry when sending the PING
# before unfailing a controller in case the session was in a bad state.
# Added new command to the CCBE and packet interface to initiate the
# group redundancy validation (VCGVALIDATION).
# Reviewed by Tim Swatosh.
#
# Revision 1.32  2002/09/03 14:32:52  SchibillaM
# TBolt00005858: Changes to CCBCL files to allow a subset of function to be built
# for field use.  The tool BuildCCBCLSubset.pl builds the subset.  These changes
# also remove ENVSTATS which is replaced by STATSENV.
# Reviewed by Randy and Craig.
#
# Revision 1.31  2002/07/29 19:52:46  McmasterM
# TBolt00002740: CCB Zion I2C support - SDIMM battery (partial completion)
#
# Revision 1.30  2002/07/25 21:35:47  NigburC
# TBolt00000000 - Added additional class descriptions for new pdisk class
# types.  Removed the check for matching CLASS types when validating
# physical disks.
# Reviewed by Tim Swatosh.
#
# Revision 1.29  2002/07/25 18:58:58  HoltyB
# TBolt00005346:  Finishing touches on VCG Shutdown
#
# Revision 1.28  2002/07/24 16:45:10  RysavyR
# Change VENDOR_ID to SYSTEM_RLS in firmware header hash. Rev by Chris N.
#
# Revision 1.27  2002/07/14 13:33:42  NigburC
# TBolt00000000 - Added additional print statements explaining that a reset
# of the controller was occurring in the process of the add controller.
#
# Revision 1.26  2002/06/28 15:40:48  NigburC
# TBolt00005117 - Added check in VCGADDCONTROLLER to get the list
# of physical disks and look for ones with a serial number matching the VCGID.
# Reviewed by Craig Menning.
#
# Revision 1.25  2002/06/19 13:14:55  NigburC
# TBolt00000665 - The check for a valid VCG was incorrectly using the test
# of CONTROLLER_SN == VCGID instead of checking VCGID being not
# zero.
#
# Revision 1.24  2002/06/17 20:13:05  NigburC
# TBolt00000665 - Many changes associated with the power-up and licensing
# changes.  This changes the way we create a VCG and load VCGs.  It also
# changes the simulator so it holds the data in the VCG not the individual
# controllers.
#
# Revision 1.23  2002/05/14 15:50:17  RysavyR
# TBolt00001593:  Added VCG firmware "Rolling Update" method support.
#
# Revision 1.22  2002/05/14 14:51:44  NigburC
# TBolt00004329 - Display the serial numbers in hex for VCGInfo and
# SERIALNUMBERS commands.  Reviewed by Mark Schibilla.
#
# Revision 1.21  2002/05/07 19:10:10  McmasterM
# TBolt00002732: VCG Master Elections - Part 1/2
# This has the solution to the ContactAllControllersComplete timeout condition.
# Two new election states were added to support recovery from an election timeout.
# Also, now only the first set of packets are quorumable during the election.
#
# Revision 1.20  2002/04/30 20:04:46  NigburC
# TBolt00004033, TBolt00002733, TBolt00002730 - Lots of changes for these
# three defects.  Mainly, modified the VCGInfo request to return all controllers
# configured as part of the VCG instead of just active controllers.  This caused
# changes in CCB, CCBE and UMC code.
# Added the REMOVE, FAIL, UNFAIL, and SHUTDOWN methods for VCGs.
# Not all of these are working completely...just a stage check-in.
#
# Revision 1.19  2002/04/29 20:13:04  McmasterM
# TBolt00002732: VCG Master Elections - Part 1/2
# Added support for failing and unfailing of a controller, along with changes
# to support checking of mastership capability before deciding on the next
# master controller.  A new election state was added which drove changes to CCBE.
#
# Revision 1.18  2002/04/24 14:17:56  NigburC
# Tbolt00003999, TBolt00004000, TBolt00004001 - Added VCGAPPLYLICENSE,
# VCGUNFAILCONTROLLER and code to keep a failed controller failed.
#
# Revision 1.17  2002/04/09 16:23:37  NigburC
# TBolt00002733 - Found a spot where we were not generating an error
# message when something did not validate properly.  Also modified the
# formatting of the messages.
#
# Revision 1.16  2002/04/08 20:23:46  NigburC
# TBolt00002730 - Added a new method to add a controller to a group.
# Renamed the vcgValidateSlave method to vcgValidateController.
#
# Revision 1.15  2002/04/04 16:40:14  NigburC
# TBolt00002733 - Added basic validation for the controller being added to
# a group (make sure it is not part of a group).  Added validation for firmware
# versions.
# Removed check for same count of drive bays and physical disks.  This would
# have failed if there was a non-existant device.
#
# Revision 1.14  2002/04/01 16:47:31  HoltyB
# Added function to retrieve current election state
#
# Revision 1.13  2002/03/18 20:34:53  NigburC
# TBolt00003338, TBolt00002733 - Added CCBE side of the command to
# retrieve the information for all disk bays in one request.
# Added the first basic implementation of controller validation.  This includes
# the VCGVALIDATESLAVE command handler and the framework to add
# additional validation steps.
#
# Revision 1.12  2002/02/11 19:08:03  HoltyB
# moved command codes to PI_CommandCodes.pm
#
# Revision 1.11  2002/02/05 23:20:09  NigburC
# TBOLT00002954,
# TBOLT00002877,
# TBOLT00002730
#
# This set of changes include the above work items and defects.  Also are
# changes associated with MRP packet structure changes.
# We have moved many functions from CPS_INIT.C to PI_VCG.C in order
# to start returning error codes from the functions that work on the VCG.
# New functions have been added including but not limited to:
#   RESCANDEVICE
#   TARGETRESLIST
#   SERVERLOOKUP (not yet implemented in CCBE and CCBCL)
#   VDISKOWNER
#
# Revision 1.10  2002/01/22 12:51:30  NigburC
# TBolt00002858 - Added the VCGSETMIRRORPARTNERS command to the
# CCBE and CCBCL.
#
# Revision 1.9  2002/01/15 20:59:17  HoltyB
# fixed the ip display in vcginfo and vcgcontrollerinfo to display the actual ip
# address
#
# Revision 1.8  2001/12/27 19:46:53  NigburC
# Modified the code for VCGInfo to not pass a parameter, it was not used anyway.
# Fixed code in the display method for VCGInfo to print the controller serial
# numbers correctly.
#
# Revision 1.7  2001/11/27 17:06:34  NigburC
# Updated the max values for the serial number parameter in vcgctrlinfo and
# for the vcgid parameter in vcginfo.
#
# Revision 1.6  2001/11/26 21:32:21  NigburC
# Fixed incorrect command code for VCG_CONTROLLER_INFO.
# Fixed incorrect hash value used when displaying VCG_INFO.
#
# Revision 1.5  2001/11/26 20:00:50  NigburC
# Removed unused fields from the VCG Controller Information return packet.
#
# Revision 1.4  2001/11/16 16:56:55  SwatoshT
# Added support for VCG PREPARE SLAVE, VCG ADD SLAVE, and VCG PING
#
# Revision 1.3  2001/11/15 22:11:49  NigburC
# Added vcgMakeSingle.
#
# Revision 1.2  2001/11/14 13:03:04  NigburC
# Modified verify parameters to return a hash instead of just the message.
# This will allow the code to return it to the command line where it will be
# processed like any other error, except there is a MESSAGE in the
# hash which will be displayed describing the parameter error that occurred.
#
# Revision 1.1  2001/11/14 12:48:51  NigburC
# Initial integration.
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
