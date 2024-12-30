# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Tim Swatosh
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


##############################################################################
# Name:     piEnvIIGet
#
# Desc:     Gets all the environmental statistics for a controller
#           and disk bays.
#
# In:       None
#
# Returns:  Returns an empty hash if errors occur else returns a
#           hash with the following information
##############################################################################
sub piEnvIIGet
{
    my ($self) = @_;

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket(PI_ENV_II_GET_CMD,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_envIIdata);
}

##############################################################################
# Name:     _envIIdata
#
# Desc:     Parses the controller and disk bay info packet and place the 
#           information in a hash
#
# In:       scalar  $packet to be parsed 
#
# Returns:
#
##############################################################################
sub _envIIdata
{
    my ($self, $seq, $recvPacket) = @_;
    my %envII;
    
    if( !(defined($recvPacket)) )
    {
        $recvPacket = $self->_receivePacketSync($seq);
    }

    if (commandCode($recvPacket) == PI_ENV_II_GET_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);
#        printf "length 0x%08X\n", $parts{DATA_LENGTH};
        
        %envII = $self->_envIIdataFmt($parts{DATA});
        
        # Handle the status and error code values        
        $envII{STATUS} = $parts{STATUS};
        $envII{ERROR_CODE} = $parts{ERROR_CODE};
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected the PI version of the X1 Environmental packet\n");
    }

    return %envII;
}

sub _envIIdataFmt
{
    my ($self, $envdata) = @_;
    my %envII;
    
    my $ENVII_DEV       = "LLa16SSSSa64LL";
    my $ENVII_MSR       = "LLLLa64LL";
    my $ENVII_DATA      = "LLLLLLLLLL";

    my $offset = 0;
    my $total_devs = 0;
    
    $envII{DEVCOUNT} = 0;

    while ( $envII{DEVCOUNT} <= $total_devs )
    {
        my $data = substr($envdata, $offset);

        (   $envII{$envII{DEVCOUNT}}{LENGTH},
            $envII{$envII{DEVCOUNT}}{ID},  
            $envII{$envII{DEVCOUNT}}{UUID},  
            $envII{$envII{DEVCOUNT}}{TYPE},  
            $envII{$envII{DEVCOUNT}}{STATUS},  
            $envII{$envII{DEVCOUNT}}{NUM_MSR},  
            $envII{$envII{DEVCOUNT}}{NUM_DEV},  
            $envII{$envII{DEVCOUNT}}{NAME},  
            $envII{$envII{DEVCOUNT}}{FLAGS},  
            $envII{$envII{DEVCOUNT}}{DYN_OFF} 
        ) = unpack ($ENVII_DEV, $data);

        $total_devs += $envII{$envII{DEVCOUNT}}{NUM_DEV};
        $offset += $envII{$envII{DEVCOUNT}}{DYN_OFF};


        for ( my $mindex = 0; $mindex < $envII{$envII{DEVCOUNT}}{NUM_MSR}; ++$mindex )
        {
            $data = substr($envdata, $offset);

            (   $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{LENGTH},
                $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{ID},  
                $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{TYPE},  
                $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{STATUS},  
                $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{NAME},  
                $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{FLAGS},  
                $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{DYN_OFF} 
            ) = unpack ($ENVII_MSR, $data);

            $data = substr($envdata, ($offset + $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{DYN_OFF}));

            (   $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{DATA_LO},
                $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{DATA_HI},
                $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MAX_ERROR_LO},  
                $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MAX_ERROR_HI},  
                $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MAX_WARN_LO},  
                $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MAX_WARN_HI},  
                $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MIN_ERROR_LO},  
                $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MIN_ERROR_HI},  
                $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MIN_WARN_LO},
                $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MIN_WARN_HI}
            ) = unpack ($ENVII_DATA, $data);

            $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{DATA} = 
                (Math::BigInt->new($envII{$envII{DEVCOUNT}}{MSR}[$mindex]{DATA_HI}) << 32) | 
                (Math::BigInt->new($envII{$envII{DEVCOUNT}}{MSR}[$mindex]{DATA_LO}));
            $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MAX_ERROR} = 
                (Math::BigInt->new($envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MAX_ERROR_HI}) << 32) | 
                (Math::BigInt->new($envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MAX_ERROR_LO}));
            $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MAX_WARN} = 
                (Math::BigInt->new($envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MAX_WARN_HI}) << 32) | 
                (Math::BigInt->new($envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MAX_WARN_LO}));
            $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MIN_ERROR} = 
                (Math::BigInt->new($envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MIN_ERROR_HI}) << 32) | 
                (Math::BigInt->new($envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MIN_ERROR_LO}));
            $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MIN_WARN} = 
                (Math::BigInt->new($envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MIN_WARN_HI}) << 32) | 
                (Math::BigInt->new($envII{$envII{DEVCOUNT}}{MSR}[$mindex]{MIN_WARN_LO}));

            $offset += $envII{$envII{DEVCOUNT}}{MSR}[$mindex]{LENGTH};
        }
        
        if ( !$envII{$envII{DEVCOUNT}}{NUM_MSR} )
        {
            $offset += $envII{$envII{DEVCOUNT}}{LENGTH};
        }

        ++$envII{DEVCOUNT};
    }

    return %envII;
}
##############################################################################
# NAME:     envIIDisplay
#
# DESC:     Print the loop statistics
#
# INPUT:    TYPE - Which statistics
#           HASH - Loop Statistics Hash
##############################################################################
sub envIIDisplay
{
    my ($self, %envII) = @_;

    my $msg = "";
    my $total_devs = 0;

    logMsg("begin\n");

    while ( $total_devs < $envII{DEVCOUNT} )
    {
        $msg .= sprintf "Device id %d *******\n", $envII{$total_devs}{ID};
        $msg .= sprintf "  name           %s\n", $envII{$total_devs}{NAME};
        $msg .= sprintf "  length         %d\n", $envII{$total_devs}{LENGTH};
        $msg .= sprintf "  uuid           %s\n", $envII{$total_devs}{UUID};
        $msg .= sprintf "  type           %d\n", $envII{$total_devs}{TYPE};
        $msg .= sprintf "  status         %d\n", $envII{$total_devs}{STATUS};
        $msg .= sprintf "  # measures     %d\n", $envII{$total_devs}{NUM_MSR};
        $msg .= sprintf "  # devices      %d\n", $envII{$total_devs}{NUM_DEV};
        $msg .= sprintf "  flags          0x%08X\n", $envII{$total_devs}{FLAGS};
        $msg .= sprintf "  dyn_off        0x%08X\n", $envII{$total_devs}{DYN_OFF};


        for ( my $mindex = 0; $mindex < $envII{$total_devs}{NUM_MSR}; ++$mindex )
        {
            $msg .= sprintf "    + Measure id %d +++++++\n", $envII{$total_devs}{MSR}[$mindex]{ID};
            $msg .= sprintf "        name         %s\n", $envII{$total_devs}{MSR}[$mindex]{NAME};
            $msg .= sprintf "        length       %d\n", $envII{$total_devs}{MSR}[$mindex]{LENGTH};
            $msg .= sprintf "        type         %d\n", $envII{$total_devs}{MSR}[$mindex]{TYPE};
            $msg .= sprintf "        status       %d\n", $envII{$total_devs}{MSR}[$mindex]{STATUS};
            $msg .= sprintf "        flags        0x%08X\n", $envII{$total_devs}{MSR}[$mindex]{FLAGS};
            $msg .= sprintf "        dyn_off      0x%08X\n", $envII{$total_devs}{MSR}[$mindex]{DYN_OFF};
            $msg .= sprintf "    ----DATA---------------\n";
            $msg .= sprintf "          value      %s\n", $envII{$total_devs}{MSR}[$mindex]{DATA};
            $msg .= sprintf "          max_error  %s\n", $envII{$total_devs}{MSR}[$mindex]{MAX_ERROR};
            $msg .= sprintf "          max_warn   %s\n", $envII{$total_devs}{MSR}[$mindex]{MAX_WARN};
            $msg .= sprintf "          min_error  %s\n", $envII{$total_devs}{MSR}[$mindex]{MIN_ERROR};
            $msg .= sprintf "          min_warn   %s\n", $envII{$total_devs}{MSR}[$mindex]{MIN_WARN};
            
            $msg .= sprintf "\n";
        }

        ++$total_devs;
    }

    $msg .= sprintf "\n";

    return $msg;

}


