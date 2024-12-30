The SystemStatusChecker is designed primarily for use within the UpgradePackage scripts 
which are applied via the ICON.


The SystemStatusChecker requires the following:
	1) Ewok-Dsc*.xml file to read all of the DSC IP addresses 
           (available on the ICON in /var/xiotech/XioWebService
        2) DSC ID passed into the script (to limit which IPS)
        3) OpErrors.txt file which holds errors to give the user
        4) OpStatus.txt file which holds status messages for the user
        5) CCBCL must be accessible in order to run the script

This is also currently only for use within upgrade packages, and therefore the 
OpErrors.txt that it emits are upgrade-specific. This can be modified later if needed,
however, all scripts that use this SystemStatusChecker will also need modification.

The SystemStatusChecker script can be run from within other scripts (like ISE Updater, Switch updater, etc) provided it has CCBE available, OR it can be run stand-alone in it's own XZP (build using "make" from this directory)


