#!/usr/bin/python3
# ----------------------------------------------------------------------------
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
# ----------------------------------------------------------------------------
# The argparse routine prints following triple quote as part of help message.
'''
Script used by upgrade.py to check and fix the EFI boot variables of a system.
EFI1 and EFI2 partitions need to do the same thing. EFI2 is backup for EFI1.
    "fixefivars.py -q"      No output, run commands to add/fix missing.
 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
BootOrder should be EFI1 and EFI2, then NSHELL, then rest of kept ones.
Want:
   AlmaLinux HD(1,GPT,a4858da5-38b0-4282-96fd-e6384b84d135,0x800,0xfa000)/File(\\EFI\\redhat\\shimx64.EFI)
   AlmaLinux HD(8,GPT,e09b5a60-69c3-4f67-961c-6c3df25b1de0,0x6fa22800,0xfa000)/File(\\EFI\\redhat\\shimx64.EFI)
 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
Verify that everything is okay, and if not suggest or do fixes.
    1) Grab "CurrentBoot", and check "BootOrder" has it first.
    2) Get all the "BootXXXX[*]" and find those that should be deleted.
    3) Make sure CurrentBoot and the BootXXXX for it has the '*'.
    4) See keep_patterns. (Things that should be kept.)
    5) See delete_patterns. (Things that do not need to be present.)
    6) Check CurrentBoot, and if different first element in BootOrder... .
    7) If only one of EFI1 or EFI2 is present, create the missing one.
        If both are missing, do nothing -- because -- how can we have booted?
    8) Determine boot order.
 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
Commands and directories used:
    "efibootmgr -v"             # For current boot order, possibilities, etc.
        List in detail the EFI Boot variables used in booting.
    "mount | egrep ' / | /etc/root2 | /boot/efi | /b_boot/efi | EFI[12] '"
        /dev/md120 on / type xfs (rw,relatime,attr2,inode64,logbufs=8,logbsize=32k,noquota)
        /dev/sdf1 on /b_boot/efi type vfat (rw,relatime,...,errors=remount-ro)
        /dev/sde1 on /boot/efi type vfat (rw,relatime,...,errors=remount-ro)
        /dev/md125 on /etc/root2 type xfs (rw,relatime,attr2,inode64,logbufs=8,logbsize=32k,noquota)
    "egrep ' root[12] | EFI[12] ' /etc/fstab"
        LABEL="root2"  /            xfs  defaults                                        0 0
        LABEL="root1"  /etc/root2   xfs  defaults                                        0 0
        LABEL="EFI1"   /boot/efi    vfat defaults,uid=0,gid=0,umask=0077,shortname=winnt 0 0
        LABEL="EFI2"   /b_boot/efi  vfat defaults,uid=0,gid=0,umask=0077,shortname=winnt 0 0
    /dev/disk/by-partuuid       # For disk partition UUIDs.
        PARTUUID is a partition-table-level UUID for the partition, a standard
        feature for all partitions on GPT-partitioned disks. It is retrieved
        from the partition table, it does not make any assumptions about the
        actual contents of the partition (such as file system type).
        Example: ... e09b5a60-69c3-4f67-961c-6c3df25b1de0 -> ../../sde1
                 ... a4858da5-38b0-4282-96fd-e6384b84d135 -> ../../sdf1
    /dev/disk/by-label          # Used to get EFI1 and EFI2 disk partitions.
        This is file system label. It may point to a disk partition, software
        raid, logical volume manager name, or multipath device name.
        Example: ... EFI1 -> ../../sde1
                 ... EFI2 -> ../../sdf1
 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
'''
# ----------------------------------------------------------------------------
# Modules to include for program.
import sys                          # System definitions (stderr, etc.).
import os                           # O/S routines - directory paths ...
import argparse                     # Command argument parsing methodology.
import re                           # Regular expression matching/parsing.
import subprocess