##############################################################################
# Name:     environmentalStatsExtended
#
# Desc:     Gets all the environmental statistics for a controller
#
# In:       None
#
# Returns:  Returns an empty hash if errors occur else returns a
#           hash with the following information
##############################################################################
sub environmentalStatsExtended
{
    my ($self) = @_;

    # verify parameters
    my $args = [['i'],
                ["environmentalStatsExtended"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket(PI_STATS_ENVIRONMENTAL_CMD,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_envStatsExtPacket);
}

##############################################################################
# Name:     piGetX1Env
#
# Desc:     Gets all the environmental statistics for a controller
#           and disk bays.  The request is made via the PI interface
#           but the data is returned using the X1 packet definition.
#
# In:       None
#
# Returns:  Returns an empty hash if errors occur else returns a
#           hash with the following information
##############################################################################
sub piGetX1Env
{
    my ($self) = @_;

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket(PI_ENVIRO_DATA_CTRL_AND_BAY_CMD,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_envCtrlAndBayPacket);
}

##############################################################################
# Name: displayEnvironmentalStatsExtended
#
# Desc: Print the environmental statistics information
#
# In:   Environmental Statistics Hash
#
##############################################################################
sub displayEnvironmentalStatsExtended
{
    my ($self, %envStats) = @_;

    my $msg = "";

    logMsg( "displayEnvironentalStatsExtended...begin\n" );

    $msg .= sprintf( "Monitor Status Information ----------------------\n" );
    $msg .= sprintf( "  Event Status Code:                        %s\n", _getI2CMonitorStatusCodeString($envStats{MONITOR_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "    Warning:                                %d\n", $envStats{MONITOR_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "    Error:                                  %d\n", $envStats{MONITOR_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "    Flags:                                  %s\n", _getI2CMonitorEventFlagsString($envStats{MONITOR_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Description:                            %s\n", $envStats{MONITOR_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "*************************************************\n" );
    $msg .= sprintf( "-- CCB Status -----------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{CCB_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{CCB_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{CCB_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{CCB_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{CCB_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "---- NVRAM Battery Status -----------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{NVRAM_BATTERY_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{NVRAM_BATTERY_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{NVRAM_BATTERY_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{NVRAM_BATTERY_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{NVRAM_BATTERY_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorNVRAMBatteryConditionString($envStats{NVRAM_BATTERY_CONDITION}) );
    $msg .= sprintf( "---- Board EEPROM Status ------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{CCB_BOARD_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{CCB_BOARD_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{CCB_BOARD_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{CCB_BOARD_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{CCB_BOARD_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorEEPROMConditionString($envStats{CCB_BOARD_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , _getI2CMonitorXCIDataString($envStats{CCB_BOARD_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "---- Memory EEPROM Status -----------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{CCB_MEMORY_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{CCB_MEMORY_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorEEPROMConditionString($envStats{CCB_MEMORY_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , _getI2CMonitorXCIDataString($envStats{CCB_MEMORY_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "*************************************************\n" );
    $msg .= sprintf( "-- Processor Board ------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{PROC_BOARD_STATUS_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_BOARD_STATUS_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_BOARD_STATUS_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{PROC_BOARD_STATUS_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{PROC_BOARD_STATUS_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "---- Power Supply Voltages ----------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Voltage Levels\n" );
    $msg .= sprintf( "      12 Volt:        %5dmV (%s)  Range from %5d to %5d\n",
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_LMV}),
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_MIN_MV},
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_MAX_MV} );
    $msg .= sprintf( "      5 Volt:         %5dmV (%s)  Range from %5d to %5d\n",
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_LMV}),
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_MIN_MV},
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_MAX_MV} );
    $msg .= sprintf( "      3.3 Volt:       %5dmV (%s)  Range from %5d to %5d\n",
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_LMV}),
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_MIN_MV},
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_MAX_MV} );
    $msg .= sprintf( "      5 Volt Standby: %5dmV (%s)  Range from %5d to %5d\n",
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_LMV}),
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_MIN_MV},
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_MAX_MV} );
    $msg .= sprintf( "---- Front End Processor ------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Temperature Event Status Code:          %s\n", _getI2CMonitorStatusCodeString($envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Temperature: %d (%s)  Range from %d to %d\n",
                    $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_CURRENT_CELSIUS},
                    _getI2CMonitorTemperatureConditionString($envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_CONDITION}),
                    $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_MIN_CELSIUS},
                    $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_MAX_CELSIUS} );
    $msg .= sprintf( "    DIMM Socket Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_LMV}),
                    $envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_MIN_MV},
                    $envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_MAX_MV} );
    $msg .= sprintf( "    Reset Condition: %s\n", _getI2CMonitorProcessorResetConditionString($envStats{PROC_BOARD_STATUS_FE_PROC_PRCV} ) );
    $msg .= sprintf( "---- Back End Processor -------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Temperature Event Status Code:          %s\n", _getI2CMonitorStatusCodeString($envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Temperature: %d (%s)  Range from %d to %d\n",
                    $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_CURRENT_CELSIUS},
                    _getI2CMonitorTemperatureConditionString($envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_CONDITION}),
                    $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_MIN_CELSIUS},
                    $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_MAX_CELSIUS} );
    $msg .= sprintf( "    DIMM Socket Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_LMV}),
                    $envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_MIN_MV},
                    $envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_MAX_MV} );
    $msg .= sprintf( "    Reset Condition: %s\n", _getI2CMonitorProcessorResetConditionString($envStats{PROC_BOARD_STATUS_BE_PROC_PRCV}) );
    $msg .= sprintf( "---- Chassis EEPROM -----------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorEEPROMConditionString($envStats{PROC_CHASSIS_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , _getI2CMonitorXCIDataString($envStats{PROC_CHASSIS_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "---- Board EEPROM -------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{PROC_BOARD_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_BOARD_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_BOARD_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{PROC_BOARD_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{PROC_BOARD_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorEEPROMConditionString($envStats{PROC_BOARD_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , _getI2CMonitorXCIDataString($envStats{PROC_BOARD_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "*************************************************\n" );
    $msg .= sprintf( "-- Front End Power Supply -----------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Supply Condition:                       %s\n", _getI2CMonitorPowerSupplyConditionString($envStats{FE_POWER_SUPPLY_STATUS_PSCV}) );
    $msg .= sprintf( "      acFailedSampleCounter:                %d\n", $envStats{FE_POWER_SUPPLY_STATUS_PSCACFSC} );
    $msg .= sprintf( "      dcFailedSampleCounter:                %d\n", $envStats{FE_POWER_SUPPLY_STATUS_PSCDCFSC} );
    $msg .= sprintf( "    Cooling Fan Condition:                  %s\n", _getI2CMonitorCoolingFanConditionString($envStats{FE_POWER_SUPPLY_STATUS_CFCV}) );
    $msg .= sprintf( "---- Assembly EEPROM ----------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorEEPROMConditionString($envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , _getI2CMonitorXCIDataString($envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "---- Interface Board EEPROM ---------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorEEPROMConditionString($envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , _getI2CMonitorXCIDataString($envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "*************************************************\n" );
    $msg .= sprintf( "-- Back End Power Supply ------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Supply Condition:                       %s\n", _getI2CMonitorPowerSupplyConditionString($envStats{BE_POWER_SUPPLY_STATUS_PSCV}) );
    $msg .= sprintf( "      acFailedSampleCounter:                %d\n", $envStats{BE_POWER_SUPPLY_STATUS_PSCACFSC} );
    $msg .= sprintf( "      dcFailedSampleCounter:                %d\n", $envStats{BE_POWER_SUPPLY_STATUS_PSCDCFSC} );
    $msg .= sprintf( "    Cooling Fan Condition:                  %s\n", _getI2CMonitorCoolingFanConditionString($envStats{BE_POWER_SUPPLY_STATUS_CFCV}) );
    $msg .= sprintf( "---- Assembly EEPROM ----------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorEEPROMConditionString($envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , _getI2CMonitorXCIDataString($envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "---- Interface Board EEPROM ---------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorEEPROMConditionString($envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , _getI2CMonitorXCIDataString($envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "*************************************************\n" );
    $msg .= sprintf( "-- Front End Buffer Board -----------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "---- Temperature --------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Temperature: %d (%s)  Range from %d to %d\n",
                    $envStats{FE_BATT_SDIMM_STATUS_TEMP_CURRENT_CELSIUS},
                    _getI2CMonitorTemperatureConditionString($envStats{FE_BATT_SDIMM_STATUS_TEMP_CONDITION}),
                    $envStats{FE_BATT_SDIMM_STATUS_TEMP_MIN_CELSIUS},
                    $envStats{FE_BATT_SDIMM_STATUS_TEMP_MAX_CELSIUS} );
    $msg .= sprintf( "---- Battery ------------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Terminal Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_LMV}),
                    $envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MIN_MV},
                    $envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MAX_MV} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorBatteryConditionString($envStats{FE_BATT_SDIMM_STATUS_BATT_CONDITION}) );
    $msg .= sprintf( "---- Fuel Gauge ---------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Flow Rate:                              0x%08x\n", $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_CFR} );
    $msg .= sprintf( "    Regulator Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_LMV}),
                    $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MIN_MV},
                    $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MAX_MV} );
    $msg .= sprintf( "    Current Flow Condition:                 %s\n", _getI2CMonitorCurrentFlowConditionString($envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_CF_CONDITION}) );
    $msg .= sprintf( "    Fuel Gauge Condition:                   %s\n", _getI2CMonitorFuelGaugeConditionString($envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_FG_CONDITION}) );
    $msg .= sprintf( "---- Main Regulator -----------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Input Voltage:  %5dmV (%s)  Range from %5d to %5d\n",
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_LMV}),
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MIN_MV},
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MAX_MV} );
    $msg .= sprintf( "    Output Voltage: %5dmV (%s)  Range from %5d to %5d\n",
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_LMV}),
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MIN_MV},
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MAX_MV} );
    $msg .= sprintf( "    Supply Voltage: %5dmV (%s)  Range from %5d to %5d\n",
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_LMV}),
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MIN_MV},
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MAX_MV} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorMainRegulatorConditionString($envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_CONDITION}) );
    $msg .= sprintf( "---- Charger -------------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Condition:                            %s\n", _getI2CMonitorChargerConditionString($envStats{FE_BATT_SDIMM_STATUS_CHARGER_CONDITION}) );
    $msg .= sprintf( "---- EEPROM -------------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorEEPROMConditionString($envStats{FE_BATT_SDIMM_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , _getI2CMonitorXCIDataString($envStats{FE_BATT_SDIMM_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "*************************************************\n" );
    $msg .= sprintf( "-- Back End Buffer Board ------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "---- Temperature --------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Temperature: %d (%s)  Range from %d to %d\n",
                    $envStats{BE_BATT_SDIMM_STATUS_TEMP_CURRENT_CELSIUS},
                    _getI2CMonitorTemperatureConditionString($envStats{BE_BATT_SDIMM_STATUS_TEMP_CONDITION}),
                    $envStats{BE_BATT_SDIMM_STATUS_TEMP_MIN_CELSIUS},
                    $envStats{BE_BATT_SDIMM_STATUS_TEMP_MAX_CELSIUS} );
    $msg .= sprintf( "---- Battery ------------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Terminal Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_LMV}),
                    $envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MIN_MV},
                    $envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MAX_MV} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorBatteryConditionString($envStats{BE_BATT_SDIMM_STATUS_BATT_CONDITION}) );
    $msg .= sprintf( "---- Fuel Gauge ---------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Flow Rate:                              0x%08x\n", $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_CFR} );
    $msg .= sprintf( "    Regulator Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_LMV}),
                    $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MIN_MV},
                    $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MAX_MV} );
    $msg .= sprintf( "    Current Flow Condition:                 %s\n", _getI2CMonitorCurrentFlowConditionString($envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_CF_CONDITION}) );
    $msg .= sprintf( "    Fuel Gauge Condition:                   %s\n", _getI2CMonitorFuelGaugeConditionString($envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_FG_CONDITION}) );
    $msg .= sprintf( "---- Main Regulator -----------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Input Voltage:  %5dmV (%s)  Range from %5d to %5d\n",
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_LMV}),
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MIN_MV},
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MAX_MV} );
    $msg .= sprintf( "    Output Voltage: %5dmV (%s)  Range from %5d to %5d\n",
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_LMV}),
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MIN_MV},
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MAX_MV} );
    $msg .= sprintf( "    Supply Voltage: %5dmV (%s)  Range from %5d to %5d\n",
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_CURRENT_MV},
                    _getI2CMonitorLimitMonitorString($envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_LMV}),
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MIN_MV},
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MAX_MV} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorMainRegulatorConditionString($envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_CONDITION}) );
    $msg .= sprintf( "---- Charger ------------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorChargerConditionString($envStats{BE_BATT_SDIMM_STATUS_CHARGER_CONDITION}) );
    $msg .= sprintf( "---- EEPROM -------------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", _getI2CMonitorStatusCodeString($envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", _getI2CMonitorEventFlagsString($envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Description:                          %s\n", $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "    Condition:                              %s\n", _getI2CMonitorEEPROMConditionString($envStats{BE_BATT_SDIMM_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , _getI2CMonitorXCIDataString($envStats{BE_BATT_SDIMM_EEPROM_XCI_DATA}) );

    return $msg;
}

##############################################################################
# Name:     i2cStatsExtended
#
# Desc:     Gets all the I2C statistics for a controller
#
# In:       None
#
# Returns:  Returns an empty hash if errors occur else returns a
#           hash with the following information
##############################################################################
sub i2cStatsExtended
{
    my ($self) = @_;

    # verify parameters
    my $args = [['i'],
                ["i2cStatsExtended"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket(PI_STATS_I2C_CMD,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_i2cStatsExtPacket);
}

##############################################################################
# Name: displayI2cStatsExtended
#
# Desc: Print the I2C statistics information
#
# In:   I2C Statistics Hash
#
##############################################################################
sub displayI2cStatsExtended
{
    my ($self, %i2cStats) = @_;

    my $msg = "";

    logMsg( "displayI2cStatsExtended...begin\n" );

    $msg .= sprintf( "I2C Status --------------------------------------\n" );
    $msg .= sprintf( "  Event Status Code:                        %s\n", _getI2CMonitorStatusCodeString($i2cStats{EVENT_PROP_STATUS}) );
    $msg .= sprintf( "    Warning:                                %d\n", $i2cStats{EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "    Error:                                  %d\n", $i2cStats{EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "    Flags:                                  %s\n", _getI2CMonitorEventFlagsString($i2cStats{EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Description:                            %s\n", $i2cStats{EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "** CCB ******************************************\n" );
    $msg .= sprintf( "---- Processor ----------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_CCB_PROCESSOR_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_CCB_PROCESSOR_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_CCB_PROCESSOR_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_CCB_PROCESSOR_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_CCB_PROCESSOR_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_CCB_PROCESSOR_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_CCB_PROCESSOR_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_CCB_PROCESSOR_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_CCB_PROCESSOR_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_CCB_PROCESSOR_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_CCB_PROCESSOR_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_CCB_PROCESSOR_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_CCB_PROCESSOR_COMM_STATS_UDC} );
    $msg .= sprintf( "---- Board EEPROM -------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_CCB_BOARD_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_CCB_BOARD_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_CCB_BOARD_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_CCB_BOARD_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_CCB_BOARD_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_CCB_BOARD_EEPROM_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_CCB_BOARD_EEPROM_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_CCB_BOARD_EEPROM_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_CCB_BOARD_EEPROM_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_CCB_BOARD_EEPROM_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_CCB_BOARD_EEPROM_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_CCB_BOARD_EEPROM_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_CCB_BOARD_EEPROM_COMM_STATS_UDC} );
    $msg .= sprintf( "---- Memory Module EEPROM -----------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_CCB_MEMORY_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_CCB_MEMORY_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_CCB_MEMORY_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_CCB_MEMORY_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_CCB_MEMORY_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_CCB_MEMORY_EEPROM_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_CCB_MEMORY_EEPROM_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_CCB_MEMORY_EEPROM_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_CCB_MEMORY_EEPROM_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_CCB_MEMORY_EEPROM_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_CCB_MEMORY_EEPROM_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_CCB_MEMORY_EEPROM_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_CCB_MEMORY_EEPROM_COMM_STATS_UDC} );
    $msg .= sprintf( "** Proc Board ***********************************\n" );
    $msg .= sprintf( "---- PCA9548 ------------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_PROC_PCA9548_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_PROC_PCA9548_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_PROC_PCA9548_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_PROC_PCA9548_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_PROC_PCA9548_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_PROC_PCA9548_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_PROC_PCA9548_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_PROC_PCA9548_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_PROC_PCA9548_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_PROC_PCA9548_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_PROC_PCA9548_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_PROC_PCA9548_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_PROC_PCA9548_COMM_STATS_UDC} );
    $msg .= sprintf( "---- LM80/LM87 ----------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_PROC_LM80_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_PROC_LM80_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_PROC_LM80_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_PROC_LM80_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_PROC_LM80_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_PROC_LM80_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_PROC_LM80_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_PROC_LM80_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_PROC_LM80_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_PROC_LM80_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_PROC_LM80_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_PROC_LM80_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_PROC_LM80_COMM_STATS_UDC} );
    $msg .= sprintf( "---- LM75/LM92 ----------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_PROC_LM75_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_PROC_LM75_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_PROC_LM75_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_PROC_LM75_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_PROC_LM75_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_PROC_LM75_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_PROC_LM75_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_PROC_LM75_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_PROC_LM75_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_PROC_LM75_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_PROC_LM75_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_PROC_LM75_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_PROC_LM75_COMM_STATS_UDC} );
    $msg .= sprintf( "---- Reset Control PCF8574 ----------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_PROC_RCPCF8574_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_PROC_RCPCF8574_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_PROC_RCPCF8574_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_PROC_RCPCF8574_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_PROC_RCPCF8574_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_PROC_RCPCF8574_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_PROC_RCPCF8574_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_PROC_RCPCF8574_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_PROC_RCPCF8574_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_PROC_RCPCF8574_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_PROC_RCPCF8574_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_PROC_RCPCF8574_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_PROC_RCPCF8574_COMM_STATS_UDC} );
    $msg .= sprintf( "---- Power Supply PCF8574 -----------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_PROC_PSPCF8574_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_PROC_PSPCF8574_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_PROC_PSPCF8574_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_PROC_PSPCF8574_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_PROC_PSPCF8574_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_PROC_PSPCF8574_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_PROC_PSPCF8574_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_PROC_PSPCF8574_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_PROC_PSPCF8574_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_PROC_PSPCF8574_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_PROC_PSPCF8574_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_PROC_PSPCF8574_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_PROC_PSPCF8574_COMM_STATS_UDC} );
    $msg .= sprintf( "---- Chassis EEPROM -----------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_PROC_CHASSIS_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_PROC_CHASSIS_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_PROC_CHASSIS_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_PROC_CHASSIS_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_PROC_CHASSIS_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_PROC_CHASSIS_EEPROM_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_PROC_CHASSIS_EEPROM_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_PROC_CHASSIS_EEPROM_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_PROC_CHASSIS_EEPROM_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_PROC_CHASSIS_EEPROM_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_PROC_CHASSIS_EEPROM_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_PROC_CHASSIS_EEPROM_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_PROC_CHASSIS_EEPROM_COMM_STATS_UDC} );
    $msg .= sprintf( "---- Board EEPROM -------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_PROC_BOARD_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_PROC_BOARD_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_PROC_BOARD_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_PROC_BOARD_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_PROC_BOARD_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_PROC_BOARD_EEPROM_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_PROC_BOARD_EEPROM_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_PROC_BOARD_EEPROM_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_PROC_BOARD_EEPROM_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_PROC_BOARD_EEPROM_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_PROC_BOARD_EEPROM_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_PROC_BOARD_EEPROM_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_PROC_BOARD_EEPROM_COMM_STATS_UDC} );
    $msg .= sprintf( "** FE Power Supply ******************************\n" );
    $msg .= sprintf( "---- Assembly EEPROM ----------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_COMM_STATS_UDC} );
    $msg .= sprintf( "---- Interface Board EEPROM ---------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_COMM_STATS_UDC} );
    $msg .= sprintf( "** BE Power Supply ******************************\n" );
    $msg .= sprintf( "---- Assembly EEPROM ----------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_COMM_STATS_UDC} );
    $msg .= sprintf( "---- Interface Board EEPROM ---------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_COMM_STATS_UDC} );
    $msg .= sprintf( "** FE Buffer Board ******************************\n" );
    $msg .= sprintf( "---- LM80/LM87 ----------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_BATT_FE_LM80_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_BATT_FE_LM80_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_BATT_FE_LM80_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_BATT_FE_LM80_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_BATT_FE_LM80_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_BATT_FE_LM80_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_BATT_FE_LM80_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_BATT_FE_LM80_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_BATT_FE_LM80_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_BATT_FE_LM80_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_BATT_FE_LM80_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_BATT_FE_LM80_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_BATT_FE_LM80_COMM_STATS_UDC} );
    $msg .= sprintf( "---- PCF8574 ------------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_BATT_FE_PCF8574_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_BATT_FE_PCF8574_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_BATT_FE_PCF8574_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_BATT_FE_PCF8574_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_BATT_FE_PCF8574_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_BATT_FE_PCF8574_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_BATT_FE_PCF8574_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_BATT_FE_PCF8574_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_BATT_FE_PCF8574_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_BATT_FE_PCF8574_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_BATT_FE_PCF8574_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_BATT_FE_PCF8574_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_BATT_FE_PCF8574_COMM_STATS_UDC} );
    $msg .= sprintf( "---- MAX1660 ------------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_BATT_FE_MAX1660_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_BATT_FE_MAX1660_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_BATT_FE_MAX1660_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_BATT_FE_MAX1660_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_BATT_FE_MAX1660_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_BATT_FE_MAX1660_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_BATT_FE_MAX1660_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_BATT_FE_MAX1660_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_BATT_FE_MAX1660_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_BATT_FE_MAX1660_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_BATT_FE_MAX1660_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_BATT_FE_MAX1660_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_BATT_FE_MAX1660_COMM_STATS_UDC} );
    $msg .= sprintf( "---- EEPROM -------------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_BATT_FE_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_BATT_FE_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_BATT_FE_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_BATT_FE_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_BATT_FE_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_BATT_FE_EEPROM_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_BATT_FE_EEPROM_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_BATT_FE_EEPROM_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_BATT_FE_EEPROM_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_BATT_FE_EEPROM_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_BATT_FE_EEPROM_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_BATT_FE_EEPROM_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_BATT_FE_EEPROM_COMM_STATS_UDC} );
    $msg .= sprintf( "** BE Buffer Board ******************************\n" );
    $msg .= sprintf( "---- LM80/LM87 ----------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_BATT_BE_LM80_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_BATT_BE_LM80_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_BATT_BE_LM80_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_BATT_BE_LM80_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_BATT_BE_LM80_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_BATT_BE_LM80_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_BATT_BE_LM80_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_BATT_BE_LM80_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_BATT_BE_LM80_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_BATT_BE_LM80_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_BATT_BE_LM80_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_BATT_BE_LM80_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_BATT_BE_LM80_COMM_STATS_UDC} );
    $msg .= sprintf( "---- PCF8574 ------------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_BATT_BE_PCF8574_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_BATT_BE_PCF8574_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_BATT_BE_PCF8574_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_BATT_BE_PCF8574_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_BATT_BE_PCF8574_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_BATT_BE_PCF8574_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_BATT_BE_PCF8574_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_BATT_BE_PCF8574_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_BATT_BE_PCF8574_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_BATT_BE_PCF8574_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_BATT_BE_PCF8574_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_BATT_BE_PCF8574_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_BATT_BE_PCF8574_COMM_STATS_UDC} );
    $msg .= sprintf( "---- MAX1660 ------------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_BATT_BE_MAX1660_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_BATT_BE_MAX1660_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_BATT_BE_MAX1660_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_BATT_BE_MAX1660_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_BATT_BE_MAX1660_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_BATT_BE_MAX1660_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_BATT_BE_MAX1660_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_BATT_BE_MAX1660_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_BATT_BE_MAX1660_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_BATT_BE_MAX1660_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_BATT_BE_MAX1660_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_BATT_BE_MAX1660_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_BATT_BE_MAX1660_COMM_STATS_UDC} );
    $msg .= sprintf( "---- EEPROM -------------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", _getI2CMonitorStatusCodeString($i2cStats{DEVICE_BATT_BE_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $i2cStats{DEVICE_BATT_BE_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $i2cStats{DEVICE_BATT_BE_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", _getI2CMonitorEventFlagsString($i2cStats{DEVICE_BATT_BE_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "        Description:                        %s\n", $i2cStats{DEVICE_BATT_BE_EEPROM_EVENT_PROP_DESCRIPTION} );
    $msg .= sprintf( "      Device Flags:                         %s\n", _getI2CMonitorDeviceFlagsString($i2cStats{DEVICE_BATT_BE_EEPROM_FLAG_VALUE}) );
    $msg .= sprintf( "        Incorrect Ack/Nack:                 %d\n", $i2cStats{DEVICE_BATT_BE_EEPROM_COMM_STATS_IANC} );
    $msg .= sprintf( "        Transmit Complete Timeout:          %d\n", $i2cStats{DEVICE_BATT_BE_EEPROM_COMM_STATS_TCTC} );
    $msg .= sprintf( "        Receive Complete Timeout:           %d\n", $i2cStats{DEVICE_BATT_BE_EEPROM_COMM_STATS_RCTC} );
    $msg .= sprintf( "        Bus Hang:                           %d\n", $i2cStats{DEVICE_BATT_BE_EEPROM_COMM_STATS_BHC} );
    $msg .= sprintf( "        Bytes Transmitted:                  %d\n", $i2cStats{DEVICE_BATT_BE_EEPROM_COMM_STATS_BTC} );
    $msg .= sprintf( "        Bytes Received:                     %d\n", $i2cStats{DEVICE_BATT_BE_EEPROM_COMM_STATS_BRC} );
    $msg .= sprintf( "        Unresponsive Device:                %d\n", $i2cStats{DEVICE_BATT_BE_EEPROM_COMM_STATS_UDC} );

    return $msg;
}

##############################################################################
# Name:     statsCacheDevices
#
# Desc:     Gets cache device statistics
#
# In:       ID - Optional ID to get cache device statistics for one device.
#           If not sent in retrieve statistics for all devices.
#
# Returns:  Returns an empty hash if errors occur else returns a
#           hash with the following information
##############################################################################
sub statsCacheDevices
{
    my ($self, $id) = @_;

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["statsCacheDevices"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet;

    if ($id == 0xFFFF)
    {
        $packet = assembleXiotechPacket(PI_STATS_CACHE_DEVICES_CMD,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);
    }
    else
    {
        my $data = pack("SS",
                        $id,
                        0);

        $packet = assembleXiotechPacket(PI_STATS_CACHE_DEVICE_CMD,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);
    }

    return $self->_handleSyncResponse($seq, $packet, \&_statsCacheDevices);
}

##############################################################################
# NAME:     statsCacheDevicesDisplay
#
# DESC:     Print the cache devices statistics
#
# INPUT:    Cache Devices Statistics Hash
##############################################################################
sub statsCacheDevicesDisplay
{
    my ($self, $id, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    if ($id == 0xFFFF)
    {
        $msg .= sprintf  "Cache Device Statistics (3 parts):\n";
        $msg .= sprintf  "\n";
        $msg .= sprintf  " VID   ERR_FLUSH_CNT  STAT     CACHE       DIRTY        IO    \n";
        $msg .= sprintf  "-----  -------------  ----  ----------  ----------  ----------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf "%5hu  ", $info{CACHEDEVS}[$i]{VC_VID};
            $msg .= sprintf "%13hu  ", $info{CACHEDEVS}[$i]{VC_ERR_FLUSH_CNT};
            $msg .= sprintf "0x%02x  ", $info{CACHEDEVS}[$i]{VC_STAT};
            $msg .= sprintf "%10hu  ", $info{CACHEDEVS}[$i]{VC_CACHE};
            $msg .= sprintf "%10hu  ", $info{CACHEDEVS}[$i]{VC_DIRTY};
            $msg .= sprintf "%10hu  ", $info{CACHEDEVS}[$i]{VC_IO};
            $msg .= sprintf  "\n";
        }

        $msg .= sprintf  "\n";

        $msg .= sprintf  " VID     WRT_CNT      FLUSHLBA        RDHITS         RDPART         RDMISS   \n";
        $msg .= sprintf  "-----  ----------  -------------  -------------  -------------  -------------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf "%5hu  ", $info{CACHEDEVS}[$i]{VC_VID};
            $msg .= sprintf "%10hu  ", $info{CACHEDEVS}[$i]{VC_WRT_CNT};
            $msg .= sprintf "%13s  ", $info{CACHEDEVS}[$i]{VC_FLUSHLBA};
            $msg .= sprintf "%13s  ", $info{CACHEDEVS}[$i]{VC_RDHITS};
            $msg .= sprintf "%13s  ", $info{CACHEDEVS}[$i]{VC_RDPART};
            $msg .= sprintf "%13s  ", $info{CACHEDEVS}[$i]{VC_RDMISS};
            $msg .= sprintf  "\n";
        }

        $msg .= sprintf  "\n";

        $msg .= sprintf  " VID       WRHITS         WRPART         WRMISS        WRTBYRES       WRTBYLEN       CAPACITY  \n";
        $msg .= sprintf  "-----  -------------  -------------  -------------  -------------  -------------  -------------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf "%5hu  ", $info{CACHEDEVS}[$i]{VC_VID};
            $msg .= sprintf "%13s  ", $info{CACHEDEVS}[$i]{VC_WRHITS};
            $msg .= sprintf "%13s  ", $info{CACHEDEVS}[$i]{VC_WRPART};
            $msg .= sprintf "%13s  ", $info{CACHEDEVS}[$i]{VC_WRMISS};
            $msg .= sprintf "%13s  ", $info{CACHEDEVS}[$i]{VC_WRTBYRES};
            $msg .= sprintf "%13s  ", $info{CACHEDEVS}[$i]{VC_WRTBYLEN};
            $msg .= sprintf "%13s  ", $info{CACHEDEVS}[$i]{VC_CAPACITY};
            $msg .= sprintf  "\n";
        }

        $msg .= sprintf  "\n";

        $msg .= sprintf  " VID       VTV       THEAD       TTAIL      FWD_WAIT    BWD_WAIT \n";
        $msg .= sprintf  "-----  ----------  ----------  ----------  ----------  ----------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf "%5hu  ", $info{CACHEDEVS}[$i]{VC_VID};
            $msg .= sprintf "%10lu  ", $info{CACHEDEVS}[$i]{VC_VTV};
            $msg .= sprintf "0x%8.8x  ", $info{CACHEDEVS}[$i]{VC_THEAD};
            $msg .= sprintf "0x%8.8x  ", $info{CACHEDEVS}[$i]{VC_TTAIL};
            $msg .= sprintf "0x%8.8x  ", $info{CACHEDEVS}[$i]{VC_FWD_WAIT};
            $msg .= sprintf "0x%8.8x  ", $info{CACHEDEVS}[$i]{VC_BWD_WAIT};
            $msg .= sprintf  "\n";
        }
    }
    else
    {
        $msg .= "Cache Device Statistics:\n";
        $msg .= sprintf "  VC_VID:              %hu\n", $info{VC_VID};
        $msg .= sprintf "  VC_ERR_FLUSH_CNT:    %hu\n", $info{VC_ERR_FLUSH_CNT};
        $msg .= sprintf "  VC_STAT:             0x%02x\n", $info{VC_STAT};
        $msg .= sprintf "  VC_CACHE:            %hu\n", $info{VC_CACHE};
        $msg .= sprintf "  VC_DIRTY:            %hu\n", $info{VC_DIRTY};
        $msg .= sprintf "  VC_IO:               %hu\n", $info{VC_IO};
        $msg .= sprintf "  VC_WRT_CNT:          %hu\n", $info{VC_WRT_CNT};
        $msg .= sprintf "  VC_FLUSHLBA:         %s\n", $info{VC_FLUSHLBA};
        $msg .= sprintf "  VC_RDHITS:           %s\n", $info{VC_RDHITS};
        $msg .= sprintf "  VC_RDPART:           %s\n", $info{VC_RDPART};
        $msg .= sprintf "  VC_RDMISS:           %s\n", $info{VC_RDMISS};
        $msg .= sprintf "  VC_WRHITS:           %s\n", $info{VC_WRHITS};
        $msg .= sprintf "  VC_WRPART:           %s\n", $info{VC_WRPART};
        $msg .= sprintf "  VC_WRMISS:           %s\n", $info{VC_WRMISS};
        $msg .= sprintf "  VC_WRTBYRES:         %s\n", $info{VC_WRTBYRES};
        $msg .= sprintf "  VC_WRTBYLEN:         %s\n", $info{VC_WRTBYLEN};
        $msg .= sprintf "  VC_CAPACITY:         %s\n", $info{VC_CAPACITY};
        $msg .= sprintf "  VC_VTV:              %lu\n", $info{VC_VTV};
        $msg .= sprintf "  VC_THEAD:            0x%x\n", $info{VC_THEAD};
        $msg .= sprintf "  VC_TTAIL:            0x%x\n", $info{VC_TTAIL};
        $msg .= sprintf "  VC_FWD_WAIT:         0x%x\n", $info{VC_FWD_WAIT};
        $msg .= sprintf "  VC_BWD_WAIT:         0x%x\n", $info{VC_BWD_WAIT};
    }

    $msg .= sprintf "\n";

    return $msg;

}

##############################################################################
# Name:     statsLoop
#
# Desc:     Gets loop statistics
#
# In:       TYPE    - Which statistics
#           CHANNEL - Which channel
#
# Returns:  Returns an empty hash if errors occur else returns a
#           hash with the following information
##############################################################################
sub statsLoop
{
    my ($self, $type, $option) = @_;

    # verify parameters
    my $args = [['i'],
                ['s'],
                ["statsLoop"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $commandCode = 0;

    if (uc($type) eq "BE")
    {
        $commandCode = PI_STATS_BACK_END_LOOP_CMD;
    }
    elsif (uc($type) eq "FE")
    {
        $commandCode = PI_STATS_FRONT_END_LOOP_CMD;
    }

	# Make code backward compatible by using a default option if
	# none is provided.
    if (!defined($option))
    {
        $option = 0;
    }


    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $option,
                    0, 0, 0);

    my $packet = assembleXiotechPacket($commandCode,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_statsLoop);
}

##############################################################################
# NAME:     statsLoopDisplay
#
# DESC:     Print the loop statistics
#
# INPUT:    TYPE - Which statistics
#           HASH - Loop Statistics Hash
##############################################################################
sub statsLoopDisplay
{
    my ($self, $type, %info) = @_;

    my $msg = "";

    logMsg("begin\n");

    if (uc($type) eq "BE")
    {
        $msg .= sprintf "Back End Loop Statistics:\n";
    }
    elsif (uc($type) eq "FE")
    {
        $msg .= sprintf "Front End Loop Statistics:\n";
    }

    $msg .= sprintf "Port count: $info{PORT_COUNT} \n\n";

    # Loop through the valid ports
    for (my $i = 0; $i < $info{PORT_COUNT}; $i++)
    {
        $msg .= sprintf "Port: %d\n", $info{$i}{PORT};
        $msg .= sprintf "-------\n";
        $msg .= sprintf "  STATUS:    0x%x\n", $info{$i}{STATUS_MRP};
        $msg .= sprintf "  LEN:       %lu\n", $info{$i}{LEN};
        $msg .= sprintf "  NUMHOSTS:  %hu\n", $info{$i}{NUMHOSTS};
        $msg .= sprintf "  LID:       %hu\n", $info{$i}{LID};
        $msg .= sprintf "  LIFCNT:    %lu\n", $info{$i}{LIFCNT};
        $msg .= sprintf "  LSSCNT:    %lu\n", $info{$i}{LSSCNT};
        $msg .= sprintf "  LSGCNT:    %lu\n", $info{$i}{LSGCNT};
        $msg .= sprintf "  PSPEC:     %lu\n", $info{$i}{PSPEC};
        $msg .= sprintf "  IVTQC:     %lu\n", $info{$i}{IVTQC};
        $msg .= sprintf "  IVCRC:     %lu\n", $info{$i}{IVCRC};
        $msg .= sprintf "  VENDID:    0x%04X\n", $info{$i}{VENDID};
        $msg .= sprintf "  MODEL:     0x%04X\n", $info{$i}{MODEL};
        $msg .= sprintf "  REVLEVEL:  %lu\n", $info{$i}{REVLEVEL};
        $msg .= sprintf "  RISCLEVEL: %lu\n", $info{$i}{RISCLEVEL};
        $msg .= sprintf "  FPMLEVEL:  %hu\n", $info{$i}{FPMLEVEL};
        $msg .= sprintf "  FPLEVEL:   %hu\n", $info{$i}{FPLEVEL};
        $msg .= sprintf "  ROMLEVEL:  %lu\n", $info{$i}{ROMLEVEL};
        $msg .= sprintf "  TYPE:      %s\n", $info{$i}{TYPE};
        $msg .= sprintf "  FWLEVEL:   %d.%d.%d\n", $info{$i}{FWMAJOR},
                             $info{$i}{FWMINOR}, $info{$i}{FWSUB};
        $msg .= sprintf "  FWATTRIB   0x%04X\n", $info{$i}{FWATTRIB};
        $msg .= sprintf "  DATARATE:  %lu\n", $info{$i}{DATARATE};
        $msg .= sprintf "  STATE:     %lu\n", $info{$i}{STATE};
        $msg .= sprintf "  NUMTARG:   %hu\n", $info{$i}{NUMTARG};
        $msg .= sprintf "  GPIOD reg: 0x%02X\n", $info{$i}{GPIOD};

        if (uc($type) eq "FE")
        {
            $msg .= sprintf  "\n";
            $msg .= sprintf  "  Targets\n";
            $msg .= sprintf  "  -------\n";
            for (my $j = 0; $j < $info{$i}{NUMTARG}; $j++)
            {
                $msg .= sprintf "  %7hu\n", $info{$i}{TGTS}[$j]{TID};
            }
        }
        $msg .= sprintf "\n";
    }

    return $msg;

}

##############################################################################
# Name:     statsPCI
#
# Desc:     Gets PCI statistics
#
# In:       TYPE - Which statistics
#
# Returns:  Returns an empty hash if errors occur else returns a
#           hash with the following information
##############################################################################
sub statsPCI
{
    my ($self, $type) = @_;

    # verify parameters
    my $args = [['i'],
                ['s'],
                ["statsPCI"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $commandCode = 0;

    if (uc($type) eq "BE")
    {
        $commandCode = PI_STATS_BACK_END_PCI_CMD;
    }
    elsif (uc($type) eq "FE")
    {
        $commandCode = PI_STATS_FRONT_END_PCI_CMD;
    }
    else
    {
        $commandCode = PI_STATS_PCI_CMD;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($commandCode,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_statsPCI);
}

##############################################################################
# NAME:     statsPCIDisplay
#
# DESC:     Print the PCI statistics
#
# INPUT:    TYPE - Which statistics
#           HASH - PCI Statistics Hash
##############################################################################
sub statsPCIDisplay
{
    my ($self, $type, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

        $msg .= sprintf  "PCI Statistics:\n";
        $msg .= sprintf  "\n";
        $msg .= sprintf  "TYPE  VRPOCOUNT  VRPICOUNT  VRPOTCOUNT  VRPITCOUNT\n";
        $msg .= sprintf  "----  ---------  ---------  ----------  ----------\n";

    if (uc($type) eq "ALL")
    {
        $msg .= sprintf  " FE   ";
        $msg .= sprintf "%9hu  ", $info{FE_VRPOCOUNT};
        $msg .= sprintf "%9hu  ", $info{FE_VRPICOUNT};
        $msg .= sprintf "%10lu  ", $info{FE_VRPOTCOUNT};
        $msg .= sprintf "%10lu", $info{FE_VRPITCOUNT};
        $msg .= sprintf  "\n";

        $msg .= sprintf  " BE   ";
        $msg .= sprintf "%9hu  ", $info{BE_VRPOCOUNT};
        $msg .= sprintf "%9hu  ", $info{BE_VRPICOUNT};
        $msg .= sprintf "%10lu  ", $info{BE_VRPOTCOUNT};
        $msg .= sprintf "%10lu", $info{BE_VRPITCOUNT};
        $msg .= sprintf  "\n";
    }
    else
    {
        if (uc($type) eq "BE")
        {
            $msg .= sprintf  " BE   ";
        }
        elsif (uc($type) eq "FE")
        {
            $msg .= sprintf  " FE   ";
        }

        $msg .= sprintf "%9hu  ", $info{VRPOCOUNT};
        $msg .= sprintf "%9hu  ", $info{VRPICOUNT};
        $msg .= sprintf "%10lu  ", $info{VRPOTCOUNT};
        $msg .= sprintf "%10lu", $info{VRPITCOUNT};
        $msg .= sprintf  "\n";
    }

    $msg .= sprintf "\n";

    return $msg;
}

##############################################################################
# Name:     statsProc
#
# Desc:     Gets proc statistics
#
# In:       TYPE - Which statistics
#
# Returns:  Returns an empty hash if errors occur else returns a
#           hash with the following information
##############################################################################
sub statsProc
{
    my ($self, $type) = @_;

    # verify parameters
    my $args = [['i'],
                ['s'],
                ["statsProc"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $commandCode = 0;

    if (uc($type) eq "BE")
    {
        $commandCode = PI_STATS_BACK_END_PROC_CMD;
    }
    elsif (uc($type) eq "FE")
    {
        $commandCode = PI_STATS_FRONT_END_PROC_CMD;
    }
    else
    {
        $commandCode = PI_STATS_PROC_CMD;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($commandCode,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_statsProc);
}

##############################################################################
# NAME:     statsProcDisplay
#
# DESC:     Print the proc statistics
#
# INPUT:    TYPE - Which statistics
#           HASH - Pric Statistics Hash
##############################################################################
sub statsProcDisplay
{
    my ($self, $type, %info) = @_;

    my $msg = "";

    logMsg("begin\n");

    if (uc($type) eq "ALL")
    {
        $msg .= sprintf "Processor Statistics:\n";
        $msg .= sprintf "\n";
        $msg .= sprintf "   FIELD    Back-End Processor  Front-End Processor\n";
        $msg .= sprintf "----------  ------------------  -------------------\n";
        $msg .= sprintf "II_VERS     %18hu  %19hu\n", $info{BE_II_VERS}, $info{FE_II_VERS};
        $msg .= sprintf "II_REV      %18hu  %19hu\n", $info{BE_II_REV}, $info{FE_II_REV};
        $msg .= sprintf "II_USER     %18s  %19s\n", $info{BE_II_USER}, $info{FE_II_USER};
        $msg .= sprintf "II_BDATE    %18s  %19s\n", $info{BE_II_BDATE}, $info{FE_II_BDATE};
        $msg .= sprintf "II_BTIME    %18s  %19s\n", $info{BE_II_BTIME}, $info{FE_II_BTIME};
        $msg .= sprintf "II_BCOUNT   %18hu  %19hu\n", $info{BE_II_BCOUNT}, $info{FE_II_BCOUNT};
        $msg .= sprintf "II_STATUS           0x%8.8x           0x%8.8x\n", $info{BE_II_STATUS}, $info{FE_II_STATUS};
        $msg .= sprintf "II_CHGCNT   %18hu  %19hu\n", $info{BE_II_CHGCNT}, $info{FE_II_CHGCNT};
        $msg .= sprintf "II_SCRUB    %18hu  %19hu\n", $info{BE_II_SCRUB}, $info{FE_II_SCRUB};
        $msg .= sprintf "II_GPRI     %18hu  %19hu\n", $info{BE_II_GPRI}, $info{FE_II_GPRI};
        $msg .= sprintf "II_UTZN     %18hu  %19hu\n", $info{BE_II_UTZN}, $info{FE_II_UTZN};
        $msg .= sprintf "II_TIME     %18lu  %19lu\n", $info{BE_II_TIME}, $info{FE_II_TIME};
        $msg .= sprintf "II_IRCUR    %18lu  %19lu\n", $info{BE_II_IRCUR}, $info{FE_II_IRCUR};
        $msg .= sprintf "II_IRMAX    %18lu  %19lu\n", $info{BE_II_IRMAX}, $info{FE_II_IRMAX};
        $msg .= sprintf "II_IRMIN    %18lu  %19lu\n", $info{BE_II_IRMIN}, $info{FE_II_IRMIN};
        $msg .= sprintf "II_IRWAIT   %18lu  %19lu\n", $info{BE_II_IRWAIT}, $info{FE_II_IRWAIT};
        $msg .= sprintf "II_SDCUR    %18lu  %19lu\n", $info{BE_II_SDCUR}, $info{FE_II_SDCUR};
        $msg .= sprintf "II_SDMAX    %18lu  %19lu\n", $info{BE_II_SDMAX}, $info{FE_II_SDMAX};
        $msg .= sprintf "II_SDMIN    %18lu  %19lu\n", $info{BE_II_SDMIN}, $info{FE_II_SDMIN};
        $msg .= sprintf "II_SDWAIT   %18lu  %19lu\n", $info{BE_II_SDWAIT}, $info{FE_II_SDWAIT};
        $msg .= sprintf "II_NCCUR    %18lu  %19lu\n", $info{BE_II_NCCUR}, $info{FE_II_NCCUR};
        $msg .= sprintf "II_NCMAX    %18lu  %19lu\n", $info{BE_II_NCMAX}, $info{FE_II_NCMAX};
        $msg .= sprintf "II_NCMIN    %18lu  %19lu\n", $info{BE_II_NCMIN}, $info{FE_II_NCMIN};
        $msg .= sprintf "II_NCWAIT   %18lu  %19lu\n", $info{BE_II_NCWAIT}, $info{FE_II_NCWAIT};
        $msg .= sprintf "II_RSCUR    %18lu  %19lu\n", $info{BE_II_RSCUR}, $info{FE_II_RSCUR};
        $msg .= sprintf "II_RSMAX    %18lu  %19lu\n", $info{BE_II_RSMAX}, $info{FE_II_RSMAX};
        $msg .= sprintf "II_RSMIN    %18lu  %19lu\n", $info{BE_II_RSMIN}, $info{FE_II_RSMIN};
        $msg .= sprintf "II_RSWAIT   %18lu  %19lu\n", $info{BE_II_RSWAIT}, $info{FE_II_RSWAIT};
        $msg .= sprintf "II_PCBCUR   %18lu  %19lu\n", $info{BE_II_PCBCUR}, $info{FE_II_PCBCUR};
        $msg .= sprintf "II_PCBMAX   %18lu  %19lu\n", $info{BE_II_PCBMAX}, $info{FE_II_PCBMAX};
        $msg .= sprintf "II_ILTCUR   %18lu  %19lu\n", $info{BE_II_ILTCUR}, $info{FE_II_ILTCUR};
        $msg .= sprintf "II_ILTMAX   %18lu  %19lu\n", $info{BE_II_ILTMAX}, $info{FE_II_ILTMAX};
        $msg .= sprintf "II_PRPCUR   %18lu  %19lu\n", $info{BE_II_PRPCUR}, $info{FE_II_PRPCUR};
        $msg .= sprintf "II_PRPMAX   %18lu  %19lu\n", $info{BE_II_PRPMAX}, $info{FE_II_PRPMAX};
        $msg .= sprintf "II_RRPCUR   %18lu  %19lu\n", $info{BE_II_RRPCUR}, $info{FE_II_RRPCUR};
        $msg .= sprintf "II_RRPMAX   %18lu  %19lu\n", $info{BE_II_RRPMAX}, $info{FE_II_RRPMAX};
        $msg .= sprintf "II_SCBCUR   %18lu  %19lu\n", $info{BE_II_SCBCUR}, $info{FE_II_SCBCUR};
        $msg .= sprintf "II_SCBMAX   %18lu  %19lu\n", $info{BE_II_SCBMAX}, $info{FE_II_SCBMAX};
        $msg .= sprintf "II_RPNCUR   %18lu  %19lu\n", $info{BE_II_RPNCUR}, $info{FE_II_RPNCUR};
        $msg .= sprintf "II_RPNMAX   %18lu  %19lu\n", $info{BE_II_RPNMAX}, $info{FE_II_RPNMAX};
        $msg .= sprintf "II_RRBCUR   %18lu  %19lu\n", $info{BE_II_RRBCUR}, $info{FE_II_RRBCUR};
        $msg .= sprintf "II_RRBMAX   %18lu  %19lu\n", $info{BE_II_RRBMAX}, $info{FE_II_RRBMAX};
        $msg .= sprintf "II_NVACUR   %18lu  %19lu\n", $info{BE_II_NVACUR}, $info{FE_II_NVACUR};
        $msg .= sprintf "II_NVAMAX   %18lu  %19lu\n", $info{BE_II_NVAMAX}, $info{FE_II_NVAMAX};
        $msg .= sprintf "II_NVAMIN   %18lu  %19lu\n", $info{BE_II_NVAMIN}, $info{FE_II_NVAMIN};
        $msg .= sprintf "II_NVAWAIT  %18lu  %19lu\n", $info{BE_II_NVAWAIT}, $info{FE_II_NVAWAIT};
    }
    else
    {
        if (uc($type) eq "BE")
        {
            $msg .= sprintf "Back End Proc Statistics:\n";
        }
        elsif (uc($type) eq "FE")
        {
            $msg .= sprintf "Front End Proc Statistics:\n";
        }

        $msg .= sprintf "  STATUS:               0x%x\n", $info{STATUS_MRP};
        $msg .= sprintf "  LEN:                  %lu\n", $info{LEN};
        $msg .= sprintf "  II_VERS:              %hu\n", $info{II_VERS};
        $msg .= sprintf "  II_REV:               %hu\n", $info{II_REV};
        $msg .= sprintf "  II_USER:              $info{II_USER}\n";
        $msg .= sprintf "  II_BDATE:             $info{II_BDATE}\n";
        $msg .= sprintf "  II_BTIME:             $info{II_BTIME}\n";
        $msg .= sprintf "  II_BCOUNT:            %hu\n", $info{II_BCOUNT};
        $msg .= sprintf "  II_STATUS:            0x%8.8x\n", $info{II_STATUS};
        $msg .= sprintf "  II_CHGCNT:            %hu\n", $info{II_CHGCNT};
        $msg .= sprintf "  II_SCRUB:             %hu\n", $info{II_SCRUB};
        $msg .= sprintf "  II_GPRI:              %hu\n", $info{II_GPRI};
        $msg .= sprintf "  II_UTZN:              %hu\n", $info{II_UTZN};
        $msg .= sprintf "  II_TIME:              %lu\n", $info{II_TIME};
        $msg .= sprintf "  II_IRCUR:             %lu\n", $info{II_IRCUR};
        $msg .= sprintf "  II_IRMAX:             %lu\n", $info{II_IRMAX};
        $msg .= sprintf "  II_IRMIN:             %lu\n", $info{II_IRMIN};
        $msg .= sprintf "  II_IRWAIT:            %lu\n", $info{II_IRWAIT};
        $msg .= sprintf "  II_SDCUR:             %lu\n", $info{II_SDCUR};
        $msg .= sprintf "  II_SDMAX:             %lu\n", $info{II_SDMAX};
        $msg .= sprintf "  II_SDMIN:             %lu\n", $info{II_SDMIN};
        $msg .= sprintf "  II_SDWAIT:            %lu\n", $info{II_SDWAIT};
        $msg .= sprintf "  II_NCCUR:             %lu\n", $info{II_NCCUR};
        $msg .= sprintf "  II_NCMAX:             %lu\n", $info{II_NCMAX};
        $msg .= sprintf "  II_NCMIN:             %lu\n", $info{II_NCMIN};
        $msg .= sprintf "  II_NCWAIT:            %lu\n", $info{II_NCWAIT};
        $msg .= sprintf "  II_RSCUR:             %lu\n", $info{II_RSCUR};
        $msg .= sprintf "  II_RSMAX:             %lu\n", $info{II_RSMAX};
        $msg .= sprintf "  II_RSMIN:             %lu\n", $info{II_RSMIN};
        $msg .= sprintf "  II_RSWAIT:            %lu\n", $info{II_RSWAIT};
        $msg .= sprintf "  II_PCBCUR:            %lu\n", $info{II_PCBCUR};
        $msg .= sprintf "  II_PCBMAX:            %lu\n", $info{II_PCBMAX};
        $msg .= sprintf "  II_ILTCUR:            %lu\n", $info{II_ILTCUR};
        $msg .= sprintf "  II_ILTMAX:            %lu\n", $info{II_ILTMAX};
        $msg .= sprintf "  II_PRPCUR:            %lu\n", $info{II_PRPCUR};
        $msg .= sprintf "  II_PRPMAX:            %lu\n", $info{II_PRPMAX};
        $msg .= sprintf "  II_RRPCUR:            %lu\n", $info{II_RRPCUR};
        $msg .= sprintf "  II_RRPMAX:            %lu\n", $info{II_RRPMAX};
        $msg .= sprintf "  II_SCBCUR:            %lu\n", $info{II_SCBCUR};
        $msg .= sprintf "  II_SCBMAX:            %lu\n", $info{II_SCBMAX};
        $msg .= sprintf "  II_RPNCUR:            %lu\n", $info{II_RPNCUR};
        $msg .= sprintf "  II_RPNMAX:            %lu\n", $info{II_RPNMAX};
        $msg .= sprintf "  II_RRBCUR:            %lu\n", $info{II_RRBCUR};
        $msg .= sprintf "  II_RRBMAX:            %lu\n", $info{II_RRBMAX};
        $msg .= sprintf "  II_NVACUR:            %lu\n", $info{II_NVACUR};
        $msg .= sprintf "  II_NVAMAX:            %lu\n", $info{II_NVAMAX};
        $msg .= sprintf "  II_NVAMIN:            %lu\n", $info{II_NVAMIN};
        $msg .= sprintf "  II_NVAWAIT:           %lu\n", $info{II_NVAWAIT};
    }

    $msg .= sprintf "\n";

    return $msg;

}

##############################################################################
# Name:     statsServer
#
# Desc:     Gets server statistics
#
# In:       ID - Server identifier
#
# Returns:  Returns an empty hash if errors occur else returns a
#           hash with the following information
##############################################################################
sub statsServer
{
    my ($self, $id) = @_;

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["statsServer"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $id,
                    0);

    my $packet = assembleXiotechPacket(PI_STATS_SERVER_CMD,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_statsServer);
}

##############################################################################
# Name:     statsServers
#
# Desc:     Gets server statistics for all valid servers
#
# In:       none
#
# Returns:  Returns an empty hash if errors occur else returns a
#           hash with the following information
##############################################################################
sub statsServers
{
    my ($self) = @_;

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket(PI_STATS_SERVERS_CMD,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_statsServers);
}

##############################################################################
# NAME:     statsServerDisplay
#
# DESC:     Print the server statistics
#
# INPUT:    Server Statistics Hash
##############################################################################
sub statsServerDisplay
{
    my ($self, %info) = @_;

    my $msg = "";

    logMsg("begin\n");

    $msg .= sprintf  "Server Statistics:\n";
    $msg .= sprintf "  STATUS:               0x%x\n", $info{STATUS_MRP};
    $msg .= sprintf "  LEN:                  %lu\n", $info{LEN};
    $msg .= sprintf  "  AG_CMDS:              $info{AG_CMDS}\n";
    $msg .= sprintf  "  AG_BYTES:             $info{AG_BYTES}\n";
    $msg .= sprintf  "  AG_WRITES:            $info{AG_WRITES}\n";
    $msg .= sprintf  "  AG_WBYTES:            $info{AG_WBYTES}\n";
    $msg .= sprintf  "  AG_READS:             $info{AG_READS}\n";
    $msg .= sprintf  "  AG_RBYTES:            $info{AG_RBYTES}\n";
    $msg .= sprintf  "  PER_CMDS:             $info{PER_CMDS}\n";
    $msg .= sprintf  "  PER_BYTES:            $info{PER_BYTES}\n";
    $msg .= sprintf  "  PER_WRITES:           $info{PER_WRITES}\n";
    $msg .= sprintf  "  PER_WBYTES:           $info{PER_WBYTES}\n";
    $msg .= sprintf  "  PER_READS:            $info{PER_READS}\n";
    $msg .= sprintf  "  PER_RBYTES:           $info{PER_RBYTES}\n";
    $msg .= sprintf "  FL_LIFCNT:            %lu\n", $info{FL_LIFCNT};
    $msg .= sprintf "  FL_LSSCNT:            %lu\n", $info{FL_LSSCNT};
    $msg .= sprintf "  FL_LSGCNT:            %lu\n", $info{FL_LSGCNT};
    $msg .= sprintf "  FL_PSPEC:             %lu\n", $info{FL_PSPEC};
    $msg .= sprintf "  FL_IVTQC:             %lu\n", $info{FL_IVTQC};
    $msg .= sprintf "  FL_IVCRC:             %lu\n", $info{FL_IVCRC};
    $msg .= sprintf "  QDEPTH:               %hu\n", $info{QDEPTH};
    $msg .= sprintf "\n";

    return $msg;

}

##############################################################################
# NAME:     statsServersDisplay
#
# DESC:     Print the server statistics
#
# INPUT:    Server Statistics Hash
##############################################################################
sub statsServersDisplay
{
    my ($self, $dspType, %info) = @_;

    my $msg = "";

    logMsg("begin\n");
    
    if (($dspType eq "ALL") || ($dspType eq "AGG"))
    {
        $msg .= sprintf  "Server Statistics - Aggregate ($info{COUNT} servers):\n";
        $msg .= sprintf  "\n";
        $msg .= sprintf  "SID     AG_CMDS    AG_BYTES   AG_WRITES   AG_WBYTES    AG_READS   AG_RBYTES\n";
        $msg .= sprintf  "---  ----------  ----------  ----------  ----------  ----------  ----------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf "%3u  ",  $info{STATSSERVERS}[$i]{SID};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{AG_CMDS};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{AG_BYTES};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{AG_WRITES};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{AG_WBYTES};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{AG_READS};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{AG_RBYTES};
        
            $msg .= sprintf  "\n";
        }
    }
    
    if (($dspType eq "ALL") || ($dspType eq "PER"))
    {
        $msg .= sprintf  "\n";
        $msg .= sprintf  "Server Statistics - Periodic ($info{COUNT} servers):\n";
        $msg .= sprintf  "\n";
        $msg .= sprintf  "SID    PER_CMDS   PER_BYTES  PER_WRITES  PER_WBYTES   PER_READS  PER_RBYTES\n";
        $msg .= sprintf  "---  ----------  ----------  ----------  ----------  ----------  ----------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf "%3u  ",  $info{STATSSERVERS}[$i]{SID};

            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{PER_CMDS};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{PER_BYTES};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{PER_WRITES};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{PER_WBYTES};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{PER_READS};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{PER_RBYTES};

            $msg .= sprintf  "\n";
        }
    }

    if (($dspType eq "ALL") || ($dspType eq "MISC"))
    {
        $msg .= sprintf  "\n";
        $msg .= sprintf  "Server Statistics - Miscellaneous ($info{COUNT} servers):\n";
        $msg .= sprintf  "\n";
        $msg .= sprintf  "SID   FL_LIFCNT   FL_LSSCNT   FL_LSGCNT    FL_PSPEC    FL_IVTQC    FL_IVCRC      QDEPTH \n";
        $msg .= sprintf  "---  ----------  ----------  ----------  ----------  ----------  ----------  ---------- \n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf "%3u  ",  $info{STATSSERVERS}[$i]{SID};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{FL_LIFCNT};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{FL_LSSCNT};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{FL_LSGCNT};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{FL_PSPEC};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{FL_IVTQC};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{FL_IVCRC};
            $msg .= sprintf "%10lu  ", $info{STATSSERVERS}[$i]{QDEPTH};
            $msg .= sprintf  "\n";
        }
    }
        
    $msg .= sprintf "\n";

    return $msg;
}

##############################################################################
# Name:     statsHAB
#
# Desc:     Gets HAB statistics
#
# In:       ID - HAB identifier
#
# Returns:  Returns an empty hash if errors occur else returns a
#           hash with the following information
##############################################################################
sub statsHAB
{
    my ($self, $id) = @_;

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0x000F],
                ["statsHAB"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $id,
                    0);

    my $packet = assembleXiotechPacket(PI_STATS_HAB_CMD,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_statsHAB);
}

##############################################################################
# NAME:     statsHABDisplay
#
# DESC:     Print the HAB statistics
#
# INPUT:    HAB Statistics Hash
##############################################################################
sub statsHABDisplay
{
    my ($self, %info) = @_;

    my $msg = "";

    logMsg("begin\n");

    $msg .= sprintf  "HAB Statistics:\n";
    $msg .= sprintf "  STATUS:               0x%x\n", $info{STATUS_MRP};
    $msg .= sprintf "  LEN:                  %lu\n", $info{LEN};
    $msg .= sprintf "  PER_CMDS:             $info{PER_CMDS}\n";
    $msg .= sprintf "  QDEPTH:               %hu\n", $info{QDEPTH};
    $msg .= sprintf "  AVG_REQ:              $info{AVG_REQ}\n";
    $msg .= sprintf "  WRT_REQ:              $info{WRT_REQ}\n";
    $msg .= sprintf "  RD_REQ:               $info{RD_REQ}\n";
    $msg .= sprintf "\n";

    return $msg;

}

##############################################################################
# Name:     statsVDisk
#
# Desc:     Gets virtual disk statistics
#
# In:       NONE
#
# Returns:  Returns an empty hash if errors occur else returns a
#           hash with the following information
##############################################################################
sub statsVDisk
{
    my ($self) = @_;

    # verify parameters
    my $args = [['i'],
                ["statsVDisk"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket(PI_STATS_VDISK_CMD,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_2);

    return $self->_handleSyncResponse($seq, $packet, \&_statsVDisk);
}

##############################################################################
# NAME:     statsVDiskDisplay
#
# DESC:     Print the virtual disk statistics
#
# INPUT:    Virtual Disk Statistics Hash
##############################################################################
sub statsVDiskDisplay
{
    my ($self, %info) = @_;

    my $msg = "";

    logMsg("begin\n");

    $msg .= sprintf  "Virtual Disk Statistics ($info{COUNT} disks):\n";
    $msg .= sprintf  "\n";
    $msg .= sprintf  "         REQUESTS /   AVG BLOCKS    TOTAL READ     TOTAL WRITE      AVG REQ/SEC   AVG BLOCKS/SEC\n";
    $msg .= sprintf  "  VID      SECOND      / SECOND      REQUESTS        REQUESTS       LAST HR       LAST HR\n";
    $msg .= sprintf  " -----   ----------   ----------   -------------   -------------    --------      --------------\n";

    for (my $i = 0; $i < $info{COUNT}; $i++)
    {
        $msg .= sprintf " %5hu  ",  $info{VDISKS}[$i]{VID};
        $msg .= sprintf " %10lu  ", $info{VDISKS}[$i]{RPS};
        $msg .= sprintf " %10lu  ", $info{VDISKS}[$i]{AVGSC};
        $msg .= sprintf " %13s  ",  $info{VDISKS}[$i]{RREQ};
        $msg .= sprintf " %13s ",  $info{VDISKS}[$i]{WREQ};
        $msg .= sprintf " %10lu  ", $info{VDISKS}[$i]{AVERAGEIO};
        $msg .= sprintf " %10lu  ", $info{VDISKS}[$i]{AVERAGESC};
        $msg .= sprintf  "\n";
    }

    $msg .= sprintf "\n";

    return $msg;
}

##############################################################################
# Name:     statsBufferBoard
#
# Desc:     Gets all the information and statistics for the buffer board
#
# In:       None
#
# Returns:  Returns an empty hash if errors occur else returns a
#           hash with the following information
##############################################################################
sub statsBufferBoard
{
    my ($self, $commandCode) = @_;

    # verify parameters
    my $args = [['i'],
                ['d',0,1],
                ["statsBufferBoard"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $data = pack("L", $commandCode);
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket(PI_STATS_BUFFER_BOARD_CMD,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_statsBufferBoardPacket);
}

##############################################################################
# Name: displayStatsBufferBoard
#
# Desc: Print the NVRAM board statistics information
#
# In:   NVRAM Board Statistics Hash
#
##############################################################################
sub displayStatsBufferBoard
{
    my ($self, %statsBufferBoard) = @_;
    my $msg = "";

    logMsg( "displayStatsBufferBoard...begin\n" );

    $msg .= sprintf( "Buffer Board Information\n" );
    $msg .= sprintf( "  MRP Header\n" );
    $msg .= sprintf( "    Status:        0x%x\n", $statsBufferBoard{MR_HDR_RSP_STATUS} );
    $msg .= sprintf( "    Length:        0x%x\n", $statsBufferBoard{MR_HDR_RSP_LENGTH} );
    $msg .= sprintf( "  Buffer Board Information\n" );
    $msg .= sprintf( "    Status:        0x%x\n", $statsBufferBoard{BUFFER_BOARD_STATUS} );
    $msg .= sprintf( "    Revision\n" );
    $msg .= sprintf( "      Major:       0x%x\n", $statsBufferBoard{BUFFER_BOARD_REVISION_MAJOR} );
    $msg .= sprintf( "      Minor:       0x%x\n", $statsBufferBoard{BUFFER_BOARD_REVISION_MINOR} );
    $msg .= sprintf( "    Size:          0x%x\n", $statsBufferBoard{BUFFER_BOARD_MEMORY_SIZE} );
    $msg .= sprintf( "    Error Count:   0x%x\n", $statsBufferBoard{BUFFER_BOARD_MEMORY_ERROR_COUNT} );
    $msg .= sprintf( "    Battery Count: 0x%x\n", $statsBufferBoard{BUFFER_BOARD_BATTERY_COUNT} );
    $msg .= sprintf( "    Battery 1\n" );
    $msg .= sprintf( "      Status:      0x%x\n", $statsBufferBoard{BUFFER_BOARD_BATTERY_1_STATUS} );
    $msg .= sprintf( "      Voltage:     0x%x (%d mV)\n",
        $statsBufferBoard{BUFFER_BOARD_BATTERY_1_VOLTAGE},
        $statsBufferBoard{BUFFER_BOARD_BATTERY_1_VOLTAGE} );
    $msg .= sprintf( "      Charge:      0x%x (%d tenths of percent)\n",
        $statsBufferBoard{BUFFER_BOARD_BATTERY_1_CHARGE_PERCENT},
        $statsBufferBoard{BUFFER_BOARD_BATTERY_1_CHARGE_PERCENT} );

    $msg .= sprintf( "    Battery 2\n" );
    $msg .= sprintf( "      Status:      0x%x\n", $statsBufferBoard{BUFFER_BOARD_BATTERY_2_STATUS} );
    $msg .= sprintf( "      Voltage:     0x%x (%d mV)\n",
        $statsBufferBoard{BUFFER_BOARD_BATTERY_2_VOLTAGE},
        $statsBufferBoard{BUFFER_BOARD_BATTERY_2_VOLTAGE} );
    $msg .= sprintf( "      Charge:      0x%x (%d tenths of percent)\n",
        $statsBufferBoard{BUFFER_BOARD_BATTERY_2_CHARGE_PERCENT},
        $statsBufferBoard{BUFFER_BOARD_BATTERY_2_CHARGE_PERCENT} );


    $msg .= sprintf( "    Battery 3\n" );
    $msg .= sprintf( "      Status:      0x%x\n", $statsBufferBoard{BUFFER_BOARD_BATTERY_3_STATUS} );
    $msg .= sprintf( "      Voltage:     0x%x (%d mV)\n",
        $statsBufferBoard{BUFFER_BOARD_BATTERY_3_VOLTAGE},
        $statsBufferBoard{BUFFER_BOARD_BATTERY_3_VOLTAGE} );
    $msg .= sprintf( "      Charge:      0x%x (%d tenths of percent)\n",
        $statsBufferBoard{BUFFER_BOARD_BATTERY_3_CHARGE_PERCENT},
        $statsBufferBoard{BUFFER_BOARD_BATTERY_3_CHARGE_PERCENT} );


    $msg .= sprintf( "    Battery 4\n" );
    $msg .= sprintf( "      Status:      0x%x\n", $statsBufferBoard{BUFFER_BOARD_BATTERY_4_STATUS} );
    $msg .= sprintf( "      Voltage:     0x%x (%d mV)\n",
        $statsBufferBoard{BUFFER_BOARD_BATTERY_4_VOLTAGE},
        $statsBufferBoard{BUFFER_BOARD_BATTERY_4_VOLTAGE} );
    $msg .= sprintf( "      Charge:      0x%x (%d tenths of percent)\n",
        $statsBufferBoard{BUFFER_BOARD_BATTERY_4_CHARGE_PERCENT},
        $statsBufferBoard{BUFFER_BOARD_BATTERY_4_CHARGE_PERCENT} );

    return $msg;
}

##############################################################################
#
#   All commands that start with a _ should not be used and are considered
#   private.
#
##############################################################################


##############################################################################
# Name:  _envStatsExtPacket
#
# Desc: Parses the system information packet and places the information in a
#       hash
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: Empty hash on error, else a hash with the following elements:
#
##############################################################################
sub _envStatsExtPacket
{
    my ($self, $seq, $recvPacket) = @_;

    my %envStats;

    if (!(defined($recvPacket)))
    {
        $recvPacket = $self->_receivePacketSync($seq);
    }

################################################################################
##  /* Things pertaining to condition reporting of the object */
##    I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##
##  /* Things pertaining to condition of the object */
##    CCB_STATUS ccbStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##      NVRAM_BATTERY_STATUS nvramBatteryStatus;
##      EEPROM_STATUS ccbBoardEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
##      EEPROM_STATUS ccbMemoryModuleEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
##    PROC_BOARD_STATUS procBoardStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##      POWER_SUPPLY_VOLTAGES_STATUS powerSupplyVoltagesStatus;
##      PROC_BOARD_PROCESSOR_STATUS frontEndProcessorStatus;
##      PROC_BOARD_PROCESSOR_STATUS backEndProcessorStatus;
##      EEPROM_STATUS chassisEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
##      EEPROM_STATUS procBoardEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
##    POWER_SUPPLY_STATUS frontEndPowerSupply;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##      UINT8 powerSupplyConditionValue;
##      UINT8 coolingFanConditionValue;
##      EEPROM_STATUS eepromStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
##    POWER_SUPPLY_STATUS backEndPowerSupply;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##      UINT8 powerSupplyConditionValue;
##      UINT8 coolingFanConditionValue;
##      EEPROM_STATUS eepromStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
##    BUFFER_BOARD_STATUS frontEndBufferBoardStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##      TEMPERATURE_STATUS temperatureStatus;
##      BATTERY_STATUS batteryStatus;
##      FUEL_GAUGE_STATUS fuelGaugeStatus;
##      MAIN_REGULATOR_STATUS mainRegulatorStatus;
##      CHARGER_STATUS chargerStatus;
##      EEPROM_STATUS eepromStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
##    BUFFER_BOARD_STATUS backEndBufferBoardStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##      TEMPERATURE_STATUS temperatureStatus;
##      BATTERY_STATUS batteryStatus;
##      FUEL_GAUGE_STATUS fuelGaugeStatus;
##      MAIN_REGULATOR_STATUS mainRegulatorStatus;
##      CHARGER_STATUS chargerStatus;
##      EEPROM_STATUS eepromStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
################################################################################
    my $I2C_MONITOR_STATUS_CODE                     = "L";
    my $I2C_MONITOR_EVENT_STATISTICS                = "LLL";
    my $I2C_MONITOR_EVENT_PROPERTIES_FLAG           = "C";
    my $I2C_MONITOR_EVENT_PROPERTIES_STRING         = "a40";

    my $I2C_MONITOR_EVENT_PROPERTIES                = $I2C_MONITOR_STATUS_CODE .
                                                      $I2C_MONITOR_EVENT_STATISTICS .
                                                      $I2C_MONITOR_EVENT_PROPERTIES_FLAG.
                                                      $I2C_MONITOR_EVENT_PROPERTIES_STRING;

    my $XCI_DATA                                    = "a64";

    my $EEPROM_STATUS                               = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      "C" .             # eepromCondition
                                                      $XCI_DATA;

    my $NVRAM_BATTERY_STATUS                        = $I2C_MONITOR_EVENT_PROPERTIES .
                                                       "C";             # nvramBatteryCondition

    my $CCB_STATUS                                  = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $NVRAM_BATTERY_STATUS .
                                                      $EEPROM_STATUS .  # ccbBoardEEPROMStatus
                                                      $EEPROM_STATUS;   # ccbMemoryModuleEEPROMStatus

    my $MILLIVOLTS                                  = "S";
    my $DEGREES_CELSIUS                             = "c";

    my $TEMPERATURE_STATUS                          = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $DEGREES_CELSIUS .
                                                      $DEGREES_CELSIUS .
                                                      $DEGREES_CELSIUS .
                                                      "C";              # conditionValue

    my $VOLTAGE_INPUT_READING                       = $MILLIVOLTS .
                                                      $MILLIVOLTS .
                                                      $MILLIVOLTS .
                                                      "C";              # limitMonitorValue

    my $POWER_SUPPLY_STATUS                         = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      "C" .             # powerSupplyCondition.acFailedCounter
                                                      "C" .             # powerSupplyCondition.dcFailedCounter
                                                      "C" .             # powerSupplyCondition.value
                                                      "C" .             # coolingFanConditionValue
                                                      $EEPROM_STATUS .  # assemblyEEPROMStatus
                                                      $EEPROM_STATUS;   # interfaceEEPROMStatus

    my $POWER_SUPPLY_VOLTAGES_STATUS                = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $VOLTAGE_INPUT_READING .
                                                      $VOLTAGE_INPUT_READING .
                                                      $VOLTAGE_INPUT_READING .
                                                      $VOLTAGE_INPUT_READING;

    my $PROC_BOARD_PROCESSOR_STATUS                 = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $TEMPERATURE_STATUS .
                                                      $VOLTAGE_INPUT_READING .
                                                      "C";              # processorResetConditionValue

    my $PROC_BOARD_STATUS                           = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $POWER_SUPPLY_VOLTAGES_STATUS .
                                                      $PROC_BOARD_PROCESSOR_STATUS .
                                                      $PROC_BOARD_PROCESSOR_STATUS .
                                                      $EEPROM_STATUS .  # chassisEEPROMStatus
                                                      $EEPROM_STATUS;   # procBoardEEPROMStatus

    my $BATTERY_STATUS                              = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $VOLTAGE_INPUT_READING .
                                                      "C";              # batteryCondition

    my $FUEL_GAUGE_STATUS                           = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      "L" .             # currentFlowRate
                                                      $VOLTAGE_INPUT_READING .
                                                      "C" .             # fuelGaugeCondition
                                                      "C";              # currentFlowCondition

    my $MAIN_REGULATOR_STATUS                       = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $VOLTAGE_INPUT_READING .
                                                      $VOLTAGE_INPUT_READING .
                                                      $VOLTAGE_INPUT_READING .
                                                      "C";              # mainRegulatorCondition

    my $CHARGER_STATUS                              = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      "C";              # chargerCondition

    my $BATTERY_SDIMM_STATUS                        = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $TEMPERATURE_STATUS .
                                                      $BATTERY_STATUS .
                                                      $FUEL_GAUGE_STATUS .
                                                      $MAIN_REGULATOR_STATUS .
                                                      $CHARGER_STATUS .
                                                      $EEPROM_STATUS;

    my $I2C_MONITOR_STATUS                          = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $CCB_STATUS .
                                                      $PROC_BOARD_STATUS .
                                                      $POWER_SUPPLY_STATUS .
                                                      $POWER_SUPPLY_STATUS .
                                                      $BATTERY_SDIMM_STATUS .
                                                      $BATTERY_SDIMM_STATUS;
    if (defined($recvPacket))
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $envStats{STATUS} = $parts{STATUS};
        $envStats{ERROR_CODE} = $parts{ERROR_CODE};
##        print "Data Length: $parts{DATA_LENGTH}\n";

        if (commandCode($recvPacket) == PI_STATS_ENVIRONMENTAL_CMD)
        {
##            print "I2C_MONITOR_STATUS: $I2C_MONITOR_STATUS\n";

            (
##    I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{MONITOR_EVENT_PROP_STATUS},
            $envStats{MONITOR_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{MONITOR_EVENT_PROP_STATS_WARNING},
            $envStats{MONITOR_EVENT_PROP_STATS_ERROR},
            $envStats{MONITOR_EVENT_PROP_FLAG_VALUE},
            $envStats{MONITOR_EVENT_PROP_DESCRIPTION},
##    CCB_STATUS ccbStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{CCB_EVENT_PROP_STATUS},
            $envStats{CCB_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{CCB_EVENT_PROP_STATS_WARNING},
            $envStats{CCB_EVENT_PROP_STATS_ERROR},
            $envStats{CCB_EVENT_PROP_FLAG_VALUE},
            $envStats{CCB_EVENT_PROP_DESCRIPTION},
##      NVRAM_BATTERY_STATUS nvramBatteryStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{NVRAM_BATTERY_EVENT_PROP_STATUS},
            $envStats{NVRAM_BATTERY_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{NVRAM_BATTERY_EVENT_PROP_STATS_WARNING},
            $envStats{NVRAM_BATTERY_EVENT_PROP_STATS_ERROR},
            $envStats{NVRAM_BATTERY_EVENT_PROP_FLAG_VALUE},
            $envStats{NVRAM_BATTERY_EVENT_PROP_DESCRIPTION},
##        UINT8 nvramBatteryCondition;
            $envStats{NVRAM_BATTERY_CONDITION},
##      EEPROM_STATUS ccbBoardEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{CCB_BOARD_EEPROM_EVENT_PROP_STATUS},
            $envStats{CCB_BOARD_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{CCB_BOARD_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{CCB_BOARD_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{CCB_BOARD_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{CCB_BOARD_EEPROM_EVENT_PROP_DESCRIPTION},
##        UINT8 eepromCondition;
            $envStats{CCB_BOARD_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{CCB_BOARD_EEPROM_XCI_DATA},
##      EEPROM_STATUS ccbMemoryModuleEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_STATUS},
            $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_DESCRIPTION},
##        UINT8 eepromCondition;
            $envStats{CCB_MEMORY_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{CCB_MEMORY_EEPROM_XCI_DATA},
##    PROC_BOARD_STATUS procBoardStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{PROC_BOARD_STATUS_EVENT_PROP_STATUS},
            $envStats{PROC_BOARD_STATUS_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_BOARD_STATUS_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_BOARD_STATUS_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_BOARD_STATUS_EVENT_PROP_FLAG_VALUE},
            $envStats{PROC_BOARD_STATUS_EVENT_PROP_DESCRIPTION},
##      POWER_SUPPLY_VOLTAGES powerSupplyVoltages;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_STATUS},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_FLAG_VALUE},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_DESCRIPTION},
##      POWER_SUPPLY_VOLTAGES_STATUS powerSupplyVoltagesStatus;
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_MAX_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_CURRENT_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_MIN_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_LMV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_MAX_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_CURRENT_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_MIN_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_LMV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_MAX_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_CURRENT_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_MIN_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_LMV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_MAX_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_CURRENT_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_MIN_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_LMV},
##      PROC_BOARD_PROCESSOR_STATUS frontEndProcessorStatus;
            $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_STATUS},
            $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_FLAG_VALUE},
            $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_DESCRIPTION},

            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_STATUS},
            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_FLAG_VALUE},
            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_DESCRIPTION},

            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_MAX_CELSIUS},
            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_CURRENT_CELSIUS},
            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_MIN_CELSIUS},
            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_CONDITION},

            $envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_MAX_MV},
            $envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_CURRENT_MV},
            $envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_MIN_MV},
            $envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_LMV},

            $envStats{PROC_BOARD_STATUS_FE_PROC_PRCV},
##      PROC_BOARD_PROCESSOR_STATUS backEndProcessorStatus;
            $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_STATUS},
            $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_FLAG_VALUE},
            $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_DESCRIPTION},

            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_STATUS},
            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_FLAG_VALUE},
            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_DESCRIPTION},

            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_MAX_CELSIUS},
            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_CURRENT_CELSIUS},
            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_MIN_CELSIUS},
            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_CONDITION},

            $envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_MAX_MV},
            $envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_CURRENT_MV},
            $envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_MIN_MV},
            $envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_LMV},

            $envStats{PROC_BOARD_STATUS_BE_PROC_PRCV},
##      EEPROM_STATUS chassisEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_STATUS},
            $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_DESCRIPTION},
##        UINT8 eepromCondition;
            $envStats{PROC_CHASSIS_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{PROC_CHASSIS_EEPROM_XCI_DATA},
##      EEPROM_STATUS procBoardEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{PROC_BOARD_EEPROM_EVENT_PROP_STATUS},
            $envStats{PROC_BOARD_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_BOARD_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_BOARD_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_BOARD_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{PROC_BOARD_EEPROM_EVENT_PROP_DESCRIPTION},
##        UINT8 eepromCondition;
            $envStats{PROC_BOARD_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{PROC_BOARD_EEPROM_XCI_DATA},
##    POWER_SUPPLY_STATUS frontEndPowerSupply;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_STATUS},
            $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_WARNING},
            $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_ERROR},
            $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_FLAG_VALUE},
            $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_DESCRIPTION},
##      UINT8 powerSupplyCondition;
            $envStats{FE_POWER_SUPPLY_STATUS_PSCACFSC},
            $envStats{FE_POWER_SUPPLY_STATUS_PSCDCFSC},
            $envStats{FE_POWER_SUPPLY_STATUS_PSCV},
##      UINT8 coolingFanConditionValue;
            $envStats{FE_POWER_SUPPLY_STATUS_CFCV},
##      EEPROM_STATUS assemblyEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATUS},
            $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_DESCRIPTION},
##        UINT8 eepromCondition;
            $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_XCI_DATA},
##      EEPROM_STATUS interfaceEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATUS},
            $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_DESCRIPTION},
##        UINT8 eepromCondition;
            $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_XCI_DATA},
##    POWER_SUPPLY_STATUS backEndPowerSupply;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_STATUS},
            $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_WARNING},
            $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_ERROR},
            $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_FLAG_VALUE},
            $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_DESCRIPTION},
##      UINT8 powerSupplyCondition;
            $envStats{BE_POWER_SUPPLY_STATUS_PSCACFSC},
            $envStats{BE_POWER_SUPPLY_STATUS_PSCDCFSC},
            $envStats{BE_POWER_SUPPLY_STATUS_PSCV},
##      UINT8 coolingFanConditionValue;
            $envStats{BE_POWER_SUPPLY_STATUS_CFCV},
##      EEPROM_STATUS assemblyEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATUS},
            $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_DESCRIPTION},
##        UINT8 eepromCondition;
            $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_XCI_DATA},
##      EEPROM_STATUS interfaceEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATUS},
            $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_DESCRIPTION},
##        UINT8 eepromCondition;
            $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_XCI_DATA},
##    BUFFER_BOARD_STATUS frontEndBufferBoardStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_STATUS},
            $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_WARNING},
            $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_ERROR},
            $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_FLAG_VALUE},
            $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_DESCRIPTION},
##      TEMPERATURE_STATUS temperatureStatus;
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATUS},
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_WARNING},
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_ERROR},
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_FLAG_VALUE},
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_DESCRIPTION},

            $envStats{FE_BATT_SDIMM_STATUS_TEMP_MAX_CELSIUS},
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_CURRENT_CELSIUS},
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_MIN_CELSIUS},
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_CONDITION},
##      BATTERY_STATUS batteryStatus;
            $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATUS},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_WARNING},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_ERROR},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_FLAG_VALUE},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_DESCRIPTION},

            $envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MAX_MV},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_CURRENT_MV},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MIN_MV},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_LMV},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_CONDITION},
