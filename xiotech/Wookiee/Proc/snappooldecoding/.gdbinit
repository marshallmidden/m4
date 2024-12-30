# Start of file .gdbinit
#
# NOTE: Blank lines in macros may cause unexpected things to print.
#
# set $RELEASE = 730
# set $RELEASE = 830
# set $RELEASE = 840
set $RELEASE = 860
#
set history filename ~/.gdb_history
set history save on
set history size 256
#-----------------------------------------------------------------------------
# The "pp" command sets the print pretty option without typing it out.
define pp
    set print pretty $arg0
end
#...
document pp
The "pp" command will set the print pretty option.
    "ON"    - Turn the print pretty ON
    "OFF"   - Turn the print pretty OFF
end
#-----------------------------------------------------------------------------
# This macro prints the vdisks information given the specified format.
define i_vdisks
    set $DSPTYPE = $arg0
    if ($DSPTYPE == 0)
        printf "  VID  Status        NAME        RID\n"
        printf "  ---  ------  ----------------  ---\n"
    end
    if ($DSPTYPE == 1)
        printf " VID  DEVSTAT     CAPACITY     MIRROR   ATTR   RAIDCNT  DRAIDCNT  PCT COMP  OWNER    NAME\n"
        printf " ---  -------  --------------  ------  ------  -------  --------  --------  -----  --------\n"
    end
    set $I = 0
    while ($I < $MAXVIRTUALS)
        if ((*(VDX*)&gVDX).vdd[$I] > 0)
            if ($DSPTYPE == 0)
                printf " %3hu",         gVDX.vdd[$I].vid
                printf "     0x%02x",   gVDX.vdd[$I].status
                printf "  %16s",        gVDX.vdd[$I].name
                set $J = 0
                set $RDD = gVDX.vdd[$I]->pRDD
                printf "  "
                while ($RDD > 0)
                    if ($J > 0)
                        printf ","
                    end
                    printf "%hu", $RDD->rid
                    set $J = $J + 1
                    set $RDD = $RDD->pNRDD
                end
            end
            if ($DSPTYPE == 1)
                printf " %3hu",         gVDX.vdd[$I].vid
                printf "     0x%02x",   gVDX.vdd[$I].status
                printf "  %14u",        gVDX.vdd[$I].devCap
                printf "    0x%2.2x",   gVDX.vdd[$I].mirror
                printf "  0x%4.4x",     gVDX.vdd[$I].attr
                printf "  %7u",         gVDX.vdd[$I].raidCnt
                printf "  %8u",         gVDX.vdd[$I].draidCnt
                printf "  %8u",         gVDX.vdd[$I].scpComp
                printf "    0x%2.2x",   gVDX.vdd[$I].owner
                printf "  %.16s",       gVDX.vdd[$I].name
            end
            printf "\n"
        end
        set $I = $I + 1
    end
end
#...
document i_vdisks
Print the vdisks information given the specified format.
    0 - VDisk status information
    1 - VDisk standard information
end
#-----------------------------------------------------------------------------
# This macro prints the vdisks status information.
define vdiskstatus
    i_vdisks 0
end
#...
document vdiskstatus
Print the vdisks information in short format (vdisks 0).
end
#-----------------------------------------------------------------------------
# This macro prints the vdisks in standard format.
define vdiskstd
    i_vdisks 1
end
#...
document vdiskstd
Print the vdisks information in standard format (vdisks 1).
end
#-----------------------------------------------------------------------------
# This macro prints the list of available vdisks.
define vdisklist
    printf "Virtual Disk List:\n"
    printf "------------------\n"
    set $I = 0
    while ($I < $MAXVIRTUALS)
        if ((*(VDX*)&gVDX).vdd[$I] > 0)
            printf " %3d    0x%8.8x\n", $I, gVDX.vdd[$I]
        end
        set $I = $I + 1
    end
    printf "\n"
end
#...
document vdisklist
Prints the list of available vdisks.
end
#-----------------------------------------------------------------------------
# This macro prints the vdisk information for a given vdisk.
define vdiskinfo
    if ((*(VDX*)&gVDX).vdd[$arg0] != 0)
        printf "Virtual Disk Information (%d):\n", $arg0
        print (*(VDX*)&gVDX).vdd[$arg0]
        set $pVDD = (VDD*)gVDX.vdd[$arg0]
        set $pRDD = $pVDD->pRDD
        print *$pVDD
        printf "  RID  STATUS  ASTATUS\n"
        printf "  ---  ------  -------\n"
        set $I = 0
        while ($I < $pVDD->raidCnt)
            printf "  %3hu",        $pRDD->rid
            printf "    0x%2.2x",   $pRDD->status
            printf "     0x%2.2x",  $pRDD->aStatus
            printf "\n"
            set $pRDD = $pRDD->pNRDD
            set $I = $I + 1
        end
    else
        printf "Virtual Disk Information (%d): NOT AVAILABLE\n", $arg0
    end
end
#...
document vdiskinfo
Print the vdisk information for a given vdisk (argument VID).
    VID - Virtual Disk Identifier
end
#=============================================================================
define exit
  quit
end
#=============================================================================
####
## Modelines:
## Local Variables:
## tab-width: 4
## indent-tabs-mode: nil
## End:
## vi:sw=4 ts=4 expandtab
# End of file .gdbinit