# ----------------------------------------------------------------------------
def main():
    ''' Main program, everything else is nested.  '''
    # ----------------------------------------------------------------------------
    delete_patterns = [
        r'\(Bus \d+ Dev \d+\)PCI RAID Adapter',
        r'CentOS',
        r'CentOS Linux',
        r'EFI Fixed Disk Boot Device \d+',
        r'EFI Floppy.*',
        r'EFI Network.*$',
        r'EFI Virtual disk.*$',
        r'Hard Drive',
        r'IBA GE Slot \d+ v\d+',
        r'IBA GE Slot \d+ v\d+',
        r'IBA GE Slot \d+ v\d+',
        r'Network Card',
        r'P\d+:',
        r'Red Hat Enterprise Linux 6',
        r'ubuntu',
        r'UEFI: \(IP4 \)*Intel\(R\) I\d+ Gigabit Network Connection',
        r'UEFI OS',
    ]

    keep_patterns = [
        r'EFI Internal Shell \(Unsupported option\)',
        r'EFI VMware Virtual SATA CDROM Drive \([0-9,]+\)',
        r'UEFI: Built-in EFI Shell',
    ]

    # These are to be kept, but need verifying, and possibly changing later.
    verify_or_fix_patterns = [
        r'AlmaLinux',
        r'Red Hat Enterprise Linux',
        r'RedHat Boot Manager',
    ]

    # ----------------------------------------------------------------------------
    efibootmgr = '/usr/sbin/efibootmgr'

    errors_occurred = 0
    warnings_occurred = 0

    etc_fstab = '/etc/fstab'
    cmd_egrep = '/usr/bin/egrep'
    dev_disk_by_label = '/dev/disk/by-label'
    dev_disk_by_partuuid = '/dev/disk/by-partuuid'
    # ----------------------------------------------------------------------------
    # Routines start.
    # ----------------------------------------------------------------------------
    def parse_args():
        nonlocal args

        parser = argparse.ArgumentParser(description=__doc__,
                                         formatter_class=argparse.RawDescriptionHelpFormatter,
                                         epilog='Note: if this script messes up, your system may/will NOT boot.')
        parser.add_argument('--quiet', '--quit', '--qui', '--qu', '--q', '-q',
                            dest='quiet', action='store_true',
                            help="No output as this script runs.")
        parser.add_argument('--dry-run', '--dry-ru', '--dry-r', '--dry-', '--dry', '--dr',
                            '--dryrun', '--dryru', '--dryr', 
                            dest='dryrun', action='store_true',
                            help="Dry run, do not execute commands to change anything.")
        parser.add_argument('--delete', '--delet', '--dele', '--del', '--de', '-d',
                            dest='delete', action='store_true',
                            help='''
                            Print out commands to delete unneeded variables and to correct EFI Boot order, etc.
                            This does NOT execute the commands.
                            You need to execute them, then verify.
                            To verify use "efibootmgr -v" before AND after reboot.
                            NOTE: some systems do not work right.
                            VMware adds things it wants in and doubles/triples and it looks horrible.
                            But, things keep working.
                            iDRAC is strange in it's own way too.
                            '''
                            )
        parser.add_argument('--verbose', '--verbos', '--verbo', '--verb', '--ver',
                            '--ve', '--v', '-v',
                            dest='verbose', action='store_true',
                            help="Verbose output as this script's parsing proceeds.")
        args = parser.parse_args()

        if args.verbose:                # Verbose overrides quiet.
            args.quiet = False
        # fi
    # End of parse_args

    # ----------------------------------------------------------------------------
    def errmsg(line):
        nonlocal args
        nonlocal errors_occurred

        if not args.quiet:
            print("# ERROR: {}".format(line), file=sys.stderr, flush=True)
            errors_occurred += 1
        # fi
    # End of errmsg

    # ----------------------------------------------------------------------------
    def warnmsg(line):
        nonlocal warnings_occurred
        nonlocal args

        if not args.quiet:
            print("# WARNING: {}".format(line), file=sys.stderr, flush=True)
            warnings_occurred += 1
        # fi
    # End of warnmsg

    # ----------------------------------------------------------------------------
    def debugmsg(line):
        nonlocal args

        if args.verbose:
            print("# DEBUG: {}".format(line), file=sys.stderr, flush=True)
        # fi
    # End of debugmsg

    # ----------------------------------------------------------------------------
    def mount_efi_boot_dirs():
        # Mount /boot/efi and /b_boot/efi -- EFI partitions.
        # Ignore errors. (Like mounted already, or failure -- it does not differentiate.)
        for efi in ['/boot/efi', '/b_boot/efi']:
            try:
                cmd = '/usr/bin/mount {} || true'.format(efi)
                out = subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True, timeout=3, universal_newlines=True).strip()
                debugmsg("out='{}'".format(out))
            except (Exception) as ex:
                errmsg(ex)
            # yrt
        # rof
    # End of mount_efi_boot_dirs

    # ----------------------------------------------------------------------------
    def parse_efibootmgr_v():
        nonlocal efibootmgr, delete_patterns, keep_patterns, verify_or_fix_patterns
        nonlocal delete_commands, inactive_commands, activate_commands
        nonlocal bootcurrent, bootorder, rest_order
        nonlocal keepefi, verify_or_fix_efi

        # Information from the efibootmgr command -- Get EFI BootXXXX variables.
        cmd_efibootmgr = '{} -v'.format(efibootmgr)
        try:
            efibootmgr_output = subprocess.check_output(cmd_efibootmgr, shell=True, universal_newlines=True).strip()
        except:
            errmsg('Error processing command "{}".'.format(cmd_efibootmgr))
            sys.exit(1)
        # yrt
        if efibootmgr_output is None or str(efibootmgr_output) == '':
            errmsg('No output from command "{}".'.format(cmd_efibootmgr))
            sys.exit(1)
        # fi
        lines = re.split('\n', efibootmgr_output)      # Break output on two newlines in a row.

        # Parse output into dictionary, key based on number, and value of [NAME,value].
        # bootcurrent = None
        order = bootorder.copy()
        debugmsg("ORDER='{}'  BOOTORDER='{}'".format(order,bootorder))
        for l in lines:
            debugmsg("    {}".format(l))
            c = re.split(r'\s+', l, maxsplit=1)
            if len(c) == 1:
                errmsg('Ignoring line with only one part to output line: "{}".'.format(c))
                continue
            # fi
            boottype = c[0]
            if boottype == 'BootCurrent:':
                bootcurrent = c[1]
            elif boottype == 'Timeout:':
                pass                                    # Ignore this line.
            elif boottype == 'BootOrder:':
                bootorder = re.split(r',', c[1])
                order = bootorder.copy()                # Remove items as found and verified.
                if bootcurrent is None:
                    warnmsg("Currently booted entry does not exist before BootOrder '{}'".format(order))
                    continue
                # fi
                if bootcurrent != order[0]:
                    warnmsg("Currently booted entry ({}) is not first in BootOrder '{}'".format(bootcurrent, order))
                    continue
                # fi
            elif len(boottype) in [8, 9] and boottype[0:4] == 'Boot':
                n = boottype[4:8]
                f = ''
                if len(boottype) == 9:
                    f = boottype[8]
                # fi
                # debugmsg("Boot - '{}' with boot active flag '{}'".format(n, f))
                a = re.split(r'\t', c[1], maxsplit=1)
                if n == bootcurrent:
                    if f != '*':
                        warnmsg("Boot{} current line does not have '*' flag!".format(n))
                        e = "{} --bootnum {} --active".format(efibootmgr, n)
                        activate_commands[n] = e
                    # fi
                    if n in order:
                        order.remove(n)
                    else:
                        if f == '*':
                            warnmsg("Boot{} current line with '*' flag not in BootOrder!".format(n))
                            rest_order.append(n)
                        # fi
                    # fi
                elif n in order:
                    if f != '*':
                        warnmsg("Boot{} in BootOrder but does not have '*' flag!".format(n))
                        e = "{} --bootnum {} --active".format(efibootmgr, n)
                        activate_commands[n] = e
                    # fi
                    order.remove(n)
                    rest_order.append(n)
                else:
                    if f == '*':
                        warnmsg("Boot{} current line with '*' flag not in BootOrder!".format(n))
                        rest_order.append(n)
                    # fi
                # fi
                # .....................................
                # See if should keep this boot line.
                resultkeep = False
                for d in keep_patterns:
                    if re.match(d, a[0]):
                        resultkeep = True
                        break
                    # fi
                # rof
                if resultkeep:
                    debugmsg("Keep 'Boot{}':".format(n))
                    if n in keepefi:
                        errmsg("Keep EFI key {} ({}) already had value:".format(n, keepefi[n][0]))
                        errmsg("    '{}'".format(keepefi[n][1]))
                        errmsg("  New key {} ({}) already had value:".format(n, f))
                        errmsg("    '{}'".format(a[1]))
                    # fi
                    keepefi[n] = [f, a]
                # fi
                # .....................................
                resultverify = False
                for d in verify_or_fix_patterns:
                    if re.match(d, a[0]):
                        resultverify = True
                        break
                    # fi
                # rof
                if resultverify:
                    debugmsg("Verify 'Boot{}':".format(n))
                    if n in verify_or_fix_efi:
                        errmsg("Verify EFI key {} ({}) already had value:".format(n, verify_or_fix_efi[n][0]))
                        errmsg("    '{}'".format(verify_or_fix_efi[n][1]))
                        errmsg("  New key {} ({}) already had value:".format(n, f))
                        errmsg("    '{}'".format(a[1]))
                    # fi
                    verify_or_fix_efi[n] = [f, a]
                # fi
                # .....................................
                # See if should delete this boot line.
                resultdelete = False
                for d in delete_patterns:
                    if re.match(d, a[0]):
                        resultdelete = True
                        break
                    # fi
                # rof
                if resultdelete and (resultkeep or resultverify):
                    errmsg("Boot{} line is found in both keep/verify and delete patterns -- please fix!".format(n))
                # fi
                if resultdelete:
                    if n == bootcurrent:    # Then must keep line, even it wish to delete it.
                        errmsg("Boot{} line should be deleted, but system is booted off it!".format(n))
                        keepefi[n] = [f, a]
                    else:
                        if n in bootorder:
                            warnmsg("Need to take out of BootOrder 'Boot{}':".format(n))
                            if n in inactive_commands:
                                errmsg("Boot{} line already to be removed from BootOrder?".format(n))
                            else:
                                inactive_commands[n] = "{} --bootnum {} --inactive".format(efibootmgr, n)
                            # fi
                        # fi
                        warnmsg("Should delete 'Boot{}':".format(n))
                        e = "{} --bootnum {} --delete-bootnum".format(efibootmgr, n)
                        if n in delete_commands:
                            errmsg("Boot{} line already to be deleted -- twice? How?".format(n))
                            errmsg("    old={}".format(delete_commands[n]))
                            errmsg("    new={}".format(e))
                        else:
                            delete_commands[n] = e
                        # fi
                    # fi
                # fi
            else:
                errmsg("Unexpected line type - '{}'".format(boottype[0:4]))
            # fi
        # rof
        if order != []:
            errmsg("Left over boot items in 'BootOrder:' ({})".format(order))
        # fi
    # End of parse_efibootmgr_v

    # ----------------------------------------------------------------------------
    def get_root1_root2_EFI1_EFI2():
        nonlocal mount, fstab, etc_fstab

        # /usr/bin/egrep '"root[12]|EFI[12]"' /etc/fstab
        # LABEL="root2"      /                 xfs  defaults                                        0 0
        # LABEL="root1"      /etc/root2        xfs  defaults                                        0 0
        # LABEL="EFI1"       /boot/efi         vfat defaults,uid=0,gid=0,umask=0077,shortname=winnt 0 0
        # LABEL="EFI2"       /b_boot/efi       vfat defaults,uid=0,gid=0,umask=0077,shortname=winnt 0 0

        # Information from /etc/fstab, get root1, root2, EFI1, and EFI2.
        cmd_fstab = '{} \'^LABEL="root[12]"|^LABEL="EFI[12]"\' {}'.format(cmd_egrep, etc_fstab)
        try:
            fstab_output = subprocess.check_output(cmd_fstab, shell=True, universal_newlines=True).strip()
        except:
            errmsg('Error processing command "{}".'.format(cmd_fstab))
            sys.exit(1)
        # yrt
        if fstab_output is None or str(fstab_output) == '':
            errmsg('No output from command "{}".'.format(cmd_fstab))
            sys.exit(1)
        # fi
        lines = re.split('\n', fstab_output)      # Break output on two newlines in a row.

        for l in lines:
            c = re.split(r'\s+', l)
            if len(c) != 6:
                errmsg('Did not find six fields in {} found line "{}"'.format(etc_fstab, l))
                sys.exit(1)
            # fi
            label = c[0][0:7]
            if label != 'LABEL="':
                errmsg('{} line "{}" does not start with LABEL="'.format(etc_fstab, l))
                sys.exit(1)
            # fi
            fs3 = c[0][7:10]
            fs4 = c[0][7:11]
            fs5 = c[0][7:12]
            oort = None
            on = c[1]
            if fs3 == 'EFI':
                oort = fs4[3]
                fs = fs4
                if on not in ['/boot/efi', '/b_boot/efi']:
                    errmsg('{} line "{}" not mounted on known location"'.format(etc_fstab, l))
                    sys.exit(1)
                # fi
            elif fs4 == 'root':
                oort = fs5[4]
                fs = fs5
                if on not in ['/', '/etc/root2']:
                    errmsg('{} line "{}" not mounted on known location"'.format(etc_fstab, l))
                    sys.exit(1)
            # fi
            if oort not in ['1', '2']:
                errmsg("End of LABEL not 1 or 2 ('{}')  line='{}'".format(oort, l))
                sys.exit(1)
            # fi
            fstab[fs] = on
        # rof
        # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
        # mount | egrep ' / | /etc/root2 | /boot/efi | /b_boot/efi | EFI[12] '
        # /dev/md120 on / type xfs (rw,relatime,attr2,inode64,logbufs=8,logbsize=32k,noquota)
        # /dev/sdf1 on /b_boot/efi type vfat (rw,relatime,fmask=0077,dmask=0077,codepage=437,iocharset=iso8859-1,shortname=winnt,errors=remount-ro)
        # /dev/sde1 on /boot/efi type vfat (rw,relatime,fmask=0077,dmask=0077,codepage=437,iocharset=iso8859-1,shortname=winnt,errors=remount-ro)
        # /dev/md125 on /etc/root2 type xfs (rw,relatime,attr2,inode64,logbufs=8,logbsize=32k,noquota)
        cmd_mount = "/usr/bin/mount | /usr/bin/egrep ' / | /etc/root2 | /boot/efi | /b_boot/efi | EFI[12] '"
        try:
            mount_output = subprocess.check_output(cmd_mount, shell=True, universal_newlines=True).strip()
        except:
            errmsg('Error processing command "{}".'.format(cmd_mount))
            sys.exit(1)
        # yrt
        if mount_output is None or str(mount_output) == '':
            errmsg('No output from command "{}".'.format(cmd_mount))
            sys.exit(1)
        # fi
        lines = re.split('\n', mount_output)      # Break output on two newlines in a row.

        for l in lines:
            c = re.split(r'\s+', l)
            if len(c) != 6:
                errmsg('Did not find six fields in mount output line "{}"'.format(l))
                sys.exit(1)
            # fi
            device = c[0]
            if c[1] != 'on':
                errmsg('Field 2 is not "on" ("{}") in mount output line "{}"'.format(l, c[1]))
                sys.exit(1)
            # fi
            directory = c[2]
            if c[3] != 'type':
                errmsg('Field 4 is not "type" ("{}") in mount output line "{}"'.format(l, c[1]))
                sys.exit(1)
            # fi
            if c[4] == 'xfs':           # root1 or root2
                if directory not in ['/', '/etc/root2']:
                    errmsg('Directory for xfs file system is not "/" , or "/etc/root2" ("{}") for line "{}"'.format(directory, l))
                    sys.exit(1)
                # fi
            elif c[4] == 'vfat':        # EFI1 or EFI2
                if directory not in ['/boot/efi', '/b_boot/efi']:
                    errmsg('Directory for xfs file system is not "/boot/efi" , or "/b_boot/efi" ("{}") for line "{}"'.format(directory, l))
                    sys.exit(1)
                # fi
            else:
                errmsg('file system type is not xfs or vfat ("{}") in mount line "{}"'.format(c[4], l))
                sys.exit(1)
            # fi
            mount[directory] = device
        # rof
        # -------------------------------------
        # Verify reasonable results in fstab and mount.
        f = fstab.copy()
        m = mount.copy()
        if 'root1' not in f:
            errmsg('{} does not have "root1" in it.'.format(etc_fstab))
        else:
            del f['root1']
        # fi
        if '/' not in mount:
            errmsg('mount does not show "/" mounted')
        else:
            del m['/']
        # fi
        # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
        if 'root2' not in f:
            errmsg('{} does not have "root2" in it.'.format(etc_fstab))
        else:
            del f['root2']
        # fi
        if '/etc/root2' not in m:
            errmsg('mount does not show "/etc/root2" mounted')
        else:
            del m['/etc/root2']
        # fi
        # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
        if 'EFI1' not in f:
            errmsg('{} does not have "EFI1" in it.'.format(etc_fstab))
        else:
            del f['EFI1']
        # fi
        if '/boot/efi' not in m:
            errmsg('mount does not show "/boot/efi" mounted')
        else:
            del m['/boot/efi']
        # fi
        # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
        if 'EFI2' not in f:
            errmsg('{} does not have "EFI2" in it.'.format(etc_fstab))
        else:
            del f['EFI2']
        # fi
        if '/b_boot/efi' not in m:
            errmsg('mount does not show "/b_boot/efi" mounted')
        else:
            del m['/b_boot/efi']
        # fi
        # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
        if f != {}:
            errmsg('{} has unexpected items in it: "{}"'.format(etc_fstab, f))
        # fi
        if m != {}:
            errmsg('mount has unexpected items in it: "{}"'.format(m))
        # fi
        # Stuff checked twice -- just in case!
    # End of get_root1_root2_EFI1_EFI2

    # ----------------------------------------------------------------------------
    def get_disk_partition_filesystem_UUIDs():
        nonlocal s_EFI1, s_EFI2, part_uuid, uuid_part

        # Get symbolic links for EFI1, then EFI2, then partition uuids.

        # ls -l /dev/disk/by-{label,uuid}/* | egrep '/sdf1$|/sde1$|EFI1|EFI2'
        #    ... /dev/disk/by-label/EFI1 -> ../../sde1
        #    ... /dev/disk/by-label/EFI2 -> ../../sdf1
        #    ... /dev/disk/by-partuuid/e09b5a60-69c3-4f67-961c-6c3df25b1de0 -> ../../sde1
        #    ... /dev/disk/by-partuuid/a4858da5-38b0-4282-96fd-e6384b84d135 -> ../../sdf1
        # .....................................
        try:
            s_EFI1 = os.readlink('{}/EFI1'.format(dev_disk_by_label))
            if s_EFI1[0:6] != '../../':
                errmsg('Symbolic link for {}/EFI1 does not start with "../../" - "{}"'.format(dev_disk_by_label, s_EFI1))
                s_EFI1 = None
            else:
                s_EFI1 = s_EFI1[6:]
            # fi
        except:
            errmsg('Unable to read symbolic link for {}/EFI1'.format(dev_disk_by_label))
            s_EFI1 = None
        # yrt
        # .....................................
        try:
            s_EFI2 = os.readlink('{}/EFI2'.format(dev_disk_by_label))
            if s_EFI2[0:6] != '../../':
                errmsg('Symbolic link for {}/EFI2 does not start with "../../" - "{}"'.format(dev_disk_by_label, s_EFI2))
                s_EFI2 = None
            else:
                s_EFI2 = s_EFI2[6:]
            # fi
        except:
            errmsg('Unable to read symbolic link for {}/EFI2'.format(dev_disk_by_label))
            s_EFI2 = None
        # yrt
        # .....................................

        # Now get all file/directory names in /dev/disk/by-partuuid/.
        try:
            names = os.listdir(dev_disk_by_partuuid)
        except:
            errmsg('Unable to read directory for {}'.format(dev_disk_by_partuuid))
            return
        # yrt
        for n in names:
            fn = dev_disk_by_partuuid + '/' + n
            if not os.path.islink(fn):
                continue
            # fi
            try:
                l = os.readlink(fn)
                if l[0:6] != '../../':
                    errmsg('Symbolic link for "{}" does not start with "../../" - "{}"'.format(fn, l))
                    continue
                # fi
                l = l[6:]
                if l in [s_EFI1, s_EFI2]:
                    part_uuid[l] = n
                    uuid_part[n] = l
                # fi
            except:
                errmsg('Unable to read symbolic link for "{}"'.format(fn))
            # yrt
        # rof
    # End of get_disk_partition_filesystem_UUIDs

    # ----------------------------------------------------------------------------
    def check_shimx64_efi():
        nonlocal boot_fix_commands
        #   ./redhat/shimx64.efi
        efi_boot = True
        efi_b_boot = True
        if (not os.path.isfile('/boot/efi/EFI/redhat/shimx64.efi') or
                not os.path.isfile('/boot/efi/EFI/redhat/fonts/unicode.pf2') or
                not os.path.isfile('/boot/efi/EFI/redhat/grub.cfg') or
                not os.path.isfile('/boot/efi/EFI/redhat/grubenv') or
                not os.path.isfile('/boot/efi/EFI/redhat/grubx64.efi')):
            warnmsg('Secure EFI boot file not present in /boot.')
            efi_boot = False
        # fi
        if (not os.path.isfile('/b_boot/efi/EFI/redhat/shimx64.efi') or
                not os.path.isfile('/b_boot/efi/EFI/redhat/fonts/unicode.pf2') or
                not os.path.isfile('/b_boot/efi/EFI/redhat/grub.cfg') or
                not os.path.isfile('/b_boot/efi/EFI/redhat/grubenv') or
                not os.path.isfile('/b_boot/efi/EFI/redhat/grubx64.efi')):
            warnmsg('Secure EFI boot files not present in /b_boot.')
            efi_b_boot = False
        # fi

        if not efi_boot:            # Check if other is present, and can copy from there.
            if not efi_b_boot:
                errmsg('Cannot fix EFI boot from backup boot directory.')
                sys.exit(1)
            # fi
            boot_fix_commands.append('tar cf - -C /b_boot/efi/EFI/redhat shimx64.efi fonts grub.cfg grubenv grubx64.efi | tar xf - -C /boot/efi/EFI/redhat')
            return
        # fi
        if not efi_b_boot:
            boot_fix_commands.append('tar cf - -C /boot/efi/EFI/redhat shimx64.efi fonts grub.cfg grubenv grubx64.efi | tar xf - -C /b_boot/efi/EFI/redhat')
        # fi
    # End of check_shimx64_efi

    # ----------------------------------------------------------------------------
    def verify_efi_variables():
        nonlocal verify_or_fix_efi
        nonlocal s_EFI1, s_EFI2, part_uuid, uuid_part
        nonlocal efi1_create, efi2_create, efi1_boot, efi2_boot

        # Go through verify_or_fix_efi dictionary and see if they are okay.
        # a) create entry for /boot/efi/EFI/redhat/shimx64.efi.
        #    Is it in verify_or_fix_efi?
        # b) create entry for /b_boot/efi/EFI/redhat/shimx64.efi.
        #    Is it in verify_or_fix_efi?
        # Note any left-over.
        # efibootmgr --create --disk /dev/sdb --part 1 --label 'AlmaLinux' --loader '\\EFI\\redhat\\shimx64.efi'
        # Boot0002* AlmaLinux HD(2,GPT,ecc8868b-c1aa-4ada-8c7a-e7b679c8a643,0x0,0x0)/File(\\EFI\\redhat\\shimx64.efi)

        efi1_c = {
            'PARTUUID' : part_uuid[s_EFI1],
            '--loader' : r'\\EFI\\REDHAT\\SHIMX64.EFI',
            '--label'  : 'AlmaLinux',
            '--part'   : s_EFI1[3],
            '--gpt'    : '',
            '--create' : '',
        }

        efi2_c = {
            'PARTUUID' : part_uuid[s_EFI2],
            '--loader' : r'\\EFI\\REDHAT\\SHIMX64.EFI',
            '--label'  : 'AlmaLinux',
            '--part'   : s_EFI2[3],
            '--gpt'    : '',
            '--create' : '',
        }

        # Create commands to create the two EFI Boot variables.
        if efi1_c != {}:
            efi1_c['--disk'] = '/dev/' + s_EFI1
            e1 = "{}".format(efibootmgr)
            for k in efi1_c:
                if k != 'PARTUUID':
                    e1 += ' ' + k + ' ' + efi1_c[k]
                # fi
            # rof
            # debugmsg("e1={}".format(e1))
        # fi
        if efi2_c != {}:
            efi2_c['--disk'] = '/dev/' + s_EFI2
            e2 = "{}".format(efibootmgr)
            for k in efi2_c:
                if k != 'PARTUUID':
                    e2 += ' ' + k + ' ' + efi2_c[k]
                # fi
            # rof
            # debugmsg("e2={}".format(e2))
        # fi

        # verify_or_fix_efi - EFI variables to verify and possibly fix.
        efi1_create = e1
        efi2_create = e2
        if verify_or_fix_efi == {}:         # Nothing to delete.
            return
        # fi
        pattern = r'^HD\(([0-9]+),GPT,([0-9a-f-]+),[0-9a-fx]+,[0-9a-fx]+\)/File\(([^)]+)\)(.*)$'
        notokay = []
        for n in verify_or_fix_efi:
            a = verify_or_fix_efi[n][1]
            w = a[1]
            aaa = re.search(pattern, w)
            if aaa is None:
                continue
            # fi
            partition = aaa.group(1)
            uuid = aaa.group(2)
            file = aaa.group(3)
            file = file.encode('unicode_escape').decode()

            if (s_EFI1[3] == partition and
                    uuid == part_uuid[s_EFI1] and
                    (file == r'\\EFI\\REDHAT\\SHIMX64.EFI' or
                     file == r'\\EFI\\redhat\\shimx64.efi') and
                    efi1_create is not None):
                debugmsg("Good, matched EFI1")
                efi1_create = None          # Already set!
                efi1_boot = n
            elif (s_EFI2[3] == partition and
                  uuid == part_uuid[s_EFI2] and
                  (file == r'\\EFI\\REDHAT\\SHIMX64.EFI' or
                   file == r'\\EFI\\redhat\\shimx64.efi') and
                  efi2_create is not None):
                debugmsg("Good, matched EFI2")
                efi2_create = None          # Already set!
                efi2_boot = n
            else:
                warnmsg("Boot{} did not match what is expected for EFI1 nor EFI2".format(n))
                warnmsg("    partition={} uuid={} file={}".format(partition, uuid, file))
                notokay.append(n)
            # fi
        # rof
        # .....................................
        for n in notokay:
            if n in bootorder:
                warnmsg("Need to take out of BootOrder 'Boot{}':".format(n))
                if n in inactive_commands:
                    errmsg("Boot{} line already to be removed from BootOrder?".format(n))
                else:
                    inactive_commands[n] = "{} --bootnum {} --inactive".format(efibootmgr, n)
                # fi
            # fi
            e = "{} --bootnum {} --delete-bootnum".format(efibootmgr, n)
            if n in delete_commands:
                errmsg("Boot{} line already to be deleted -- twice? How?".format(n))
                errmsg("    old={}".format(delete_commands[n]))
                errmsg("    new={}".format(e))
            else:
                delete_commands[n] = e
            # fi
        # rof
    # End of verify_efi_variables

    # ============================================================================
    # main processing.
    # .....................................
    args = None
    # .....................................
    parse_args()
    # .....................................
    mount_efi_boot_dirs()
    # .....................................
    # Items needed from efibootmgr.
    delete_commands = {}            # efibootmgr delete commands
    inactive_commands = {}          # efibootmgr --inactive commands
    bootcurrent = None
    bootorder = []
    rest_order = []
    activate_commands = {}
    keepefi = {}
    verify_or_fix_efi = {}
    parse_efibootmgr_v()
    # .....................................
    # Parse output into dictionary, key of label, and value is where mounted.
    fstab = {}
    # Parse output into dictionary, key of directory mounted on, and device is value.
    mount = {}
    get_root1_root2_EFI1_EFI2()
    # .....................................
    # Find disk partitions for EFI labels, and their UUIDs.
    s_EFI1 = None       # Disk partition (sde1) containing EFI1 label.
    s_EFI2 = None       # Disk partition (sdf1) containing EFI2 label.
    part_uuid = {}      # partition name gives uuid.  { sdf1 : a4858da5-38b0-4282-96fd-e6384b84d135 }
    uuid_part = {}      # uuid gives partition name.  { e09b5a60-69c3-4f67-961c-6c3df25b1de0 : sde1 }
    get_disk_partition_filesystem_UUIDs()
    # .....................................
    # Check that shimx64.efi is in correct places.
    boot_fix_commands = []
    check_shimx64_efi()
    # .....................................
    # Figure out commands that can fix EFI variables.
    efi1_create = None                                  # command for creating EFI1 variable.
    efi2_create = None                                  # command for creating EFI2 variable.
    efi1_boot = None                                    # for Boot Order.
    efi2_boot = None                                    # for Boot Order.
    verify_efi_variables()

    if efi1_boot in rest_order:
        rest_order.remove(efi1_boot)
    # fi
    if efi2_boot in rest_order:
        rest_order.remove(efi2_boot)
    # fi
    debugmsg("------------------------------------------------------------------------------")
    debugmsg("fstab={}".format(fstab))                 # Current fstab for root1/2,EFI1/2.
    debugmsg("mount={}".format(mount))                 # Current mounts for root1/2,EFI1/2.
    debugmsg("bootcurrent='{}'".format(bootcurrent))   # Currently booted from.
    debugmsg("bootorder='{}'".format(bootorder))       # Current boot order.
    debugmsg("rest_order='{}'".format(','.join(rest_order)))
    # debugmsg("activate_commands='{}'".format(','.join(activate_commands)))
    # debugmsg("s_EFI1 - {}/EFI1 is '{}'".format(dev_disk_by_label, s_EFI1))
    # debugmsg("s_EFI2 - {}/EFI2 is '{}'".format(dev_disk_by_label, s_EFI2))
    # debugmsg("part_uuid='{}'".format(part_uuid))
    # debugmsg("uuid-part='{}'".format(uuid_part))
    # Values in EFI that should be kept ... figure out what is missing.
    if keepefi != {}:
        debugmsg("keepefi - EFI variables to keep:")
        for k in keepefi:
            debugmsg("  Boot{}{} value='{}'".format(k, keepefi[k][0], keepefi[k][1]))
        # rof
    # fi
    # verify_or_fix_efi - EFI variables to verify and possibly fix.
    if verify_or_fix_efi != {}:
        debugmsg("verify_or_fix_efi - EFI variables to verify and possibly fix:")
        for k in verify_or_fix_efi:
            debugmsg("  Boot{}{} value='{}'".format(k, verify_or_fix_efi[k][0], verify_or_fix_efi[k][1]))
    # rof
    # =====================================
    if boot_fix_commands != []:
        debugmsg("# Command(s) to fix missing files in boot EFI directory:")
        for k in boot_fix_commands:
            print("{}".format(k))
            if not args.dryrun:
                try:
                    subprocess.check_output(k, shell=True, universal_newlines=True).strip()
                except:
                    errmsg('Error processing command "{}".'.format(k))
                # yrt
            # fi
        # rof
    # fi
    # .....................................
    if efi1_create is not None or efi2_create is not None:
        debugmsg("# Commands to create EFI Boot variables:")
    # fi
    if efi1_create is not None:
        print('{}'.format(efi1_create))
        if not args.dryrun:
            try:
                subprocess.check_output(efi1_create, shell=True, universal_newlines=True).strip()
            except:
                errmsg('Error processing command "{}".'.format(efi1_create))
            # yrt
        # fi
    # fi
    if efi2_create is not None:
        print('{}'.format(efi2_create))
        if not args.dryrun:
            try:
                subprocess.check_output(efi2_create, shell=True, universal_newlines=True).strip()
            except:
                errmsg('Error processing command "{}".'.format(efi2_create))
            # yrt
        # fi
    # fi
    # .....................................
    if args.delete and (inactive_commands != {} or delete_commands != {}):
        debugmsg("# Commands to delete EFI Boot variables not desired/needed.")
        print('# Suggested command(s) to execute:')
        if inactive_commands != {}:
            for d in inactive_commands:
                print('{}'.format(inactive_commands[d]))
            # rof
        # fi
        if delete_commands != {}:
            for d in delete_commands:
                print('{}'.format(delete_commands[d]))
            # rof
        # fi
    # fi
    # .....................................
    if activate_commands != {}:
        debugmsg("# Command(s) to activate EFI Boot variables already active or should active.")
        print('# Suggested command(s) to execute:')
        for a in activate_commands:
            bootorder_cmd = activate_commands[a]
            print('{}'.format(bootorder_cmd))
            if not args.dryrun:
                try:
                    subprocess.check_output(bootorder_cmd, shell=True, universal_newlines=True).strip()
                except:
                    errmsg('Error processing command "{}".'.format(bootorder_cmd))
                # yrt
            # fi
        # rof
    # fi
    # .....................................
    # Figure out bootorder.
    if efi1_boot and efi2_boot:
        if rest_order != []:
            neworder = ','.join([efi1_boot, efi2_boot] + rest_order)
        else:
            neworder = ','.join([efi1_boot, efi2_boot])
        # fi
    else:
        neworder = ''
    # fi
    debugmsg("neworder='{}'".format(neworder))
    if efi1_create is not None or efi2_create is not None:
        errmsg('Cannot specify boot order until above create commands are successful and deletes are done.')
        errmsg('Note: creates are put first, so everything should boot okay. :)')
    elif neworder != ','.join(bootorder):
        #   efibootmgr --quiet --bootorder 7,9,4,8,a
        debugmsg("# Command to set EFI Boot Order:")
        bootorder_cmd = "{} --quiet --bootorder {}".format(efibootmgr, neworder)
        print('{}'.format(bootorder_cmd))
        if not args.dryrun:
            try:
                subprocess.check_output(bootorder_cmd, shell=True, universal_newlines=True).strip()
            except:
                errmsg('Error processing command "{}".'.format(bootorder_cmd))
            # yrt
        # fi
    # fi

    if errors_occurred != 0:
        print('# Number of errors={}'.format(errors_occurred))
    # fi
    if warnings_occurred != 0:
        print('# Number of warnings={}'.format(warnings_occurred))
    # fi
# End of main

# ----------------------------------------------------------------------------
# Execute the main routine.
if __name__ == '__main__':
    main()
    sys.exit(0)
# fi
# ----------------------------------------------------------------------------
# End of file fixefivars.py