##      FUEL_GAUGE_STATUS fuelGaugeStatus;
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATUS},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_WARNING},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_ERROR},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_FLAG_VALUE},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_DESCRIPTION},

            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_CFR},

            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MAX_MV},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_CURRENT_MV},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MIN_MV},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_LMV},

            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_FG_CONDITION},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_CF_CONDITION},
##      MAIN_REGULATOR_STATUS mainRegulatorStatus;
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATUS},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_WARNING},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_ERROR},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_FLAG_VALUE},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_DESCRIPTION},

            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MAX_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_CURRENT_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MIN_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_LMV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MAX_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_CURRENT_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MIN_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_LMV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MAX_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_CURRENT_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MIN_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_LMV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_CONDITION},
##      CHARGER_STATUS chargerStatus;
            $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATUS},
            $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_WARNING},
            $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_ERROR},
            $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_FLAG_VALUE},
            $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_DESCRIPTION},

            $envStats{FE_BATT_SDIMM_STATUS_CHARGER_CONDITION},
##      EEPROM_STATUS eepromStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_STATUS},
            $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_DESCRIPTION},
##        UINT8 eepromCondition;
            $envStats{FE_BATT_SDIMM_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{FE_BATT_SDIMM_EEPROM_XCI_DATA},
##    BUFFER_BOARD_STATUS backEndBufferBoardStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_STATUS},
            $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_WARNING},
            $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_ERROR},
            $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_FLAG_VALUE},
            $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_DESCRIPTION},
##      TEMPERATURE_STATUS temperatureStatus;
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATUS},
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_WARNING},
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_ERROR},
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_FLAG_VALUE},
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_DESCRIPTION},

            $envStats{BE_BATT_SDIMM_STATUS_TEMP_MAX_CELSIUS},
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_CURRENT_CELSIUS},
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_MIN_CELSIUS},
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_CONDITION},
##      BATTERY_STATUS batteryStatus;
            $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATUS},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_WARNING},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_ERROR},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_FLAG_VALUE},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_DESCRIPTION},

            $envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MAX_MV},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_CURRENT_MV},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MIN_MV},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_LMV},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_CONDITION},
##      FUEL_GAUGE_STATUS fuelGaugeStatus;
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATUS},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_WARNING},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_ERROR},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_FLAG_VALUE},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_DESCRIPTION},

            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_CFR},

            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MAX_MV},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_CURRENT_MV},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MIN_MV},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_LMV},

            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_FG_CONDITION},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_CF_CONDITION},
##      MAIN_REGULATOR_STATUS mainRegulatorStatus;
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATUS},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_WARNING},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_ERROR},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_FLAG_VALUE},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_DESCRIPTION},

            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MAX_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_CURRENT_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MIN_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_LMV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MAX_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_CURRENT_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MIN_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_LMV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MAX_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_CURRENT_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MIN_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_LMV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_CONDITION},
##      CHARGER_STATUS chargerStatus;
            $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATUS},
            $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_WARNING},
            $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_ERROR},
            $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_FLAG_VALUE},
            $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_DESCRIPTION},

            $envStats{BE_BATT_SDIMM_STATUS_CHARGER_CONDITION},
##      EEPROM_STATUS eepromStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_STATUS},
            $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_DESCRIPTION},
##        UINT8 eepromCondition;
            $envStats{BE_BATT_SDIMM_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{BE_BATT_SDIMM_EEPROM_XCI_DATA}) =
                unpack($I2C_MONITOR_STATUS, $parts{DATA});
        }
        else
        {
            $self->_handleError($recvPacket);
            logMsg("Unexpected packet: We expected a system info. packet\n");
        }
    }

    return %envStats;
}

##############################################################################
# Name:  _i2cStatsExtPacket
#
# Desc: Parses the system information packet and places the information in a
#       hash
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: Empty hash on error, else a hash with the following elements:
#
##############################################################################
sub _i2cStatsExtPacket
{
    my ($self, $seq, $recvPacket) = @_;

    my %envStats;

    if (!(defined($recvPacket)))
    {
        $recvPacket = $self->_receivePacketSync($seq);
    }

################################################################################
##  /* Things pertaining to condition reporting of the object */
##    I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##
##  /* Things pertaining to condition of the object */
##    I2C_MONITOR_CCB_DEVICES ccbDevices;
##      I2C_MONITOR_DEVICE_PROPERTIES processor;
##      I2C_MONITOR_DEVICE_PROPERTIES boardEEPROM;
##      I2C_MONITOR_DEVICE_PROPERTIES memoryModuleEEPROM;
##    I2C_MONITOR_PROC_BOARD_DEVICES procBoardDevices;
##      I2C_MONITOR_DEVICE_PROPERTIES lm80_lm87;
##      I2C_MONITOR_DEVICE_PROPERTIES lm75_lm92;
##      I2C_MONITOR_DEVICE_PROPERTIES resetControlPCF8574;
##      I2C_MONITOR_DEVICE_PROPERTIES powerSupplyPCF8574;
##      I2C_MONITOR_DEVICE_PROPERTIES pca9548;
##      I2C_MONITOR_DEVICE_PROPERTIES chassisEEPROM;
##      I2C_MONITOR_DEVICE_PROPERTIES procBoardEEPROM;
##    I2C_MONITOR_POWER_SUPPLY_DEVICES frontEndPowerSupplyDevices;
##      I2C_MONITOR_DEVICE_PROPERTIES eeprom;
##    I2C_MONITOR_POWER_SUPPLY_DEVICES backEndPowerSupplyDevices;
##      I2C_MONITOR_DEVICE_PROPERTIES eeprom;
##    I2C_MONITOR_BUFFER_BOARD_DEVICES frontEndBufferBoardDevices;
##      I2C_MONITOR_DEVICE_PROPERTIES lm80_lm87;
##      I2C_MONITOR_DEVICE_PROPERTIES pcf8574;
##      I2C_MONITOR_DEVICE_PROPERTIES max1660;
##      I2C_MONITOR_DEVICE_PROPERTIES eeprom;
##    I2C_MONITOR_BUFFER_BOARD_DEVICES backEndBufferBoardDevices;
##      I2C_MONITOR_DEVICE_PROPERTIES lm80_lm87;
##      I2C_MONITOR_DEVICE_PROPERTIES pcf8574;
##      I2C_MONITOR_DEVICE_PROPERTIES max1660;
##      I2C_MONITOR_DEVICE_PROPERTIES eeprom;
################################################################################

    my $I2C_MONITOR_STATUS_CODE                     = "L";
    my $I2C_MONITOR_EVENT_STATISTICS                = "LLL";
    my $I2C_MONITOR_EVENT_PROPERTIES_FLAG           = "C";
    my $I2C_MONITOR_EVENT_PROPERTIES_STRING         = "a40";

    my $I2C_MONITOR_EVENT_PROPERTIES                = $I2C_MONITOR_STATUS_CODE .
                                                      $I2C_MONITOR_EVENT_STATISTICS .
                                                      $I2C_MONITOR_EVENT_PROPERTIES_FLAG.
                                                      $I2C_MONITOR_EVENT_PROPERTIES_STRING;

    my $I2C_MONITOR_DEVICE_FLAG                     = "C";

    my $I2C_MONITOR_COMMUNICATION_STATISTICS        = "LLLLLLL";

    my $I2C_MONITOR_DEVICE_PROPERTIES               = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $I2C_MONITOR_DEVICE_FLAG .
                                                      $I2C_MONITOR_COMMUNICATION_STATISTICS;

    my $I2C_MONITOR_CCB_DEVICES                     = $I2C_MONITOR_DEVICE_PROPERTIES .
                                                      $I2C_MONITOR_DEVICE_PROPERTIES .
                                                      $I2C_MONITOR_DEVICE_PROPERTIES;

    my $I2C_MONITOR_PROC_BOARD_DEVICES              = $I2C_MONITOR_DEVICE_PROPERTIES .
                                                      $I2C_MONITOR_DEVICE_PROPERTIES .
                                                      $I2C_MONITOR_DEVICE_PROPERTIES .
                                                      $I2C_MONITOR_DEVICE_PROPERTIES .
                                                      $I2C_MONITOR_DEVICE_PROPERTIES .
                                                      $I2C_MONITOR_DEVICE_PROPERTIES .
                                                      $I2C_MONITOR_DEVICE_PROPERTIES;

    my $I2C_MONITOR_POWER_SUPPLY_DEVICES            = $I2C_MONITOR_DEVICE_PROPERTIES .
                                                      $I2C_MONITOR_DEVICE_PROPERTIES;

    my $I2C_MONITOR_BATTERY_SDIMM_DEVICES           = $I2C_MONITOR_DEVICE_PROPERTIES .
                                                      $I2C_MONITOR_DEVICE_PROPERTIES .
                                                      $I2C_MONITOR_DEVICE_PROPERTIES .
                                                      $I2C_MONITOR_DEVICE_PROPERTIES;

    my $I2C_HW_DATA                                 = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $I2C_MONITOR_CCB_DEVICES .
                                                      $I2C_MONITOR_PROC_BOARD_DEVICES .
                                                      $I2C_MONITOR_POWER_SUPPLY_DEVICES .
                                                      $I2C_MONITOR_POWER_SUPPLY_DEVICES .
                                                      $I2C_MONITOR_BATTERY_SDIMM_DEVICES .
                                                      $I2C_MONITOR_BATTERY_SDIMM_DEVICES;

    if (defined($recvPacket))
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $envStats{STATUS} = $parts{STATUS};
        $envStats{ERROR_CODE} = $parts{ERROR_CODE};
##        print "Data Length: $parts{DATA_LENGTH}\n";

        if (commandCode($recvPacket) == PI_STATS_I2C_CMD)
        {
##            print "I2C_HW_DATA: $I2C_HW_DATA\n";

            (
##    I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{EVENT_PROP_STATUS},
            $envStats{EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{EVENT_PROP_STATS_WARNING},
            $envStats{EVENT_PROP_STATS_ERROR},
            $envStats{EVENT_PROP_FLAG_VALUE},
            $envStats{EVENT_PROP_DESCRIPTION},
##    I2C_MONITOR_CCB_DEVICES ccbDevices;
##      I2C_MONITOR_DEVICE_PROPERTIES processor;
            $envStats{DEVICE_CCB_PROCESSOR_EVENT_PROP_STATUS},
            $envStats{DEVICE_CCB_PROCESSOR_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_CCB_PROCESSOR_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_CCB_PROCESSOR_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_CCB_PROCESSOR_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_CCB_PROCESSOR_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_CCB_PROCESSOR_FLAG_VALUE},
            $envStats{DEVICE_CCB_PROCESSOR_COMM_STATS_IANC},
            $envStats{DEVICE_CCB_PROCESSOR_COMM_STATS_TCTC},
            $envStats{DEVICE_CCB_PROCESSOR_COMM_STATS_RCTC},
            $envStats{DEVICE_CCB_PROCESSOR_COMM_STATS_BHC},
            $envStats{DEVICE_CCB_PROCESSOR_COMM_STATS_BTC},
            $envStats{DEVICE_CCB_PROCESSOR_COMM_STATS_BRC},
            $envStats{DEVICE_CCB_PROCESSOR_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES boardEEPROM;
            $envStats{DEVICE_CCB_BOARD_EEPROM_EVENT_PROP_STATUS},
            $envStats{DEVICE_CCB_BOARD_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_CCB_BOARD_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_CCB_BOARD_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_CCB_BOARD_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_CCB_BOARD_EEPROM_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_CCB_BOARD_EEPROM_FLAG_VALUE},
            $envStats{DEVICE_CCB_BOARD_EEPROM_COMM_STATS_IANC},
            $envStats{DEVICE_CCB_BOARD_EEPROM_COMM_STATS_TCTC},
            $envStats{DEVICE_CCB_BOARD_EEPROM_COMM_STATS_RCTC},
            $envStats{DEVICE_CCB_BOARD_EEPROM_COMM_STATS_BHC},
            $envStats{DEVICE_CCB_BOARD_EEPROM_COMM_STATS_BTC},
            $envStats{DEVICE_CCB_BOARD_EEPROM_COMM_STATS_BRC},
            $envStats{DEVICE_CCB_BOARD_EEPROM_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES memoryModuleEEPROM;
            $envStats{DEVICE_CCB_MEMORY_EEPROM_EVENT_PROP_STATUS},
            $envStats{DEVICE_CCB_MEMORY_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_CCB_MEMORY_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_CCB_MEMORY_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_CCB_MEMORY_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_CCB_MEMORY_EEPROM_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_CCB_MEMORY_EEPROM_FLAG_VALUE},
            $envStats{DEVICE_CCB_MEMORY_EEPROM_COMM_STATS_IANC},
            $envStats{DEVICE_CCB_MEMORY_EEPROM_COMM_STATS_TCTC},
            $envStats{DEVICE_CCB_MEMORY_EEPROM_COMM_STATS_RCTC},
            $envStats{DEVICE_CCB_MEMORY_EEPROM_COMM_STATS_BHC},
            $envStats{DEVICE_CCB_MEMORY_EEPROM_COMM_STATS_BTC},
            $envStats{DEVICE_CCB_MEMORY_EEPROM_COMM_STATS_BRC},
            $envStats{DEVICE_CCB_MEMORY_EEPROM_COMM_STATS_UDC},
##    I2C_MONITOR_PROC_BOARD_DEVICES procBoardDevices;
##      I2C_MONITOR_DEVICE_PROPERTIES pca9548;
            $envStats{DEVICE_PROC_PCA9548_EVENT_PROP_STATUS},
            $envStats{DEVICE_PROC_PCA9548_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_PROC_PCA9548_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_PROC_PCA9548_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_PROC_PCA9548_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_PROC_PCA9548_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_PROC_PCA9548_FLAG_VALUE},
            $envStats{DEVICE_PROC_PCA9548_COMM_STATS_IANC},
            $envStats{DEVICE_PROC_PCA9548_COMM_STATS_TCTC},
            $envStats{DEVICE_PROC_PCA9548_COMM_STATS_RCTC},
            $envStats{DEVICE_PROC_PCA9548_COMM_STATS_BHC},
            $envStats{DEVICE_PROC_PCA9548_COMM_STATS_BTC},
            $envStats{DEVICE_PROC_PCA9548_COMM_STATS_BRC},
            $envStats{DEVICE_PROC_PCA9548_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES lm80_lm87;
            $envStats{DEVICE_PROC_LM80_EVENT_PROP_STATUS},
            $envStats{DEVICE_PROC_LM80_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_PROC_LM80_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_PROC_LM80_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_PROC_LM80_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_PROC_LM80_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_PROC_LM80_FLAG_VALUE},
            $envStats{DEVICE_PROC_LM80_COMM_STATS_IANC},
            $envStats{DEVICE_PROC_LM80_COMM_STATS_TCTC},
            $envStats{DEVICE_PROC_LM80_COMM_STATS_RCTC},
            $envStats{DEVICE_PROC_LM80_COMM_STATS_BHC},
            $envStats{DEVICE_PROC_LM80_COMM_STATS_BTC},
            $envStats{DEVICE_PROC_LM80_COMM_STATS_BRC},
            $envStats{DEVICE_PROC_LM80_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES lm75_lm92;
            $envStats{DEVICE_PROC_LM75_EVENT_PROP_STATUS},
            $envStats{DEVICE_PROC_LM75_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_PROC_LM75_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_PROC_LM75_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_PROC_LM75_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_PROC_LM75_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_PROC_LM75_FLAG_VALUE},
            $envStats{DEVICE_PROC_LM75_COMM_STATS_IANC},
            $envStats{DEVICE_PROC_LM75_COMM_STATS_TCTC},
            $envStats{DEVICE_PROC_LM75_COMM_STATS_RCTC},
            $envStats{DEVICE_PROC_LM75_COMM_STATS_BHC},
            $envStats{DEVICE_PROC_LM75_COMM_STATS_BTC},
            $envStats{DEVICE_PROC_LM75_COMM_STATS_BRC},
            $envStats{DEVICE_PROC_LM75_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES resetControlPCF8574;
            $envStats{DEVICE_PROC_RCPCF8574_EVENT_PROP_STATUS},
            $envStats{DEVICE_PROC_RCPCF8574_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_PROC_RCPCF8574_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_PROC_RCPCF8574_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_PROC_RCPCF8574_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_PROC_RCPCF8574_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_PROC_RCPCF8574_FLAG_VALUE},
            $envStats{DEVICE_PROC_RCPCF8574_COMM_STATS_IANC},
            $envStats{DEVICE_PROC_RCPCF8574_COMM_STATS_TCTC},
            $envStats{DEVICE_PROC_RCPCF8574_COMM_STATS_RCTC},
            $envStats{DEVICE_PROC_RCPCF8574_COMM_STATS_BHC},
            $envStats{DEVICE_PROC_RCPCF8574_COMM_STATS_BTC},
            $envStats{DEVICE_PROC_RCPCF8574_COMM_STATS_BRC},
            $envStats{DEVICE_PROC_RCPCF8574_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES powerSupplyPCF8574;
            $envStats{DEVICE_PROC_PSPCF8574_EVENT_PROP_STATUS},
            $envStats{DEVICE_PROC_PSPCF8574_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_PROC_PSPCF8574_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_PROC_PSPCF8574_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_PROC_PSPCF8574_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_PROC_PSPCF8574_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_PROC_PSPCF8574_FLAG_VALUE},
            $envStats{DEVICE_PROC_PSPCF8574_COMM_STATS_IANC},
            $envStats{DEVICE_PROC_PSPCF8574_COMM_STATS_TCTC},
            $envStats{DEVICE_PROC_PSPCF8574_COMM_STATS_RCTC},
            $envStats{DEVICE_PROC_PSPCF8574_COMM_STATS_BHC},
            $envStats{DEVICE_PROC_PSPCF8574_COMM_STATS_BTC},
            $envStats{DEVICE_PROC_PSPCF8574_COMM_STATS_BRC},
            $envStats{DEVICE_PROC_PSPCF8574_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES chassisEEPROM;
            $envStats{DEVICE_PROC_CHASSIS_EEPROM_EVENT_PROP_STATUS},
            $envStats{DEVICE_PROC_CHASSIS_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_PROC_CHASSIS_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_PROC_CHASSIS_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_PROC_CHASSIS_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_PROC_CHASSIS_EEPROM_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_PROC_CHASSIS_EEPROM_FLAG_VALUE},
            $envStats{DEVICE_PROC_CHASSIS_EEPROM_COMM_STATS_IANC},
            $envStats{DEVICE_PROC_CHASSIS_EEPROM_COMM_STATS_TCTC},
            $envStats{DEVICE_PROC_CHASSIS_EEPROM_COMM_STATS_RCTC},
            $envStats{DEVICE_PROC_CHASSIS_EEPROM_COMM_STATS_BHC},
            $envStats{DEVICE_PROC_CHASSIS_EEPROM_COMM_STATS_BTC},
            $envStats{DEVICE_PROC_CHASSIS_EEPROM_COMM_STATS_BRC},
            $envStats{DEVICE_PROC_CHASSIS_EEPROM_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES procBoardEEPROM;
            $envStats{DEVICE_PROC_BOARD_EEPROM_EVENT_PROP_STATUS},
            $envStats{DEVICE_PROC_BOARD_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_PROC_BOARD_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_PROC_BOARD_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_PROC_BOARD_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_PROC_BOARD_EEPROM_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_PROC_BOARD_EEPROM_FLAG_VALUE},
            $envStats{DEVICE_PROC_BOARD_EEPROM_COMM_STATS_IANC},
            $envStats{DEVICE_PROC_BOARD_EEPROM_COMM_STATS_TCTC},
            $envStats{DEVICE_PROC_BOARD_EEPROM_COMM_STATS_RCTC},
            $envStats{DEVICE_PROC_BOARD_EEPROM_COMM_STATS_BHC},
            $envStats{DEVICE_PROC_BOARD_EEPROM_COMM_STATS_BTC},
            $envStats{DEVICE_PROC_BOARD_EEPROM_COMM_STATS_BRC},
            $envStats{DEVICE_PROC_BOARD_EEPROM_COMM_STATS_UDC},
##    I2C_MONITOR_POWER_SUPPLY_DEVICES frontEndPowerSupplyDevices;
##      I2C_MONITOR_DEVICE_PROPERTIES assemblyEEPROM;
            $envStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_EVENT_PROP_STATUS},
            $envStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_FLAG_VALUE},
            $envStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_COMM_STATS_IANC},
            $envStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_COMM_STATS_TCTC},
            $envStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_COMM_STATS_RCTC},
            $envStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_COMM_STATS_BHC},
            $envStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_COMM_STATS_BTC},
            $envStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_COMM_STATS_BRC},
            $envStats{DEVICE_SUPPLY_FE_ASSEMBLY_EEPROM_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES interfaceEEPROM;
            $envStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_EVENT_PROP_STATUS},
            $envStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_FLAG_VALUE},
            $envStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_COMM_STATS_IANC},
            $envStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_COMM_STATS_TCTC},
            $envStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_COMM_STATS_RCTC},
            $envStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_COMM_STATS_BHC},
            $envStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_COMM_STATS_BTC},
            $envStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_COMM_STATS_BRC},
            $envStats{DEVICE_SUPPLY_FE_INTERFACE_EEPROM_COMM_STATS_UDC},
##    I2C_MONITOR_POWER_SUPPLY_DEVICES backEndPowerSupplyDevices;
##      I2C_MONITOR_DEVICE_PROPERTIES assemblyEEPROM;
            $envStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_EVENT_PROP_STATUS},
            $envStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_FLAG_VALUE},
            $envStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_COMM_STATS_IANC},
            $envStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_COMM_STATS_TCTC},
            $envStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_COMM_STATS_RCTC},
            $envStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_COMM_STATS_BHC},
            $envStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_COMM_STATS_BTC},
            $envStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_COMM_STATS_BRC},
            $envStats{DEVICE_SUPPLY_BE_ASSEMBLY_EEPROM_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES interfaceEEPROM;
            $envStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_EVENT_PROP_STATUS},
            $envStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_FLAG_VALUE},
            $envStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_COMM_STATS_IANC},
            $envStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_COMM_STATS_TCTC},
            $envStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_COMM_STATS_RCTC},
            $envStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_COMM_STATS_BHC},
            $envStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_COMM_STATS_BTC},
            $envStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_COMM_STATS_BRC},
            $envStats{DEVICE_SUPPLY_BE_INTERFACE_EEPROM_COMM_STATS_UDC},
##    I2C_MONITOR_BUFFER_BOARD_DEVICES frontEndBufferBoardDevices;
##      I2C_MONITOR_DEVICE_PROPERTIES lm80_lm87;
            $envStats{DEVICE_BATT_FE_LM80_EVENT_PROP_STATUS},
            $envStats{DEVICE_BATT_FE_LM80_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_BATT_FE_LM80_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_BATT_FE_LM80_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_BATT_FE_LM80_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_BATT_FE_LM80_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_BATT_FE_LM80_FLAG_VALUE},
            $envStats{DEVICE_BATT_FE_LM80_COMM_STATS_IANC},
            $envStats{DEVICE_BATT_FE_LM80_COMM_STATS_TCTC},
            $envStats{DEVICE_BATT_FE_LM80_COMM_STATS_RCTC},
            $envStats{DEVICE_BATT_FE_LM80_COMM_STATS_BHC},
            $envStats{DEVICE_BATT_FE_LM80_COMM_STATS_BTC},
            $envStats{DEVICE_BATT_FE_LM80_COMM_STATS_BRC},
            $envStats{DEVICE_BATT_FE_LM80_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES pcf8574;
            $envStats{DEVICE_BATT_FE_PCF8574_EVENT_PROP_STATUS},
            $envStats{DEVICE_BATT_FE_PCF8574_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_BATT_FE_PCF8574_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_BATT_FE_PCF8574_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_BATT_FE_PCF8574_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_BATT_FE_PCF8574_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_BATT_FE_PCF8574_FLAG_VALUE},
            $envStats{DEVICE_BATT_FE_PCF8574_COMM_STATS_IANC},
            $envStats{DEVICE_BATT_FE_PCF8574_COMM_STATS_TCTC},
            $envStats{DEVICE_BATT_FE_PCF8574_COMM_STATS_RCTC},
            $envStats{DEVICE_BATT_FE_PCF8574_COMM_STATS_BHC},
            $envStats{DEVICE_BATT_FE_PCF8574_COMM_STATS_BTC},
            $envStats{DEVICE_BATT_FE_PCF8574_COMM_STATS_BRC},
            $envStats{DEVICE_BATT_FE_PCF8574_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES max1660;
            $envStats{DEVICE_BATT_FE_MAX1660_EVENT_PROP_STATUS},
            $envStats{DEVICE_BATT_FE_MAX1660_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_BATT_FE_MAX1660_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_BATT_FE_MAX1660_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_BATT_FE_MAX1660_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_BATT_FE_MAX1660_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_BATT_FE_MAX1660_FLAG_VALUE},
            $envStats{DEVICE_BATT_FE_MAX1660_COMM_STATS_IANC},
            $envStats{DEVICE_BATT_FE_MAX1660_COMM_STATS_TCTC},
            $envStats{DEVICE_BATT_FE_MAX1660_COMM_STATS_RCTC},
            $envStats{DEVICE_BATT_FE_MAX1660_COMM_STATS_BHC},
            $envStats{DEVICE_BATT_FE_MAX1660_COMM_STATS_BTC},
            $envStats{DEVICE_BATT_FE_MAX1660_COMM_STATS_BRC},
            $envStats{DEVICE_BATT_FE_MAX1660_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES eeprom;
            $envStats{DEVICE_BATT_FE_EEPROM_EVENT_PROP_STATUS},
            $envStats{DEVICE_BATT_FE_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_BATT_FE_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_BATT_FE_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_BATT_FE_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_BATT_FE_EEPROM_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_BATT_FE_EEPROM_FLAG_VALUE},
            $envStats{DEVICE_BATT_FE_EEPROM_COMM_STATS_IANC},
            $envStats{DEVICE_BATT_FE_EEPROM_COMM_STATS_TCTC},
            $envStats{DEVICE_BATT_FE_EEPROM_COMM_STATS_RCTC},
            $envStats{DEVICE_BATT_FE_EEPROM_COMM_STATS_BHC},
            $envStats{DEVICE_BATT_FE_EEPROM_COMM_STATS_BTC},
            $envStats{DEVICE_BATT_FE_EEPROM_COMM_STATS_BRC},
            $envStats{DEVICE_BATT_FE_EEPROM_COMM_STATS_UDC},
##    I2C_MONITOR_BUFFER_BOARD_DEVICES backEndBufferBoardDevices;
##      I2C_MONITOR_DEVICE_PROPERTIES lm80_lm87;
            $envStats{DEVICE_BATT_BE_LM80_EVENT_PROP_STATUS},
            $envStats{DEVICE_BATT_BE_LM80_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_BATT_BE_LM80_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_BATT_BE_LM80_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_BATT_BE_LM80_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_BATT_BE_LM80_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_BATT_BE_LM80_FLAG_VALUE},
            $envStats{DEVICE_BATT_BE_LM80_COMM_STATS_IANC},
            $envStats{DEVICE_BATT_BE_LM80_COMM_STATS_TCTC},
            $envStats{DEVICE_BATT_BE_LM80_COMM_STATS_RCTC},
            $envStats{DEVICE_BATT_BE_LM80_COMM_STATS_BHC},
            $envStats{DEVICE_BATT_BE_LM80_COMM_STATS_BTC},
            $envStats{DEVICE_BATT_BE_LM80_COMM_STATS_BRC},
            $envStats{DEVICE_BATT_BE_LM80_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES pcf8574;
            $envStats{DEVICE_BATT_BE_PCF8574_EVENT_PROP_STATUS},
            $envStats{DEVICE_BATT_BE_PCF8574_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_BATT_BE_PCF8574_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_BATT_BE_PCF8574_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_BATT_BE_PCF8574_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_BATT_BE_PCF8574_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_BATT_BE_PCF8574_FLAG_VALUE},
            $envStats{DEVICE_BATT_BE_PCF8574_COMM_STATS_IANC},
            $envStats{DEVICE_BATT_BE_PCF8574_COMM_STATS_TCTC},
            $envStats{DEVICE_BATT_BE_PCF8574_COMM_STATS_RCTC},
            $envStats{DEVICE_BATT_BE_PCF8574_COMM_STATS_BHC},
            $envStats{DEVICE_BATT_BE_PCF8574_COMM_STATS_BTC},
            $envStats{DEVICE_BATT_BE_PCF8574_COMM_STATS_BRC},
            $envStats{DEVICE_BATT_BE_PCF8574_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES max1660;
            $envStats{DEVICE_BATT_BE_MAX1660_EVENT_PROP_STATUS},
            $envStats{DEVICE_BATT_BE_MAX1660_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_BATT_BE_MAX1660_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_BATT_BE_MAX1660_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_BATT_BE_MAX1660_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_BATT_BE_MAX1660_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_BATT_BE_MAX1660_FLAG_VALUE},
            $envStats{DEVICE_BATT_BE_MAX1660_COMM_STATS_IANC},
            $envStats{DEVICE_BATT_BE_MAX1660_COMM_STATS_TCTC},
            $envStats{DEVICE_BATT_BE_MAX1660_COMM_STATS_RCTC},
            $envStats{DEVICE_BATT_BE_MAX1660_COMM_STATS_BHC},
            $envStats{DEVICE_BATT_BE_MAX1660_COMM_STATS_BTC},
            $envStats{DEVICE_BATT_BE_MAX1660_COMM_STATS_BRC},
            $envStats{DEVICE_BATT_BE_MAX1660_COMM_STATS_UDC},
##      I2C_MONITOR_DEVICE_PROPERTIES eeprom;
            $envStats{DEVICE_BATT_BE_EEPROM_EVENT_PROP_STATUS},
            $envStats{DEVICE_BATT_BE_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{DEVICE_BATT_BE_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{DEVICE_BATT_BE_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{DEVICE_BATT_BE_EEPROM_EVENT_PROP_FLAG_VALUE},
            $envStats{DEVICE_BATT_BE_EEPROM_EVENT_PROP_DESCRIPTION},
            $envStats{DEVICE_BATT_BE_EEPROM_FLAG_VALUE},
            $envStats{DEVICE_BATT_BE_EEPROM_COMM_STATS_IANC},
            $envStats{DEVICE_BATT_BE_EEPROM_COMM_STATS_TCTC},
            $envStats{DEVICE_BATT_BE_EEPROM_COMM_STATS_RCTC},
            $envStats{DEVICE_BATT_BE_EEPROM_COMM_STATS_BHC},
            $envStats{DEVICE_BATT_BE_EEPROM_COMM_STATS_BTC},
            $envStats{DEVICE_BATT_BE_EEPROM_COMM_STATS_BRC},
            $envStats{DEVICE_BATT_BE_EEPROM_COMM_STATS_UDC}) =
                unpack($I2C_HW_DATA, $parts{DATA});
        }
        else
        {
            $self->_handleError($recvPacket);
            logMsg("Unexpected packet: We expected a system info. packet\n");
        }
    }

    return %envStats;
}


##############################################################################
# Name:  _statsBufferBoardPacket
#
# Desc: Parses the system information packet and places the information in a
#       hash
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: Empty hash on error, else a hash with the following elements:
#
##############################################################################
sub _statsBufferBoardPacket
{
    my ($self, $seq, $recvPacket) = @_;
    my %statsBufferBoard;

    if( !(defined($recvPacket)) )
    {
        $recvPacket = $self->_receivePacketSync($seq);
    }

    if( defined($recvPacket) )
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $statsBufferBoard{STATUS}     = $parts{STATUS};
        $statsBufferBoard{ERROR_CODE} = $parts{ERROR_CODE};
##        print "Data Length: $parts{DATA_LENGTH}\n";

        if( commandCode($recvPacket) == PI_STATS_BUFFER_BOARD_CMD )
        {
            # MR_HDR_RSP header
            my $MR_HDR_RSP_RSVD                             = "CCC";
            my $MR_HDR_RSP_STATUS                           = "C";
            my $MR_HDR_RSP_LEN                              = "L";
            my $MR_HDR_RSP                                  = $MR_HDR_RSP_RSVD . 
                                                              $MR_HDR_RSP_STATUS .
                                                              $MR_HDR_RSP_LEN;
            
            # NVRAM_BOARD_REVISION revision
            my $BOARD_REVISION_MAJOR                        = "L";
            my $BOARD_REVISION_MINOR                        = "L";
            my $BOARD_REVISION                              = $BOARD_REVISION_MAJOR .
                                                              $BOARD_REVISION_MINOR;

            # NVRAM_BATTERY_INFO batteryInformation
            my $BATTERY_INFO_STATUS                         = "L";
            my $BATTERY_INFO_VOLTAGE                        = "S";
            my $BATTERY_INFO_CHARGE_PERCENT                 = "S";
            my $BATTERY_INFO                                = $BATTERY_INFO_STATUS .
                                                              $BATTERY_INFO_VOLTAGE .
                                                              $BATTERY_INFO_CHARGE_PERCENT;

            # NVRAM_BOARD_INFO boardInfo
            my $BOARD_STATUS                                = "L";
            my $MEMORY_SIZE                                 = "L";
            my $MEMORY_ERROR_COUNT                          = "L";
            my $BATTERY_COUNT                               = "L";
            my $BUFFER_BOARD_INFO                           = $MR_HDR_RSP .
                                                              $BOARD_STATUS .
                                                              $BOARD_REVISION .
                                                              $MEMORY_SIZE .
                                                              $MEMORY_ERROR_COUNT .
                                                              $BATTERY_COUNT .
                                                              $BATTERY_INFO .
                                                              $BATTERY_INFO .
                                                              $BATTERY_INFO .
                                                              $BATTERY_INFO;

            (
                # MR_HDR_RSP
                $statsBufferBoard{MR_HDR_RSP_RSVD_1},
                $statsBufferBoard{MR_HDR_RSP_RSVD_2},
                $statsBufferBoard{MR_HDR_RSP_RSVD_3},
                $statsBufferBoard{MR_HDR_RSP_STATUS},
                $statsBufferBoard{MR_HDR_RSP_LENGTH},

                # BOARD_INFO
                $statsBufferBoard{BUFFER_BOARD_STATUS},

                # BOARD_REVISION
                $statsBufferBoard{BUFFER_BOARD_REVISION_MAJOR},
                $statsBufferBoard{BUFFER_BOARD_REVISION_MINOR},

                $statsBufferBoard{BUFFER_BOARD_MEMORY_SIZE},
                $statsBufferBoard{BUFFER_BOARD_MEMORY_ERROR_COUNT},
                $statsBufferBoard{BUFFER_BOARD_BATTERY_COUNT},

                # BATTERY_INFO
                $statsBufferBoard{BUFFER_BOARD_BATTERY_1_STATUS},
                $statsBufferBoard{BUFFER_BOARD_BATTERY_1_VOLTAGE},
                $statsBufferBoard{BUFFER_BOARD_BATTERY_1_CHARGE_PERCENT},

                $statsBufferBoard{BUFFER_BOARD_BATTERY_2_STATUS},
                $statsBufferBoard{BUFFER_BOARD_BATTERY_2_VOLTAGE},
                $statsBufferBoard{BUFFER_BOARD_BATTERY_2_CHARGE_PERCENT},

                $statsBufferBoard{BUFFER_BOARD_BATTERY_3_STATUS},
                $statsBufferBoard{BUFFER_BOARD_BATTERY_3_VOLTAGE},
                $statsBufferBoard{BUFFER_BOARD_BATTERY_3_CHARGE_PERCENT},

                $statsBufferBoard{BUFFER_BOARD_BATTERY_4_STATUS},
                $statsBufferBoard{BUFFER_BOARD_BATTERY_4_VOLTAGE},
                $statsBufferBoard{BUFFER_BOARD_BATTERY_4_CHARGE_PERCENT}
            ) = unpack($BUFFER_BOARD_INFO, $parts{DATA});
        }
        else
        {
            $self->_handleError( $recvPacket );
            logMsg( "Unexpected packet: We expected a system info. packet\n" );
        }
    }

    return %statsBufferBoard;
}


##############################################################################
# NAME:     _statsCacheDevices
#
# DESC:     Parses the cache devices statistics packet and places the
#           information in a hash.
#
# INPUT:    scalar  $sequenceID         Sequence id
#           scalar  $recvPacket         Packet to parse
#
# Returns: Empty hash on error, else a hash with the following elements:
##############################################################################
sub _statsCacheDevices
{
    my ($self, $seq, $recvPacket) = @_;

    my %info;

    if (commandCode($recvPacket) == PI_STATS_CACHE_DEVICES_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $info{COUNT}
        ) = unpack("S", $parts{DATA});

        my @cacheDevs;

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            my $start = 4 + (136 * $i);

            my $rsvd;
            my %vc_flushLBA;
            my %vc_rdhits;
            my %vc_rdpart;
            my %vc_rdmiss;
            my %vc_wrhits;
            my %vc_wrpart;
            my %vc_wrmiss;
            my %vc_wrtbyres;
            my %vc_wrtbylen;
            my %vc_capacity;

            (
            $rsvd,
            $cacheDevs[$i]{STATUS_MRP},
            $cacheDevs[$i]{LEN},
            $cacheDevs[$i]{VC_VID},
            $cacheDevs[$i]{VC_ERR_FLUSH_CNT},
            $cacheDevs[$i]{VC_STAT},
            $rsvd,
            $cacheDevs[$i]{VC_CACHE},
            $cacheDevs[$i]{VC_DIRTY},
            $cacheDevs[$i]{VC_IO},
            $cacheDevs[$i]{VC_WRT_CNT},
            $vc_flushLBA{LO_LONG}, $vc_flushLBA{HI_LONG},
            $vc_rdhits{LO_LONG}, $vc_rdhits{HI_LONG},
            $vc_rdpart{LO_LONG}, $vc_rdpart{HI_LONG},
            $vc_rdmiss{LO_LONG}, $vc_rdmiss{HI_LONG},
            $vc_wrhits{LO_LONG}, $vc_wrhits{HI_LONG},
            $vc_wrpart{LO_LONG}, $vc_wrpart{HI_LONG},
            $vc_wrmiss{LO_LONG}, $vc_wrmiss{HI_LONG},
            $vc_wrtbyres{LO_LONG}, $vc_wrtbyres{HI_LONG},
            $vc_wrtbylen{LO_LONG}, $vc_wrtbylen{HI_LONG},
            $vc_capacity{LO_LONG}, $vc_capacity{HI_LONG},
            $rsvd,
            $cacheDevs[$i]{VC_VTV},
            $cacheDevs[$i]{VC_THEAD},
            $cacheDevs[$i]{VC_TTAIL},
            $cacheDevs[$i]{VC_FWD_WAIT},
            $cacheDevs[$i]{VC_BWD_WAIT}
            ) = unpack("a3CLSCCa4LLLLLLLLLLLLLLLLLLLLLLLLa4LLLLL", substr($parts{DATA}, $start));


            # Now fixup all the 64 bit  numbers
            $cacheDevs[$i]{VC_FLUSHLBA} = longsToBigInt(%vc_flushLBA);
            $cacheDevs[$i]{VC_RDHITS} = longsToBigInt(%vc_rdhits);
            $cacheDevs[$i]{VC_RDPART} = longsToBigInt(%vc_rdpart);
            $cacheDevs[$i]{VC_RDMISS} = longsToBigInt(%vc_rdmiss);
            $cacheDevs[$i]{VC_WRHITS} = longsToBigInt(%vc_wrhits);
            $cacheDevs[$i]{VC_WRPART} = longsToBigInt(%vc_wrpart);
            $cacheDevs[$i]{VC_WRMISS} = longsToBigInt(%vc_wrmiss);
            $cacheDevs[$i]{VC_WRTBYRES} = longsToBigInt(%vc_wrtbyres);
            $cacheDevs[$i]{VC_WRTBYLEN} = longsToBigInt(%vc_wrtbylen);
            $cacheDevs[$i]{VC_CAPACITY} = longsToBigInt(%vc_capacity);
        }

        $info{CACHEDEVS} = [@cacheDevs];
    }
    elsif (commandCode($recvPacket) == PI_STATS_CACHE_DEVICE_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        my $rsvd;
        my %vc_flushLBA;
        my %vc_rdhits;
        my %vc_rdpart;
        my %vc_rdmiss;
        my %vc_wrhits;
        my %vc_wrpart;
        my %vc_wrmiss;
        my %vc_wrtbyres;
        my %vc_wrtbylen;
        my %vc_capacity;

        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{VC_VID},
        $info{VC_ERR_FLUSH_CNT},
        $info{VC_STAT},
        $rsvd,
        $info{VC_CACHE},
        $info{VC_DIRTY},
        $info{VC_IO},
        $info{VC_WRT_CNT},
        $vc_flushLBA{LO_LONG}, $vc_flushLBA{HI_LONG},
        $vc_rdhits{LO_LONG}, $vc_rdhits{HI_LONG},
        $vc_rdpart{LO_LONG}, $vc_rdpart{HI_LONG},
        $vc_rdmiss{LO_LONG}, $vc_rdmiss{HI_LONG},
        $vc_wrhits{LO_LONG}, $vc_wrhits{HI_LONG},
        $vc_wrpart{LO_LONG}, $vc_wrpart{HI_LONG},
        $vc_wrmiss{LO_LONG}, $vc_wrmiss{HI_LONG},
        $vc_wrtbyres{LO_LONG}, $vc_wrtbyres{HI_LONG},
        $vc_wrtbylen{LO_LONG}, $vc_wrtbylen{HI_LONG},
        $vc_capacity{LO_LONG}, $vc_capacity{HI_LONG},
        $rsvd,
        $info{VC_VTV},
        $info{VC_THEAD},
        $info{VC_TTAIL},
        $info{VC_FWD_WAIT},
        $info{VC_BWD_WAIT}
        ) = unpack("a3CLSCCa4LLLLLLLLLLLLLLLLLLLLLLLLa4LLLLL", $parts{DATA});

        # Now fixup all the 64 bit  numbers
        $info{VC_FLUSHLBA} = longsToBigInt(%vc_flushLBA);
        $info{VC_RDHITS} = longsToBigInt(%vc_rdhits);
        $info{VC_RDPART} = longsToBigInt(%vc_rdpart);
        $info{VC_RDMISS} = longsToBigInt(%vc_rdmiss);
        $info{VC_WRHITS} = longsToBigInt(%vc_wrhits);
        $info{VC_WRPART} = longsToBigInt(%vc_wrpart);
        $info{VC_WRMISS} = longsToBigInt(%vc_wrmiss);
        $info{VC_WRTBYRES} = longsToBigInt(%vc_wrtbyres);
        $info{VC_WRTBYLEN} = longsToBigInt(%vc_wrtbylen);
        $info{VC_CAPACITY} = longsToBigInt(%vc_capacity);
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a cache devices statistics packet.\n");
    }

    return %info;
}

##############################################################################
# NAME:     _statsLoop
#
# DESC:     Parses the loop statistics packet and places the
#           information in a hash.
#
# INPUT:    scalar  $sequenceID         Sequence id
#           scalar  $recvPacket         Packet to parse
#
# Returns: Empty hash on error, else a hash with the following elements:
##############################################################################
sub _statsLoop
{
    my ($self, $seq, $recvPacket) = @_;

    my %info;

    if (commandCode($recvPacket) == PI_STATS_BACK_END_LOOP_CMD ||
        commandCode($recvPacket) == PI_STATS_FRONT_END_LOOP_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # /* --- Front End or Back End Loop Statistics Response --- */
        # typedef struct _PI_STATS_LOOPS_RSP
        # {
        #     UINT16          count;      /* Number of valid ports    */
        #     UINT8           rsvd[2];    /* RESERVED                 */
        #     PI_STATS_LOOP   stats[0];   /* Start of loop stats      */
        # } PI_STATS_LOOPS_RSP;

        (
        $info{PORT_COUNT},
        $rsvd
        ) = unpack("SS", $parts{DATA});

        my $start = 4;     # adjust start for the data above

        # Loop through each port's data
        for (my $i = 0; $i < $info{PORT_COUNT}; $i++)
        {
            my $data = substr $parts{DATA}, $start;

            # typedef struct _PI_STATS_LOOP
            # {
            #     UINT16      length;     /* Length of data to follow     */
            #     UINT16      port;       /* Number of valid channels     */
            #     PI_LOOP_OUT stats[0];   /* Start of loop stats          */
            # } PI_STATS_LOOP;

            (
            $info{$i}{LEN_THIS_PORT},
            $info{$i}{PORT},

            $rsvd,
            $info{$i}{STATUS_MRP},
            $info{$i}{LEN},
            $info{$i}{NUMHOSTS},
            $info{$i}{LID},
            $rsvd,
            $info{$i}{LIFCNT},
            $info{$i}{LSSCNT},
            $info{$i}{LSGCNT},
            $info{$i}{PSPEC},
            $info{$i}{IVTQC},
            $info{$i}{IVCRC},
            $info{$i}{VENDID},
            $info{$i}{MODEL},
            $info{$i}{REVLEVEL},
            $info{$i}{RISCLEVEL},

            $info{$i}{FPMLEVEL},
            $info{$i}{FPLEVEL},
            $info{$i}{ROMLEVEL},
            $info{$i}{TYPE},
            $info{$i}{FWMAJOR},
            $info{$i}{FWMINOR},
            $info{$i}{FWSUB},
            $info{$i}{FWATTRIB},
            $info{$i}{DATARATE},
            $info{$i}{STATE},
            $info{$i}{NUMTARG},
            $info{$i}{GPIOD},
            $rsvd
            ) = unpack("SS a3CL CCa2 LLLLLLLL SSCCS a8 SSSS SSCCCa2", $data);

            # Move the start point past the above data.  LEN_THIS_PORT
            # does not include its own size so add 2 bytes for this.
            #
            $start += $info{$i}{LEN_THIS_PORT} + 2 - (2 * $info{$i}{NUMTARG});

            my $tgt = substr($parts{DATA}, $start);
            my @tgts;

            # Get the target IDs
            for (my $j = 0; $j < $info{$i}{NUMTARG}; $j++)
            {
                my $start = 2 * $j;
                (
                $tgts[$j]{TID}
                ) = unpack("S", substr($tgt, $start, 2));
            }

            $start += 2 * $info{$i}{NUMTARG};

            $info{$i}{TGTS} = [@tgts];
        }
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a loop statistics packet.\n");
    }

    return %info;
}

##############################################################################
# NAME:     _statsPCI
#
# DESC:     Parses the PCI statistics packet and places the
#           information in a hash.
#
# INPUT:    scalar  $sequenceID         Sequence id
#           scalar  $recvPacket         Packet to parse
#
# Returns: Empty hash on error, else a hash with the following elements:
##############################################################################
sub _statsPCI
{
    my ($self, $seq, $recvPacket) = @_;

    my %info;

    if (commandCode($recvPacket) == PI_STATS_BACK_END_PCI_CMD ||
        commandCode($recvPacket) == PI_STATS_FRONT_END_PCI_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{VRPOCOUNT},
        $info{VRPICOUNT},
        $info{VRPOTCOUNT},
        $info{VRPITCOUNT},
        $rsvd
        ) = unpack("a3CLSSLLa4", $parts{DATA});
    }
    elsif (commandCode($recvPacket) == PI_STATS_PCI_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        # FE PCI STATS
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{FE_VRPOCOUNT},
        $info{FE_VRPICOUNT},
        $info{FE_VRPOTCOUNT},
        $info{FE_VRPITCOUNT},
        $rsvd,

        # BE PCI_STATS
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{BE_VRPOCOUNT},
        $info{BE_VRPICOUNT},
        $info{BE_VRPOTCOUNT},
        $info{BE_VRPITCOUNT},
        $rsvd
        ) = unpack("a3CLSSLLa4a3CLSSLLa4", $parts{DATA});
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a PCI statistics packet.\n");
    }

    return %info;
}

##############################################################################
# NAME:     _statsProc
#
# DESC:     Parses the proc statistics packet and places the
#           information in a hash.
#
# INPUT:    scalar  $sequenceID         Sequence id
#           scalar  $recvPacket         Packet to parse
#
# Returns: Empty hash on error, else a hash with the following elements:
##############################################################################
sub _statsProc
{
    my ($self, $seq, $recvPacket) = @_;

    my %info;

    if (commandCode($recvPacket) == PI_STATS_BACK_END_PROC_CMD ||
        commandCode($recvPacket) == PI_STATS_FRONT_END_PROC_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{II_VERS},
        $info{II_REV},
        $info{II_USER},

        $info{II_BDATE},
        $info{II_BTIME},

        $info{II_BCOUNT},
        $info{II_STATUS},
        $info{II_CHGCNT},
        $info{II_SCRUB},
        $info{II_GPRI},
        $info{II_UTZN},
        $info{II_TIME},
        $info{II_IRCUR},

        $info{II_IRMAX},
        $info{II_IRMIN},
        $info{II_IRWAIT},
        $info{II_SDCUR},

        $info{II_SDMAX},
        $info{II_SDMIN},
        $info{II_SDWAIT},
        $info{II_NCCUR},

        $info{II_NCMAX},
        $info{II_NCMIN},
        $info{II_NCWAIT},
        $info{II_RSCUR},

        $info{II_RSMAX},
        $info{II_RSMIN},
        $info{II_RSWAIT},
        $info{II_PCBCUR},

        $info{II_PCBMAX},
        $info{II_ILTCUR},
        $info{II_ILTMAX},
        $info{II_PRPCUR},

        $info{II_PRPMAX},
        $info{II_RRPCUR},
        $info{II_RRPMAX},
        $info{II_SCBCUR},

        $info{II_SCBMAX},
        $info{II_RPNCUR},
        $info{II_RPNMAX},
        $info{II_RRBCUR},

        $info{II_RRBMAX},
        $rsvd,
        $info{II_NVACUR},
        $info{II_NVAMAX},

        $info{II_NVAMIN},
        $info{II_NVAWAIT}
        ) = unpack("a3CLSSa4 a8a8 SSCCCCLL LLLL LLLL LLLL LLLL LLLL LLLL LLLL La4LL LL", $parts{DATA});
    }
    elsif (commandCode($recvPacket) == PI_STATS_PROC_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        # FE PROC STATS
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{FE_II_VERS},
        $info{FE_II_REV},
        $info{FE_II_USER},

        $info{FE_II_BDATE},
        $info{FE_II_BTIME},

        $info{FE_II_BCOUNT},
        $info{FE_II_STATUS},
        $info{FE_II_CHGCNT},
        $info{FE_II_SCRUB},
        $info{FE_II_GPRI},
        $info{FE_II_UTZN},
        $info{FE_II_TIME},
        $info{FE_II_IRCUR},

        $info{FE_II_IRMAX},
        $info{FE_II_IRMIN},
        $info{FE_II_IRWAIT},
        $info{FE_II_SDCUR},

        $info{FE_II_SDMAX},
        $info{FE_II_SDMIN},
        $info{FE_II_SDWAIT},
        $info{FE_II_NCCUR},

        $info{FE_II_NCMAX},
        $info{FE_II_NCMIN},
        $info{FE_II_NCWAIT},
        $info{FE_II_RSCUR},

        $info{FE_II_RSMAX},
        $info{FE_II_RSMIN},
        $info{FE_II_RSWAIT},
        $info{FE_II_PCBCUR},

        $info{FE_II_PCBMAX},
        $info{FE_II_ILTCUR},
        $info{FE_II_ILTMAX},
        $info{FE_II_PRPCUR},

        $info{FE_II_PRPMAX},
        $info{FE_II_RRPCUR},
        $info{FE_II_RRPMAX},
        $info{FE_II_SCBCUR},

        $info{FE_II_SCBMAX},
        $info{FE_II_RPNCUR},
        $info{FE_II_RPNMAX},
        $info{FE_II_RRBCUR},

        $info{FE_II_RRBMAX},
        $rsvd,
        $info{FE_II_NVACUR},
        $info{FE_II_NVAMAX},

        $info{FE_II_NVAMIN},
        $info{FE_II_NVAWAIT},

        # BE PROC STATS
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{BE_II_VERS},
        $info{BE_II_REV},
        $info{BE_II_USER},

        $info{BE_II_BDATE},
        $info{BE_II_BTIME},

        $info{BE_II_BCOUNT},
        $info{BE_II_STATUS},
        $info{BE_II_CHGCNT},
        $info{BE_II_SCRUB},
        $info{BE_II_GPRI},
        $info{BE_II_UTZN},
        $info{BE_II_TIME},
        $info{BE_II_IRCUR},

        $info{BE_II_IRMAX},
        $info{BE_II_IRMIN},
        $info{BE_II_IRWAIT},
        $info{BE_II_SDCUR},

        $info{BE_II_SDMAX},
        $info{BE_II_SDMIN},
        $info{BE_II_SDWAIT},
        $info{BE_II_NCCUR},

        $info{BE_II_NCMAX},
        $info{BE_II_NCMIN},
        $info{BE_II_NCWAIT},
        $info{BE_II_RSCUR},

        $info{BE_II_RSMAX},
        $info{BE_II_RSMIN},
        $info{BE_II_RSWAIT},
        $info{BE_II_PCBCUR},

        $info{BE_II_PCBMAX},
        $info{BE_II_ILTCUR},
        $info{BE_II_ILTMAX},
        $info{BE_II_PRPCUR},

        $info{BE_II_PRPMAX},
        $info{BE_II_RRPCUR},
        $info{BE_II_RRPMAX},
        $info{BE_II_SCBCUR},

        $info{BE_II_SCBMAX},
        $info{BE_II_RPNCUR},
        $info{BE_II_RPNMAX},
        $info{BE_II_RRBCUR},

        $info{BE_II_RRBMAX},
        $rsvd,
        $info{BE_II_NVACUR},
        $info{BE_II_NVAMAX},

        $info{BE_II_NVAMIN},
        $info{BE_II_NVAWAIT}
        ) = unpack("a3CLSSa4 a8a8 SSCCCCLL LLLL LLLL LLLL LLLL LLLL LLLL LLLL La4LL LL  a3CLSSa4 a8a8 SSCCCCLL LLLL LLLL LLLL LLLL LLLL LLLL LLLL La4LL LL", $parts{DATA});
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a proc statistics packet.\n");
    }

    return %info;
}

##############################################################################
# NAME:     _statsServer
#
# DESC:     Parses the server statistics packet and places the
#           information in a hash.
#
# INPUT:    scalar  $sequenceID         Sequence id
#           scalar  $recvPacket         Packet to parse
#
# Returns: Empty hash on error, else a hash with the following elements:
##############################################################################
sub _statsServer
{
    my ($self, $seq, $recvPacket) = @_;

    my %info;

    if (commandCode($recvPacket) == PI_STATS_SERVER_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;
        my %ag_cmds;
        my %ag_bytes;
        my %ag_writes;
        my %ag_wbytes;
        my %ag_reads;
        my %ag_rbytes;
        my %per_cmds;
        my %per_bytes;
        my %per_writes;
        my %per_wbytes;
        my %per_reads;
        my %per_rbytes;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $ag_cmds{LO_LONG}, $ag_cmds{HI_LONG},
        $ag_bytes{LO_LONG}, $ag_bytes{HI_LONG},
        $ag_writes{LO_LONG}, $ag_writes{HI_LONG},
        $ag_wbytes{LO_LONG}, $ag_wbytes{HI_LONG},
        $ag_reads{LO_LONG}, $ag_reads{HI_LONG},
        $ag_rbytes{LO_LONG}, $ag_rbytes{HI_LONG},
        $per_cmds{LO_LONG}, $per_cmds{HI_LONG},
        $per_bytes{LO_LONG}, $per_bytes{HI_LONG},
        $per_writes{LO_LONG}, $per_writes{HI_LONG},
        $per_wbytes{LO_LONG}, $per_wbytes{HI_LONG},
        $per_reads{LO_LONG}, $per_reads{HI_LONG},
        $per_rbytes{LO_LONG}, $per_rbytes{HI_LONG},
        $info{FL_LIFCNT},
        $info{FL_LSSCNT},
        $info{FL_LSGCNT},
        $info{FL_PSPEC},
        $info{FL_IVTQC},
        $info{FL_IVCRC},
        $info{QDEPTH},
        $rsvd
        ) = unpack("a3CLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLSa2", $parts{DATA});

        # Now fixup all the 64 bit  numbers
        $info{AG_CMDS} = longsToBigInt(%ag_cmds);
        $info{AG_BYTES} = longsToBigInt(%ag_bytes);
        $info{AG_WRITES} = longsToBigInt(%ag_writes);
        $info{AG_WBYTES} = longsToBigInt(%ag_wbytes);
        $info{AG_READS} = longsToBigInt(%ag_reads);
        $info{AG_RBYTES} = longsToBigInt(%ag_rbytes);
        $info{PER_CMDS} = longsToBigInt(%per_cmds);
        $info{PER_BYTES} = longsToBigInt(%per_bytes);
        $info{PER_WRITES} = longsToBigInt(%per_writes);
        $info{PER_WBYTES} = longsToBigInt(%per_wbytes);
        $info{PER_READS} = longsToBigInt(%per_reads);
        $info{PER_RBYTES} = longsToBigInt(%per_rbytes);
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a server statistics packet.\n");
    }

    return %info;
}


##############################################################################
# NAME:     _statsServers
#
# DESC:     Parses the server statistics packet and places the
#           information in a hash.
#
# INPUT:    scalar  $sequenceID         Sequence id
#           scalar  $recvPacket         Packet to parse
#
# Returns: Empty hash on error, else a hash with the following elements:
##############################################################################
sub _statsServers
{
    my ($self, $seq, $recvPacket) = @_;

    my %info;

    if (commandCode($recvPacket) == PI_STATS_SERVERS_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);
        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
            $info{COUNT},
            $rsvd
        ) = unpack("SS", $parts{DATA});

        my @statsServers;

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            my $start = 4 + (136 * $i);

            my %ag_cmds;
            my %ag_bytes;
            my %ag_writes;
            my %ag_wbytes;
            my %ag_reads;
            my %ag_rbytes;
            my %per_cmds;
            my %per_bytes;
            my %per_writes;
            my %per_wbytes;
            my %per_reads;
            my %per_rbytes;

            (
                $statsServers[$i]{SID},
                $rsvd,
                $rsvd,
                $statsServers[$i]{STATUS_MRP},
                $statsServers[$i]{LEN},
                $ag_cmds{LO_LONG}, $ag_cmds{HI_LONG},
                $ag_bytes{LO_LONG}, $ag_bytes{HI_LONG},
                $ag_writes{LO_LONG}, $ag_writes{HI_LONG},
                $ag_wbytes{LO_LONG}, $ag_wbytes{HI_LONG},
                $ag_reads{LO_LONG}, $ag_reads{HI_LONG},
                $ag_rbytes{LO_LONG}, $ag_rbytes{HI_LONG},
                $per_cmds{LO_LONG}, $per_cmds{HI_LONG},
                $per_bytes{LO_LONG}, $per_bytes{HI_LONG},
                $per_writes{LO_LONG}, $per_writes{HI_LONG},
                $per_wbytes{LO_LONG}, $per_wbytes{HI_LONG},
                $per_reads{LO_LONG}, $per_reads{HI_LONG},
                $per_rbytes{LO_LONG}, $per_rbytes{HI_LONG},
                $statsServers[$i]{FL_LIFCNT},
                $statsServers[$i]{FL_LSSCNT},
                $statsServers[$i]{FL_LSGCNT},
                $statsServers[$i]{FL_PSPEC},
                $statsServers[$i]{FL_IVTQC},
                $statsServers[$i]{FL_IVCRC},
                $statsServers[$i]{QDEPTH},
                $rsvd
            ) = unpack("SSa3CLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLSa2", substr($parts{DATA}, $start));

            # Now fixup all the 64 bit  numbers
            $statsServers[$i]{AG_CMDS} = longsToBigInt(%ag_cmds);
            $statsServers[$i]{AG_BYTES} = longsToBigInt(%ag_bytes);
            $statsServers[$i]{AG_WRITES} = longsToBigInt(%ag_writes);
            $statsServers[$i]{AG_WBYTES} = longsToBigInt(%ag_wbytes);
            
            $statsServers[$i]{AG_READS} = longsToBigInt(%ag_reads);
            $statsServers[$i]{AG_RBYTES} = longsToBigInt(%ag_rbytes);
            
            $statsServers[$i]{PER_CMDS} = longsToBigInt(%per_cmds);
            $statsServers[$i]{PER_BYTES} = longsToBigInt(%per_bytes);
            $statsServers[$i]{PER_WRITES} = longsToBigInt(%per_writes);
            $statsServers[$i]{PER_WBYTES} = longsToBigInt(%per_wbytes);
            $statsServers[$i]{PER_READS} = longsToBigInt(%per_reads);
            $statsServers[$i]{PER_RBYTES} = longsToBigInt(%per_rbytes);
        } 
        
        $info{STATSSERVERS} = [@statsServers];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a servers statistics packet.\n");
    }

    return %info;
}


##############################################################################
# NAME:     _statsHAB
#
# DESC:     Parses the HAB statistics packet and places the
#           information in a hash.
#
# INPUT:    scalar  $sequenceID         Sequence id
#           scalar  $recvPacket         Packet to parse
#
# Returns: Empty hash on error, else a hash with the following elements:
##############################################################################
sub _statsHAB
{
    my ($self, $seq, $recvPacket) = @_;

    my %info;

    if (commandCode($recvPacket) == PI_STATS_HAB_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;
        my %perCmds;
        my %avgReqSize;
        my %wrtReq;
        my %rdReq;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
            $rsvd,
            $info{STATUS_MRP},
            $info{LEN},
            $perCmds{LO_LONG}, $perCmds{HI_LONG},
            
            $info{QDEPTH},
            $rsvd,
            $avgReqSize{LO_LONG}, $avgReqSize{HI_LONG},

            $wrtReq{LO_LONG}, $wrtReq{HI_LONG},
            $rdReq{LO_LONG}, $rdReq{HI_LONG},
            
            $rsvd
        ) = unpack("a3CLLL Sa6LL LLLL a16", $parts{DATA});

        # Now fixup all the 64 bit numbers
        $info{PER_CMDS} = longsToBigInt(%perCmds);
        $info{AVG_REQ} = longsToBigInt(%avgReqSize);
        $info{WRT_REQ} = longsToBigInt(%wrtReq);
        $info{RD_REQ} = longsToBigInt(%rdReq);
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected an HAB statistics packet.\n");
    }

    return %info;
}

##############################################################################
# NAME:     _statsVDisk
#
# DESC:     Parses the virtual disk statistics packet and places the
#           information in a hash.
#
# INPUT:    scalar  $sequenceID         Sequence id
#           scalar  $recvPacket         Packet to parse
#
# Returns: Empty hash on error, else a hash with the following elements:
##############################################################################
sub _statsVDisk
{
    my ($self, $seq, $recvPacket) = @_;

    my %info;

    if (commandCode($recvPacket) == PI_STATS_VDISK_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $info{COUNT}
        ) = unpack("S", $parts{DATA});

        my @vdisks;

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            my $start = 4 + (120 * $i);

            my $rsvd;
            my %capacity;
            my %rreq;
            my %wreq;

            # Unpack the data
            (
            $rsvd,
            $vdisks[$i]{STATUS_MRP},
            $vdisks[$i]{LEN},
            $vdisks[$i]{VID},
            $vdisks[$i]{MIRROR},
            $vdisks[$i]{DEVSTAT},
            $vdisks[$i]{SCORVID},
            $vdisks[$i]{SCPCOMP},
            $vdisks[$i]{RAIDCNT},

            $capacity{LO_LONG}, $capacity{HI_LONG},
            $vdisks[$i]{ERROR},
            $vdisks[$i]{QD},

            $vdisks[$i]{RPS},
            $vdisks[$i]{AVGSC},
            $rreq{LO_LONG}, $rreq{HI_LONG},

            $wreq{LO_LONG}, $wreq{HI_LONG},
            $vdisks[$i]{ATTR},
            $vdisks[$i]{DRAIDCNT},
            $rsvd,

            $vdisks[$i]{SPRC},
            $vdisks[$i]{SPSC},
            $vdisks[$i]{SCHEAD},
            $vdisks[$i]{SCTAIL},

            $vdisks[$i]{CPSCMT},
            $vdisks[$i]{VLINKS},
            $vdisks[$i]{NAME},
            $vdisks[$i]{TIMESTAMP},
            $vdisks[$i]{LACCESS},
            $vdisks[$i]{AVERAGEIO},
            $vdisks[$i]{AVERAGESC}
            ) = unpack("a3CLSCCSCC LLLL LLLL LLSCa5 LLLL LLa16LLLL", substr($parts{DATA}, $start));

            $vdisks[$i]{CAPACITY} = longsToBigInt(%capacity);
            $vdisks[$i]{RREQ} = longsToBigInt(%rreq);
            $vdisks[$i]{WREQ} = longsToBigInt(%wreq);
        }

        $info{VDISKS} = [@vdisks];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual disk statistics packet.\n");
    }

    return %info;
}

##############################################################################
# Name: _getGoodFailStr
#
# Desc: Get the string FAIL or GOOD depending on the input value
#
# In:   Value to be checked (1 = FAIL, 0 = GOOD)
#
# Out:  String containing either FAIL or GOOD
##############################################################################
sub _getGoodFailStr
{
    my ($val) = @_;
    my $str;

    if ($val)
    {
        $str = "FAIL";
    }
    else
    {
        $str = "GOOD";
    }

    return $str;
}

##############################################################################
# Name:  _genericCmd2ResponsePacket
#
# Desc: Handles a response packet from a generic2 request.
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: hash with the follwing (empty if failure):
#               STATUS
#               ERROR_CODE
#               VERSION
#               COUNT
#               BUILDID
#               TIMESTAMP
#               DATA
#
##############################################################################
sub _genericCmd2ResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    my %rdData;

    logMsg("_genericCmd2ResponsePacket...begin\n");

    if (defined($recvPacket))
    {
        my %parts = disassembleXiotechPacket($recvPacket);
        my @ts;

        $rdData{STATUS} = $parts{STATUS};
        $rdData{ERROR_CODE} = $parts{ERROR_CODE};
        if(defined $parts{DATA} and length($parts{DATA}) >= 20) {
            ($rdData{VERSION},
             $rdData{COUNT},
             $rdData{BUILDID},
             @ts ) = unpack "A4A4A4SCCCCCC", $parts{DATA};
            $rdData{TIMESTAMP} = sprintf "%02X/%02X/%04X %02X:%02X:%02X",
            $ts[1],$ts[2],$ts[0],$ts[4],$ts[5],$ts[6];
            $rdData{DATA} = substr $parts{DATA}, 20;
        }
    }

    return %rdData;
}


##############################################################################
# Name: _getI2CMonitorStatusCodeString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub _getI2CMonitorStatusCodeString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_STATUS_CODE_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_STATUS_CODE_NOT_PRESENT )
    {
        $str = "Not_Present";
    }
    elsif( $val == I2C_MONITOR_STATUS_CODE_VALID )
    {
        $str = "Valid";
    }
    elsif( $val == I2C_MONITOR_STATUS_CODE_BUSY )
    {
        $str = "Busy";
    }
    elsif( $val == I2C_MONITOR_STATUS_CODE_NOT_READY )
    {
        $str = "Not_Ready";
    }
    elsif( $val == I2C_MONITOR_STATUS_CODE_ERROR )
    {
        $str = "Error";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: _getI2CMonitorTemperatureConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub _getI2CMonitorTemperatureConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_TEMPERATURE_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_TEMPERATURE_CONDITION_NORMAL )
    {
        $str = "Normal";
    }
    elsif( $val == I2C_MONITOR_TEMPERATURE_CONDITION_COLD )
    {
        $str = "Cold";
    }
    elsif( $val == I2C_MONITOR_TEMPERATURE_CONDITION_HOT )
    {
        $str = "Hot";
    }
    elsif( $val == I2C_MONITOR_TEMPERATURE_CONDITION_COLD_CRITICAL )
    {
        $str = "Cold Critical";
    }
    elsif( $val == I2C_MONITOR_TEMPERATURE_CONDITION_HOT_CRITICAL )
    {
        $str = "Hot Critical";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: _getI2CMonitorLimitMonitorString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub _getI2CMonitorLimitMonitorString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_LIMIT_MONITOR_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_LIMIT_MONITOR_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_LIMIT_MONITOR_TRIPPED )
    {
        $str = "Tripped";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: _getI2CMonitorPowerSupplyConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub _getI2CMonitorPowerSupplyConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_POWER_SUPPLY_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_POWER_SUPPLY_CONDITION_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_POWER_SUPPLY_CONDITION_HIGH_TEMPERATURE )
    {
        $str = "High Temperature";
    }
    elsif( $val == I2C_MONITOR_POWER_SUPPLY_CONDITION_DC_FAILED )
    {
        $str = "DC Failed";
    }
    elsif( $val == I2C_MONITOR_POWER_SUPPLY_CONDITION_AC_FAILED )
    {
        $str = "AC Failed";
    }
    elsif( $val == I2C_MONITOR_POWER_SUPPLY_CONDITION_INSERTED )
    {
        $str = "Inserted";
    }
    elsif( $val == I2C_MONITOR_POWER_SUPPLY_CONDITION_NOT_PRESENT )
    {
        $str = "Not Present";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: _getI2CMonitorCoolingFanConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub _getI2CMonitorCoolingFanConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_COOLING_FAN_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_COOLING_FAN_CONDITION_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_COOLING_FAN_CONDITION_FAILED )
    {
        $str = "Failed";
    }
    elsif( $val == I2C_MONITOR_COOLING_FAN_CONDITION_NOT_PRESENT )
    {
        $str = "Not Present";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: _getI2CMonitorProcessorResetConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub _getI2CMonitorProcessorResetConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_PROCESSOR_RESET_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_PROCESSOR_RESET_CONDITION_RUNNING )
    {
        $str = "Running";
    }
    elsif( $val == I2C_MONITOR_PROCESSOR_RESET_CONDITION_RESET )
    {
        $str = "Reset";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: _getI2CMonitorBatteryConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub _getI2CMonitorBatteryConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_BATTERY_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_BATTERY_CONDITION_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_BATTERY_CONDITION_LOW_CAPACITY )
    {
        $str = "Low Capacity";
    }
    elsif( $val == I2C_MONITOR_BATTERY_CONDITION_UNDER_VOLTAGE )
    {
        $str = "Under Voltage";
    }
    elsif( $val == I2C_MONITOR_BATTERY_CONDITION_OVER_VOLTAGE )
    {
        $str = "Over Voltage";
    }
    elsif( $val == I2C_MONITOR_BATTERY_CONDITION_NOT_PRESENT )
    {
        $str = "Not Present";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: _getI2CMonitorCurrentFlowConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub _getI2CMonitorCurrentFlowConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_CURRENT_FLOW_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_CURRENT_FLOW_CONDITION_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_CURRENT_FLOW_CONDITION_ABNORMAL )
    {
        $str = "Abnormal";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: _getI2CMonitorFuelGaugeConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub _getI2CMonitorFuelGaugeConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_FUEL_GAUGE_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_FUEL_GAUGE_CONDITION_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_FUEL_GAUGE_CONDITION_SHUTDOWN )
    {
        $str = "Shutdown";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: _getI2CMonitorMainRegulatorConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub _getI2CMonitorMainRegulatorConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_MAIN_REGULATOR_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_MAIN_REGULATOR_CONDITION_OPERATIONAL )
    {
        $str = "Operational";
    }
    elsif( $val == I2C_MONITOR_MAIN_REGULATOR_CONDITION_SHUTDOWN_ERROR )
    {
        $str = "Shutdown-Error";
    }
    elsif( $val == I2C_MONITOR_MAIN_REGULATOR_CONDITION_SHUTDOWN_GOOD )
    {
        $str = "Shutdown-Good";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: _getI2CMonitorChargerConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub _getI2CMonitorChargerConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_CHARGER_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_CHARGER_CONDITION_IDLE )
    {
        $str = "Idle";
    }
    elsif( $val == I2C_MONITOR_CHARGER_CONDITION_TRICKLE )
    {
        $str = "Trickle";
    }
    elsif( $val == I2C_MONITOR_CHARGER_CONDITION_BULK )
    {
        $str = "Bulk";
    }
    elsif( $val == I2C_MONITOR_CHARGER_CONDITION_OVER )
    {
        $str = "Over";
    }
    elsif( $val == I2C_MONITOR_CHARGER_CONDITION_TOPOFF )
    {
        $str = "Topoff";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: _getI2CMonitorNVRAMBatteryConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub _getI2CMonitorNVRAMBatteryConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_NVRAM_BATTERY_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_NVRAM_BATTERY_CONDITION_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_NVRAM_BATTERY_CONDITION_FAILED )
    {
        $str = "Failed";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: _getI2CMonitorEEPROMConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub _getI2CMonitorEEPROMConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_EEPROM_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_EEPROM_CONDITION_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_EEPROM_CONDITION_BAD_CRC )
    {
        $str = "Bad_CRC";
    }
    elsif( $val == I2C_MONITOR_EEPROM_CONDITION_NOT_READABLE )
    {
        $str = "Not_Readable";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: _getI2CMonitorXCIDataString
#
# Desc: Get the string, depending on the input value
#
# In:   Binary XCI data
#
# Out:  String
##############################################################################
sub _getI2CMonitorXCIDataString
{
    my ($xciArray) = @_;
    my $msg = "";
    my $string = "";
    my @stuff;

    @stuff = unpack( "C8", substr($xciArray, 0, 8) );
    $msg .= sprintf( "      manuJedecId:                          [%02x %02x %02x %02x %02x %02x %02x %02x]\n",
        $stuff[0], $stuff[1], $stuff[2], $stuff[3], $stuff[4], $stuff[5], $stuff[6], $stuff[7] );

    @stuff = unpack( "C1", substr($xciArray, 8, 1) );
    $msg .= sprintf( "      manuLocation:                         [%02x]\n", $stuff[0] );

    $string = unpack( "a7", substr($xciArray, 9, 7) );
    $string =~ s/[^ -~]/\?/g;
    $msg .= sprintf( "      modulePartNumber:                     [%s]\n", $string );

    $string = unpack( "a4", substr($xciArray, 16, 4) );
    $string =~ s/[^ -~]/\?/g;
    $msg .= sprintf( "      moduleDashNumber:                     [%s]\n", $string );

    $string = unpack( "a2", substr($xciArray, 20, 2) );
    $string =~ s/[^ -~]/\?/g;
    $msg .= sprintf( "      moduleRevisionLetters:                [%s]\n", $string );

    @stuff = unpack( "C5", substr($xciArray, 22, 5) );
    $msg .= sprintf( "      reserved-1:                           [%02x %02x %02x %02x %02x]\n",
        $stuff[0], $stuff[1], $stuff[2], $stuff[3], $stuff[4] );

    @stuff = unpack( "C2", substr($xciArray, 27, 2) );
    $msg .= sprintf( "      revisionCode:                         [%02x %02x]\n",
        $stuff[0], $stuff[1] );

    @stuff = unpack( "C1", substr($xciArray, 29, 1) );
    $msg .= sprintf( "      manuYear:                             [%02x]\n", $stuff[0] );

    @stuff = unpack( "C1", substr($xciArray, 30, 1) );
    $msg .= sprintf( "      manuWeek:                             [%02x]\n", $stuff[0] );

    @stuff = unpack( "C4", substr($xciArray, 31, 4) );
    $msg .= sprintf( "      asmSerialNumber:                      [%02x %02x %02x %02x]\n",
        $stuff[0], $stuff[1], $stuff[2], $stuff[3] );

    @stuff = unpack( "C23", substr($xciArray, 35, 23) );
    $msg .= sprintf( "      reserved-2:                           [%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x]\n",
        $stuff[0],  $stuff[1],  $stuff[2],  $stuff[3],  $stuff[4],
        $stuff[5],  $stuff[6],  $stuff[7],  $stuff[8],  $stuff[9],
        $stuff[10], $stuff[11], $stuff[12], $stuff[13], $stuff[14],
        $stuff[15], $stuff[16], $stuff[17], $stuff[18], $stuff[19],
        $stuff[20], $stuff[21], $stuff[22] );

    @stuff = unpack( "C4", substr($xciArray, 58, 4) );
    $msg .= sprintf( "      crc:                                  [%02x %02x %02x %02x]\n",
        $stuff[0], $stuff[1], $stuff[2], $stuff[3] );

    @stuff = unpack( "C2", substr($xciArray, 62, 2) );
    $msg .= sprintf( "      vendorSpecific:                       [%02x %02x]",
        $stuff[0], $stuff[1] );

    return $msg;
}


##############################################################################
# Name: _getI2CMonitorEventFlagsString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub _getI2CMonitorEventFlagsString
{
    my ($val) = @_;
    my $str;

    if( $val == 0 )
    {
        $str = "None";
    }
    else
    {
        $str = "";

        if( $val & (1 << 0) )
        {
            $str = "$str-Info ";
        }

        if( $val & (1 << 4) )
        {
            $str = "$str-Info+ ";
        }

        if( $val & (1 << 1) )
        {
            $str = "$str-Warning ";
        }

        if( $val & (1 << 5) )
        {
            $str = "$str-Warning+ ";
        }

        if( $val & (1 << 2) )
        {
            $str = "$str-Error ";
        }

        if( $val & (1 << 6) )
        {
            $str = "$str-Error+ ";
        }

        if( $val & (1 << 3) )
        {
            $str = "$str-Debug ";
        }

        if( $val & (1 << 7) )
        {
            $str = "$str-Debug+ ";
        }
    }

    return $str;
}


##############################################################################
# Name: _getI2CMonitorDeviceFlagsString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub _getI2CMonitorDeviceFlagsString
{
    my ($val) = @_;
    my $str;

    if( $val == 0 )
    {
        $str = "None";
    }
    else
    {
        $str = "";

        if( $val & (1 << 0) )
        {
            $str = "$str-Init ";
        }

        if( $val & (1 << 1) )
        {
            $str = "$str-Busy ";
        }

        if( $val & (1 << 2) )
        {
            $str = "$str-FRC ";
        }

        if( $val & (1 << 3) )
        {
            $str = "$str-RVal ";
        }

        if( $val & (1 << 4) )
        {
            $str = "$str-Psnt ";
        }

        if( $val & (1 << 7) )
        {
            $str = "$str-Mal ";
        }

        if( $val & 0x60 )
        {
            $str = "$str-Rsvd";
        }
    }
    return $str;
}

##############################################################################
# Name:     _envCtrlAndBayPacket
#
# Desc:     Parses the controller and disk bay info packet and place the 
#           information in a hash
#
# In:       scalar  $packet to be parsed 
#
# Returns:
#
##############################################################################
sub _envCtrlAndBayPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_ENVIRO_DATA_CTRL_AND_BAY_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);
        
        # The data portion of the packet is idential to the X1 version.
        # Call a common function that handles both.
        %info = _parseX1GetEnviro(%parts);
        
        # Handle the status and error code values        
        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected the PI version of the X1 Environmental packet\n");
    }

    return %info;
}


1;
##############################################################################
# Change log:
# $Log$
# Revision 1.4  2006/12/06 16:06:37  BharadwajS
# TBolt00017136 Committing back the stats vdisk changes
#
# Revision 1.3  2006/12/06 00:26:12  BharadwajS
# TBolt00017136 Updating the STATS_VDISK_CMD
#
# Revision 1.2  2006/07/17 20:38:32  RustadM
# TBolt00014770
# Move 750 branch onto main.
#
# Revision 1.1.1.1.30.4  2006/06/06 16:51:41  HoltyB
# TBolt00000000:Changed to monitor single PS status
#
# Revision 1.1.1.1.30.3  2006/05/31 18:28:04  HoltyB
# TBolt00000000:More changes to hw monitor
#
# Revision 1.1.1.1.30.2  2006/05/15 17:01:28  HoltyB
# TBolt00000000:Added environmental PI interface/async events/validation
#
# Revision 1.1.1.1.30.1  2006/04/26 09:06:20  BharadwajS
# CCBE changes for PI Versioning
#
# Revision 1.1.1.1  2005/05/04 18:53:54  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.52  2004/12/16 19:12:02  SchibillaM
# TBolt00011891: Implement PI_StatsServers - stats for all valid servers on a  controller.
# Reviewed by Chris.
#
# Revision 1.51  2004/08/25 16:04:05  McmasterM
# TBolt00011106: Add interface to CCB and CCBE to test MicroMemory interface
# Added commandCode and decoder.  Reviewed by Randy Rysavy.
#
# Revision 1.50  2004/08/24 14:00:00  McmasterM
# TBolt00011106: Add interface to CCB and CCBE to test MicroMemory interface
# Done.  Reviewed by Randy Rysavy.
#
# Revision 1.49  2004/07/27 22:05:46  WaggieL
# TBolt00010362:  Handle new GPIOD field in STATSLOOP response.  Also tweaked to not print targets header for BE ports (which have no targets).
# Rev. by Mike Hicken
#
# Revision 1.48  2004/07/27 12:13:26  SchibillaM
# TBolt00010893: Add support for X1 environmental packet in PI interface.
# Reviewed by Chris.
#
# Revision 1.47  2004/06/25 13:50:22  SchibillaM
# TBolt00010632: Add PI and X1 support for HAB Stats.  Reviewed by Chris.
#
# Revision 1.46  2003/12/30 15:35:55  NigburC
# TBolt00000000 - Changed the II status field to be displayed in hex when
# displaying both FE and BE information.
#
# Revision 1.45  2003/10/15 15:25:55  HoweS
# Tbolt00009415 - Added Server with stats option to target resource list.
# Reviewed by Chris.
#
# Revision 1.44  2003/09/17 15:59:39  McmasterM
# Changed statsenv display to show hex data for manufacturing.
#
# Revision 1.43  2003/09/11 15:50:56  SchibillaM
# TBolt00009161: Add support for option input parm to PI_STATS_LOOPS_REQ.  This
# is required to be able to get error counter info from the command.  Reviewed by Randy.
#
# Revision 1.42  2003/08/06 12:29:12  NigburC
# TBolt00007744 - Converted the create/delete server and II packets to use
# the MR_Defs.h definitions.  Found a difference in the II structures between
# PROC ii.inc and OS_II.h.  The reserved bytes near the end of the structure
# were commented as 4 in assembly but really reserved 8.  This was changed
# to reserve only 4.
# Reviewed by Mark Schibilla.
#
# Revision 1.41  2003/06/26 16:01:16  NigburC
# TBolt00008620 - Updated the STATS_CACHE_DEV(s) structure so it has
# the updated fields that the PROC added.  This also required the CCBE code
# to change to parse those new fields and display them.
# Reviewed by Craig "Boxcar" Menning.
#
# Revision 1.40  2003/06/03 19:46:18  MenningC
# TBOLT00000000: Changed many of the 'display' functions to fill a string rather than print to the screen. This supports use by the test scripts. Reviewed by Randy R.
#
# Revision 1.39  2003/05/12 18:25:04  McmasterM
# TBolt00008217: Temperature Log Events incorrect for ProcA/B and Bay
# Changed temperature events to look identical to those on MAG, and added
# the COLD_CRITICAL temperature range (type ERROR).  Made some other
# changes to make monitor log events look like other events.
# Portions reviewed by Tim Swatosh
#
# Revision 1.38  2003/05/08 15:32:01  MenningC
# Tbolt00000000: Changed the print method for statsenv. Rather than directly printing, prints to a string and the caller prints the string.
#
# Revision 1.37  2003/05/05 21:33:50  TeskeJ
# tbolt00008227 - scrubbing changes
# rev by Bryan
#
# Revision 1.36  2003/04/28 20:27:21  McmasterM
# TBolt00007376: Buffer battery "NOT_PRESENT" log message repeated
# TBolt00007097: When one PS is powercycled twice, the DC_FAILED only appears once
# I2C monitor changes to improve log message accuracy and behavior.
#
# Revision 1.35  2003/04/24 20:07:16  TeskeJ
# tbolt00008158 - added devstat and raidcount to vdisks display
# rev by Jeff
#
# Revision 1.34  2003/02/17 19:30:02  McmasterM
# TBolt00000000: Display temperature as a signed value.
#
# Revision 1.33  2003/01/16 21:44:06  SchibillaM
# TBolt00006514: Add support for X1 Electrical Signatures and version in X1 Config.
#
# Revision 1.32  2003/01/15 14:51:12  HoltyB
# TBolt00000000: Changes for new loopstats.
#
# Revision 1.31  2003/01/15 06:00:31  McmasterM
# TBolt00006728: Power supply failures no longer appear in logs
# TBolt00006673: Create startup hardware report (partial completion)
# Made changes to be more verbose on the monitor log messages.
#
# Revision 1.30  2003/01/07 20:17:11  McmasterM
# TBolt00006501: Add XCI gathering of PS Interface board data
# TBolt00006492: I2C monitor asserting error on boards without PCA9548 switch
#
# Revision 1.29  2002/12/18 22:56:37  NigburC
# TBolt00000000 -- Updated the statsproc to parse and display the processor
# utilization value.
# Reviewed by Jim Snead.
#
# Revision 1.28  2002/12/17 23:36:51  McmasterM
# TBolt00006250: Add support for I2C switch device
# TBolt00006251: Add support for new I2C EEPROMs (component info collection)
# Full switch support and nearly all of the EEPROM support is in place.
#
# Revision 1.27  2002/12/11 21:51:43  NigburC
# TBolt00006456 - Fixed the parsing of the stats vdisk packet, it did not get
# updated with the changes made to the VDEV_OUT packet.
# Reviewed by Craig Menning.
#
# Revision 1.26  2002/10/30 15:08:19  McmasterM
# TBolt00006241: Add support for LM92 I2C device
#
# Revision 1.25  2002/10/11 14:03:39  SchibillaM
# TBolt00006165.  Fixed perl code to worked with change to PI_VDEV_OUT.
# This struct now has rid[0] instead of rid[1]
# Reviewed by Chris.
#
# Revision 1.24  2002/10/01 19:01:27  RysavyR
# TBolt00006013:  Add the ability to handle and process BF style packets on
# the X1 port. Reviewed by TimSw.
#
# Revision 1.23  2002/09/04 14:01:11  SchibillaM
# TBolt00005858: Error introduced when ENVSTATS was removed.  Function
# displayEnvironmentalStatsExtended was misspelled here but spelled
# correctly in regression_single.pl causing it to fail.
#
# Revision 1.22  2002/08/27 21:43:36  McmasterM
# TBolt00005893: Every time Bigfoot is shutdown logs reflect validation errors
# Validation errors are now changed to warnings, and vcg shutdown no longer
# causes the validation to fail.
# Reviewed by Randy Rysavy
#
# Revision 1.21  2002/08/20 18:53:42  NigburC
# TBolt00005709 - Added the state field to the statsloop output.
# Reviewed by Jeff Williams.
#
# Revision 1.20  2002/07/29 19:52:46  McmasterM
# TBolt00002740: CCB Zion I2C support - SDIMM battery (partial completion)
#
# Revision 1.19  2002/07/02 14:30:21  McmasterM
# TBolt00002740: CCB Zion I2C support - SDIMM battery (partial completion)
# The code is gathering new environmental data, but it is not yet connected to
# any async events.  These changes impact both boot and runtime code.
# Reviewed by Randy Rysavy
#
# Revision 1.18  2002/07/01 20:15:15  SchibillaM
# TBolt00004786: Implement BE & FE Loop Stats.  Stats for all valid port are
# returned in one big response packed.  This is a redefinition of the Loop Stats
# response and requires coordinated CCB, CCBE and UMC code.
# TBolt00004481: Implement BE Device Paths.  Some files in this check in
# also include changes for BE Device paths.
# Review by Randy Rysavy (C, Perl),  Chris (Java)
#
# Revision 1.17  2002/06/29 14:24:08  HoltyB
# TBolt00005018: Fixed SERVERCREATE, made up to date with current input structure
# Added support for new environmental data
#
# Revision 1.16  2002/04/23 19:17:21  SchibillaM
# TBolt00003921: Rearrange command codes.
# Reviewed by Chris.
#
# Revision 1.15  2002/04/23 19:16:06  SchibillaM
# TBolt00003921: Rearrange command codes.
# Reviewed by Chris.
#
# Revision 1.14  2002/04/12 16:22:36  NigburC
# TBolt00003265 - The cache device info structure changed to include
# capacity and 8 bytes of reserved information.  The MRP structure and the
# CCBE code had to be updated to handle this change.
#
# Revision 1.13  2002/04/01 16:46:09  HoltyB
# Added fw level to stats loop
#
# Revision 1.12  2002/02/26 20:52:11  NigburC
# TBolt00000159 - Added additional code to start retrieving statistics information
# in bulk.
#
# Also added in the new PI_PDISKS command to retrieve information for all
# pdisks in one PI request (multiple MRPs).
#
# Revision 1.11  2002/02/11 19:08:03  HoltyB
# moved command codes to PI_CommandCodes.pm
#
# Revision 1.10  2001/12/11 20:16:24  NigburC
# Added the CHANNEL parameter to STATSLOOP.
# Fixed the parsing of the STATSPCI packet to match what is returned from
# the MRP.
#
# Revision 1.9  2001/12/10 21:04:34  NigburC
# Added the following commands:
# STATSCACHEDEV
# STATSLOOP
# STATSPCI
# STATSPROC
# STATSSERVER
#
# Revision 1.8  2001/11/27 20:21:21  RysavyR
# Added the ability to reset the CCB via the "generic" command handler.
#
# Revision 1.7  2001/11/26 19:27:46  RysavyR
# Added the GETREPORT command with only the heap statistics/memory leak
# report supported right now..
#
# Revision 1.6  2001/11/15 17:04:21  NigburC
# Made sure logInfo and envStats added the STATUS and ERROR_CODE
# fields to the hash returned.
#
# Revision 1.5  2001/11/15 13:56:39  SwatoshT
# Added support for environmental statistics.
#
# Revision 1.4  2001/11/14 13:03:04  NigburC
# Modified verify parameters to return a hash instead of just the message.
# This will allow the code to return it to the command line where it will be
# processed like any other error, except there is a MESSAGE in the
# hash which will be displayed describing the parameter error that occurred.
#
# Revision 1.3  2001/11/07 22:19:06  NigburC
# Removed the code for the 3007 port.
#
# Revision 1.2  2001/11/07 13:51:13  NigburC
# Change the usage of the ID tag to HEADER tag.
#
# Revision 1.1  2001/11/05 19:48:53  NigburC
# Moved the environmental statatistics code to another PM file to help keep
# things organized.  These functions were removed from cmdMgr.pm.
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
