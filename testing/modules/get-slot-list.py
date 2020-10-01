#!/usr/bin/python3
#-----------------------------------------------------------------------------
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#-----------------------------------------------------------------------------
# 2020-05-19 Modified from perl script to python script.
#
# The argparse routine prints following triple quote as part of help message.
'''
Script used by other scripts to determine Fiber Channel Targets/Initiators, rports
seen on initiator (also called host) ports, WWPN's/PCI slots/Speeds/LinkState, etc.

Note: Commands may be abbreviated to uniqueness. Example: "i" for "inititor".

Synopsis: (May be abbreviated to uniqueness...)
    initiator  - FC initiators on their own line.
    target     - FC targets on their own line.
    qla2xxx    - Outputs a line in format for /etc/modprobe.d/qla2xxx.conf,
                 where the first half of found FC devices are made Target.
    lots       - A lot of detail but not as much as no argument. --json does this.
    full       - No argument -- see (nothing) below.
    (nothing)  - Output FC WWPNs, cards, PCI slot, speeds, link state, etc.,
                 "initiator", "target", and physical slots cards are in.
                 Add "-s" for initiators/targets seen by initiator ports.

Examples:
    get-slot-list.py
    get-slot-list.py initiator
    get-slot-list.py target
    get-slot-list.py qla2xxx > /etc/modprobe.d/qla2xxx.conf
    get-slot-list.py lots --json
    get-slot-list.py --seen
    get-slot-list.py --luns
    get-slot-list.py --vv
    get-slot-list.py --verbose
'''
#-----------------------------------------------------------------------------
# Modules to include for program.
import sys
import json
#-----------------------------------------------------------------------------
def print_json(str, json_to_print):
    if str and str != '':
        print(str)
    print(json.dumps(json_to_print, skipkeys=True, sort_keys=True, indent=2,
                     ensure_ascii=True, separators=(',', ':') ) )
    return

#-----------------------------------------------------------------------------
def main(values):
    #-----------------------------------------------------------------------------
    import argparse                             # Process arguments.
    import os                                   # O/S routines - directory paths ...
    import glob
    import re
    import subprocess
    #-----------------------------------------------------------------------------
    # Routines start.
    #-----------------------------------------------------------------------------
    def parse_args(values):

        init = ('i', 'in', 'ini', 'init', 'initi', 'initia', 'initiat', 'initiato',
                'initiator', 'initiators')
        targ = ('t', 'ta', 'tar', 'targ', 'targe', 'target', 'targets')
        qla2 = ('q', 'ql', 'qla', 'qla2', 'qla2x', 'qla2xx', 'qla2xxx', 'qla2xxx.',
                'qla2xxx.c', 'qla2xxx.co', 'qla2xxx.con', 'qla2xxx.conf')
        lots = ('l', 'lo', 'lot', 'lots')
        full = ('f', 'fu', 'ful', 'full')

        parser = argparse.ArgumentParser(
            description=__doc__,
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog= 'List Linux host numbers, Fibre Channel WWPNs, Card type and Firmware versions,\n' +
                    'PCI slot numbers, Speed and Supported Speeds, and Link States.\n' +
                    'May be useful for setting up the file: /etc/modprobe.d/qla2xxx.conf\n')
        parser.add_argument('--verbose', '-v', action='store_true',
                            help = 'Print each line of qla2xxx.conf file.')
        parser.add_argument('--vv', '-vv', action='store_true',
                            help = 'Print each Fibre line of \'lspci -vmmD\' output.')
        parser.add_argument('--json', '-j', '--JSON', action='store_true',
                            help = 'Print in JSON format, without headers.')
        parser.add_argument('--seen', '-s', action='store_true',
                            help = 'Print rports seen on host ports (for "full"/"lots".')
        parser.add_argument('--luns', '-l', action='store_true',
                            help = 'Print LUNS available on host ports for "full"/"lots".')
        parser.add_argument('rest', nargs='*',
                            metavar='initiator|target|qla2xxx|lots|full',
                            help='Optional output or format limiting.')
        args = parser.parse_args(values)
        # A few more semi-arguments to create.
        args.initiator = False
        args.target = False
        args.qla2xxx = False
        args.lots = False
        args.full = False
        error = False
        for a in args.rest:
            if a in init:
                args.initiator = True
            elif a in targ:
                args.target = True
            elif a in qla2:
                args.qla2xxx = True
            elif a in full:
                args.full = True
            elif a in lots:
                args.lots = True
            else:
                if not error:
                    print('-' * 78, file=sys.stderr)
                print("ERROR - unrecognized argument '{}'".format(a), file=sys.stderr)
                error = True
        if error:
            print('-' * 78, file=sys.stderr)
            parser.print_help()
            print('-' * 78, file=sys.stderr)
            print('ERROR - read line(s) above help message!', file=sys.stderr)
            exit(1)

        # If no positional argument, default to full.
        if not args.initiator and not args.target and not args.qla2xxx and not args.lots:
            args.full = True
        # NOTDONEYET -- Anything to check for things that do not make sense?
        return (args)
    # End of parse_args

    #-----------------------------------------------------------------------------
    def readfile(filename):
        try:
            with open(filename, 'r') as f:
                return f.read().splitlines()
        except:
            return []
    # End of readfile

    #-----------------------------------------------------------------------------
    def remove_quotes(string):
        if len(string) > 2:
            if string.startswith('"'):
                if string.endswith('"'):
                    string = string[1:]
                    string = string[:-1]
            elif string.startswith("'"):
                if string.endswith("'"):
                    string = string[1:]
                    string = string[:-1]
        return string
    # End of remove_quotes

    #-----------------------------------------------------------------------------
    # Append stderr to JSON array, or text for stderr.
    def errmsg(string, err_out):
        if args.json:
            return(err_out + [string])
        return(err_out + string + '\n')

    #-----------------------------------------------------------------------------
    # Parse configuration file.
    def parse_qla2xxx_config_file():
        nonlocal slot, wwpn, qle

        # The QLogic driver configuration file. (Specify targets verses initiators.)
        ConfigFile = '/etc/modprobe.d/qla2xxx.conf'
        # ConfigFile = '../etc.modprobe.d/qla2xxx.conf'

        error = False
        verbose = ''
        if args.json:
            err_out = []
        else:
            err_out = ''

        FC_slots = readfile(ConfigFile)
        if not FC_slots and not args.qla2xxx:
            err_out = errmsg('Nothing in file {}'.format(ConfigFile), err_out)
            qle = True                              # Default all qlogic ports as initiator.
            return verbose, err_out

        if args.verbose:
            verbose += '{}:'.format(ConfigFile) + '\n'
        l = 0                                       # Line count in file.
        for i in FC_slots:
            orig_line = i
            l = l + 1                               # Line count.
            if args.verbose:
                verbose += "{}: '{}'".format(l, orig_line) + '\n'
            i = i.strip()                           # Get rid of leading and trailing spaces.
            i = i.split('#')
            i = i[0].rstrip()
            if not i:
                continue
            # if args.verbose:
            #     verbose += "process line='{}'".format(i) + '\n'

            i = i.split()                           # Split line on white space(s).

            if i[0] != 'options' or i[1] != 'qla2xxx':
                err_out = errmsg("Unexpected line $l='{}'".format(orig_line), err_out)
                error = True
                continue

            i = i[2:]                               # Toss first two 'words', we know what they are.
            # NOTE: default is qlini_mode=enabled (0) in nonlocal variable initialization.
            for j in i:
                if j.startswith('qlini_mode='):
                    opt = remove_quotes(j[11:])
                    # print("process qli_mode='{}'".format(j), file=sys.stderr)
                    if opt == 'disabled':
                        qle = False                 # All ports are to be Target mode.
                        continue
                    if opt == 'enabled':
                        qle = True                  # All ports are to be Initiator mode.
                        continue

                elif j.startswith('qlini_targets='):
                    j = remove_quotes(j[14:])
                    # print("process qlini_targets='{}'".format(j), file=sys.stderr)
                    slot_or_wwpn = j.split(',')
                    for k in slot_or_wwpn:
                        if k.isdigit():
                            slot.update({k : k})
                            qle = True              # We have one target mode at least!
                            continue
                        if k.startswith(('0x', '0X')):
                            k = k[2:]
                        k = k.replace(':', '').lower()
                        if len(k) != 16:
                            err_out = errmsg("WWPN format is not 16 digits, line {} '{}'".format(l, orig_line), err_out)
                            error = True
                            continue
                        try:
                            tmp = int(k, 16)
                        except:
                            err_out = errmsg("WWPN format is bad for line {} '{}'".format(l, orig_line), err_out)
                            error = True
                            continue
                        k = ':'.join(k[m:m+2] for m in range(0, len(k), 2))
                        wwpn.update({str(k) : str(k)})
                        qle = True
                        continue

                else:
                    err_out = errmsg("Ignoring unexpected token '{}' in line '{}'".format(j, orig_line), err_out)

        if args.verbose and not args.json:
            verbose += 'done processing line(s)' + '\n'

        # Exit if parsing error in configuration file..
        if error:
            if args.json:
                out_json.update(out_json, {'verbose':verbose})
                out_json.update(out_json, {'stderr':err_out})
                print_json('error in parse_qla2xxx_config_file out_json=', out_json)
            else:
                print(verbose)
                print(err_out, file=sys.stderr)
            exit(1)

        # Verify legal combinations in the file.
        # Cannot have all QLogic cards as target and nothing else.
        if not qle:
            if not bool(slot) or not bool(wwpn):
                err_out = errmsg('Cannot have all QLogic ports as initiator and specific ones specified.', err_out)
                if args.json:
                    out_json.update(out_json, {'verbose':verbose})
                    out_json.update(out_json, {'stderr':err_out})
                    print_json('Bad qla2xxx.conf file with initiators and targets out_json=', out_json)
                else:
                    print(verbose)
                    print(err_out, file=sys.stderr)
                exit(1)
        return verbose, err_out
    # End of parse_qla2xxx_config_file

    #-----------------------------------------------------------------------------
    # We have Target port. That gives both fc_remote_ports and target -- for below.
    def output_rport_info(host, tmp_output):
        nonlocal all_output

        if args.json:
            err_out = []
        else:
            err_out = ''

        if not args.seen and not args.luns:
            # If not printing rports seen on target ports.
            if not args.json:
                # If need to append header.
                all_output.append(tmp_output)
            else:
                all_output.append(tmp_output)
            return err_out

        # Note: string.format() verse glob's normal use of {}.
        whereat = glob.glob('/sys/devices/pci*/*/*/{}'.format(host))
        if not whereat:
            err_out = errmsg('Did not find directory for /sys/devices/pci*/*/*/{}'.format(host), err_out)
            if not args.json:
                all_output.append(tmp_output)
            else:
                all_output.append(tmp_output)
            return err_out

        if len(whereat) != 1:
            err_out = errmsg('Did not find ONE directory for /sys/devices/pci*/*/*/{} -- {}'.format(host, len(whereat)), err_out)
            if not args.json:
                all_output.append(tmp_output)
            else:
                all_output.append(tmp_output)
            return err_out

        rports = glob.glob('{}/rport-*'.format(whereat[0]))
        rports = sorted(rports)
        for p in rports:
            tmp_rports = dict()
            (h_d, rp) = os.path.split(p)
            # Process possible fc_remote_ports/${rp} directory here.
            if os.path.isdir('{}/fc_remote_ports/{}'.format(p, rp)):
                many0 = readfile('{}/fc_remote_ports/{}/port_name'.format(p, rp))
                many1 = readfile('{}/fc_remote_ports/{}/node_name'.format(p, rp))
                many2 = readfile('{}/fc_remote_ports/{}/port_state'.format(p, rp))
                many3 = readfile('{}/fc_remote_ports/{}/roles'.format(p, rp))
                if args.json:
                    tmp_rports = {'port name':many0[0],
                                'node name':many1[0],
                                'port state':many2[0],
                                'roles':many3[0]}
                else:
                    tmp_rports = '\n    {}  {}  {:<10}  {}'.format(many0[0], many1[0], many2[0], many3[0])
            # See if any target* here.
            rp_targets = glob.glob('{}/target*/*:*:*'.format(p))
            # if no luns for rport, nothing to save or print.
            if not rp_targets and not args.seen:
                continue
            if not args.json:
                tmp_output += tmp_rports
            rp_targets = sorted(rp_targets)
            if args.json:
                rp_output = []
            else:
                rp_output = ''
            for rp_t in rp_targets:
                rpt_lun = re.findall('^.*:([0-9]+)$', rp_t)

                many0 = readfile('{}/type'.format(rp_t))
                many1 = readfile('{}/wwid'.format(rp_t))
                many2 = readfile('{}/vendor'.format(rp_t))
                many3 = readfile('{}/model'.format(rp_t))
                if not args.json:
                    rp_tmp_out = '\n      LUN={:>2}  {:<8}  {:<44}  {:>8}  {}'.format(
                                    rpt_lun[0],
                                    scsi_short_device_types[int(many0[0])],
                                    many1[0],
                                    many2[0],
                                    many3[0])
                else:
                    rp_tmp_out = { 'lun':rpt_lun[0],
                                 'SCSI type':scsi_short_device_types[int(many0[0])].strip(),
                                 'wwid':many1[0].strip(),
                                 'vendor':many2[0].strip(),
                                 'model':many3[0].strip() }
                # args.luns without args.seen does not print the following.
                if args.seen:
                    many4 = readfile('{}/ioerr_cnt'.format(rp_t))
                    many5 = readfile('{}/dh_state'.format(rp_t))
                    many6 = readfile('{}/state'.format(rp_t))
                    many7 = readfile('{}/access_state'.format(rp_t))
                    if args.json:
                        rp_tmp_out.update( { 'ioerr cnt':many4[0],
                                           'dh_state':many5[0].strip(),
                                           'state':many6[0].strip(),
                                           'access_state':many7[0].strip() })
                    else:
                        rp_tmp_out += '\n      ioerr={:<10}  {:<8}  {:<10}  {}'.format(
                                        many4[0], many5[0], many6[0], many7[0])
                if args.json:
                    rp_output += [ rp_tmp_out ]
                else:
                    rp_output += rp_tmp_out
            if args.json:
                if rp_output:
                    tmp_rports.update( {'remote targets':rp_output} )
                    tmp_output.update( tmp_rports )
            else:
                tmp_output += rp_output
        if args.json and tmp_output:
            all_output += [ { 'rports': tmp_output } ]
        else:
            all_output.append(tmp_output)

        return err_out
    # End of output_rport_info

    #-----------------------------------------------------------------------------
    def print_header():
        nonlocal args, all_output

        # Note: --json does not print out a header.
        if (args.full or args.lots) and not args.json:
            tmp_output = ' {:<6.6} {:<12.12} {:<17.17} {:<8.8} {:<23.23} {:>7.7} - {}\n'.format(
                           'Host', 'PCI Slot', 'Symbolic Name', 'State', 'WWPN',
                                'Speed', 'Supported Speeds')
            if args.seen or args.luns:
                tmp_output = tmp_output + '    {:<18}  {:18}  {:<10}  {}\n'.format(
                             'Seen     port_name', '         node_name', 'port_state', 'roles')
                tmp_output = tmp_output + '      {:>6}  {:>8}  {:<44}  {:<8}  {}\n'.format(
                             'LUN   ', 'SCSItype', 'wwid', 'vendor', 'model')
            if args.seen:
                tmp_output = tmp_output + '      {:<16}  {:>8}  {:<10}  {}'.format(
                             'ioerr_cnt', 'dh_state', 'state', 'access_state')
            tmp_output = tmp_output.rstrip('\n')
            all_output.append(tmp_output)
            ht.append('')       # Array used for ' ' or 'T' in front of port_name (WWPN).
        return
    # End of print_header

    #-----------------------------------------------------------------------------
    def parse_lspci():
        nonlocal args, target_wwpns, initiator_wwpns, physical_slots, ht
        #-- nonlocal target_slots

        # Information from the lspci command -- slots, etc.
        cmd_lspci = '/usr/sbin/lspci -vmmD'         # Get physical pci devices on machine.

        verbose =''
        if args.json:
            err_out = []
            output = dict()
        else:
            output = ''
            err_out = ''
        if args.vv:
            verbose += '{}:'.format(cmd_lspci) + '\n'

        lspci_output = subprocess.check_output(cmd_lspci, shell=True, close_fds=True, universal_newlines=True).strip()
        if lspci_output is not None and str(lspci_output) != '':
            lspci = re.split('\n\n', lspci_output)      # Break output on two newlines in a row.
        else:
            lspci = []

        fc_target_ports = []                            # List of FC targets.

        for i in lspci:
            if not 'Fibre Channel' in i:                # Double new line section must have Fibre Channel.
                continue
            if args.vv:
                verbose += i + '\n\n'                    # Print each Fibre section processed.
            lines = i.splitlines()

            flag_slot = False
            for k in lines:
                if 'PhySlot:' in k or 'Rev:' in k:
                    if flag_slot:
                        continue
                    flag_slot = True
                    # k = physical slot (which really isn't physical slot ... ).
                    if 'Rev:' not in k:
                        k = re.sub('^.*[ \t]+', '', k)
                    else:
                        k = 0

                    # Get pci slot name.
                    pci_slot = re.sub('^Slot:[ \t]+', '', lines[0])

                    whereat = glob.glob('/sys/devices/pci*/*/{}/host*/fc_host/host*'.format(pci_slot))
                    if len(whereat) != 1:
                        continue

                    (h_d, host) = os.path.split(whereat[0])
                    symbolic_name = readfile('{}/symbolic_name'.format(whereat[0]))
                    if not args.json:
                        symbolic_name = symbolic_name[0].replace('FW:', '')
                        symbolic_name = re.sub(' DVR:.*', '', symbolic_name)
                        symbolic_name = re.sub(' FVR:.*', '', symbolic_name)
                    else:
                        symbolic_name = symbolic_name[0]

                    if args.json:
                        physical_slots[k] = { 'slot':k, 'card type and firmware':symbolic_name }
                    else:
                        physical_slots[k] = '{:>02} - {}'.format(k, symbolic_name)

                    port_name = readfile('{}/port_name'.format(whereat[0]))
                    port_name = re.sub('^0x', '', port_name[0])
                    port_name = ':'.join(port_name[m:m+2] for m in range(0, len(port_name), 2))
                    if port_name is None or port_name == '':
                        continue
                    fc_target_ports.append(port_name)

                    if args.lots or args.full:
                        port_state = readfile('{}/port_state'.format(whereat[0]))
                        port_state = port_state[0]
                        speed = readfile('{}/speed'.format(whereat[0]))
                        speed = re.sub(' ', '', speed[0])
                        supported_speeds = readfile('{}/supported_speeds'.format(whereat[0]))
                        supported_speeds = re.sub(' ', '', supported_speeds[0])
                        if args.json:
                            tmp_output = { 'linux host':host,
                                           'pci slot':pci_slot,
                                           'card&firmware':symbolic_name,
                                           'port state':port_state,
                                           'port name':port_name,
                                           'speed':speed,
                                           'supported speeds':supported_speeds
                                         }
                        else:
                            tmp_output = ' {:<6.6} {:<12.12} {:<17.17} {:<8.8} {:<23.23} {:>7.7} - {}'.format(
                                           host, pci_slot, symbolic_name, port_state, port_name,
                                           speed, supported_speeds)

                        # Following puts tmp_output into all_output array, and
                        # potentially makes it much longer output with --seen.
                        err_out += output_rport_info(host, tmp_output)
                        ht.append(port_name)

                    if re.match('^QLE', symbolic_name):
                        if not qle:
                            target_wwpns[port_name] = port_name
                            break
                        else:
                            # Check if slot number matches.
                            flag_nomore = False
                            for j in sorted(slot.keys()):
                                if k == j:
                                    #-- target_slots[k] = k
                                    target_wwpns[port_name] = port_name
                                    flag_nomore = True
                                    break
                            if flag_nomore:
                                break
                            # Check if port_name matches.
                            for j in sorted(wwpn.keys()):
                                if j == port_name:
                                    target_wwpns[port_name] = port_name
                                    flag_nomore = True
                                    break
                            if flag_nomore:
                                break
                    initiator_wwpns[port_name] = port_name
                    break
        # Print out port list for /etc/modprobe.d/qla2xxx.conf.
        # First half of ports are for target mode.
        if args.qla2xxx:
            if len(fc_target_ports) < 2:
                err_out = errmsg('Must have more than one qlogic port, not {}'.format(len(fc_target_ports)), err_out)
            else:
                port_list = ','.join(fc_target_ports[:int(len(fc_target_ports) / 2)])
                tmp_output = 'options qla2xxx qlini_mode="enabled" qlini_targets="{}"'.format(port_list)
                if args.json:
                    output.update( { 'qla2xxx.conf': tmp_output } )
                else:
                    output += tmp_output + '\n'
        return verbose, output, err_out
    # End of parse_lspci

    #-----------------------------------------------------------------------------
    # Print all_output having 'T' or ' ' in front for targets that are now known.
    def all_output_T_before():
        nonlocal args, all_output, target_wwpns, ht

        if args.json:
            output = dict()
        else:
            output = ''
        if not args.lots and not args.full:
            return output

        if args.json:
            output = all_output
            j = 0
            for a in all_output:
                if ht[j] == '':
                    output[j].update( { 'target/host':'Initiator' } )
                elif ht[j] in target_wwpns.keys():
                    output[j].update( { 'target/host':'Target' } )
                else:
                    output[j].update( { 'target/host':'Initiator' } )
                j = j + 1
        else:
            j = 0
            for a in all_output:
                h = ' '
                if ht[j] == '':
                    pass
                elif ht[j] in target_wwpns.keys():
                    h = 'T'
                output += h + a + '\n'
                j = j + 1

        return output
    # End of all_output_T_before

    #-----------------------------------------------------------------------------
    def check_target_wwpn():
        nonlocal wwpn, target_wwpns, physical_slots, initiator_wwpns

        if args.json:
            err_out = []
        else:
            err_out = ''
        error = False
        # Check configuration specified wwpn's with what was found for targets.
        for i in sorted(wwpn.keys()):
            flag = 0
            for j in sorted(target_wwpns.keys()):
                if j == i:
                    flag = 1
                    break
            if flag == 0 and (args.full or args.lots):
                err_out = errmsg('ERROR: Did not find configuration WWPN {} in physically found target WWPNs.'.format(i), err_out)
                error = True

        # Print out any target WWPN not physically present.
        if error:
            output = all_output_T_before()
            n = []
            for i in sorted(physical_slots.keys()):
                n.append(physical_slots[i])
            if not args.json:
                print(output, file=sys.stderr)
                print('targets:', file=sys.stderr)
                print('  ' + '\n'.join(sorted(target_wwpns.keys())), file=sys.stderr)
                print('initiators:', file=sys.stderr)
                print('  ' + '\n'.join(sorted(initiator_wwpns.keys())), file=sys.stderr)
                print('physical_slots:', file=sys.stderr)
                print('  ' + '\n'.join(n), file=sys.stderr)
                print(err_out, file=sys.stderr)
            else:
                output.update( {'targets': sorted(target_wwpns.keys()) } )
                output.update( {'initiators': sorted(initiator_wwpns.keys()) } )
                output.update( {'physical slots': n } )
                output.update( {'stderr':err_out})
                print_json('', output)
            exit(1)
        return
    # End of check_target_wwpn

    #-----------------------------------------------------------------------------
    #-- This code left here -- but doesn't work with qla2xxx module loaded before
    #-- Linux has set slot numbers. (Like via initramfs.) Do not use slot notation. :)
    #--
    #-- def check_target_slots():
    #--     nonlocal slot, target_slots, physical_slots
    #--
    #--     i = sorted(slot.keys())
    #--     j = sorted(target_slots.keys())
    #--     if i != j:
    #--         print('Did not find specified slot.', file=sys.stderr)
    #--         print('Specified: ' + ' '.join(sorted(slot.keys())), file=sys.stderr)
    #--         print('Found: ' + ' '.join(sorted(target_slots.keys())), file=sys.stderr)
    #--         print('physical_slots:', file=sys.stderr)
    #--         m = sorted(physical_slots.keys())
    #--         n = []
    #--         for o in m:
    #--             n.append(physical_slots[o])
    #--         print('  ' + '\n  '.join(n), file=sys.stderr)
    #--         exit(0)
    #--     return
    #-- # End of check_target_slots

    #-----------------------------------------------------------------------------
    # Print out appropriate response depending upon argument to command.
    def print_output():
        nonlocal args, target_wwpns, initiator_wwpns, physical_slots

        # Output table format.
        output = all_output_T_before()
        if args.json:
            if output and output != '':
                output = { 'qlogic ports': output }
            else:
                output = dict()

        # Target output.
        if args.target or args.full:
            if args.json:
                output.update( {'targets': sorted(target_wwpns.keys()) } )
            elif args.full:
                output += 'targets:\n'
                output += '  ' + '\n  '.join(sorted(target_wwpns.keys())) + '\n'
            else:
                output += '\n'.join(sorted(target_wwpns.keys())) + '\n'

        # Blank line between targets/initiators to denote separation, if both specified.
        if (args.target and args.initiator) and not args.json:
            output += '\n'

        # Initiator output.
        if args.initiator or args.full:
            if args.json:
                output.update( {'initiators': sorted(initiator_wwpns.keys()) } )
            elif args.full:
                output += 'initiators:\n'
                output += '  ' + '\n  '.join(sorted(initiator_wwpns.keys())) + '\n'
            else:
                output += '\n'.join(sorted(initiator_wwpns.keys())) + '\n'

        # (non, kind of) Physical slot output.
        if args.full:
            m = sorted(physical_slots.keys())
            n = []
            for o in m:
                n.append(physical_slots[o]);
            if args.json:
                output.update( {"physical slots":n} )
            else:
                output += 'physical_slots:\n'
                output += '  ' + '\n  '.join(n) +'\n'

        return output
    # End of print_output

    #-----------------------------------------------------------------------------
    # Variables set from parsing QLogic configuration file.
    slot = dict()                               # Slots specifically mentioned.
    wwpn = dict()                               # WWPNs specifically mentioned (Targets).
    qle = True                                  # True if all QLE are to be initiators.
    #-----------------------------------------------------------------------------
    all_output = []                             # Accumulate output before printing it.
    #-----------------------------------------------------------------------------
    target_wwpns = dict()                       # WWPN's that are targets.
    initiator_wwpns = dict()                    # WWPN's that are initiators.
    physical_slots = dict()                     # Physical slots of WWPN's.
    #-- target_slots = dict()                       # Physical slots are targets. (Doesn't work.)
    ht = []                                     # Leading 'T' or ' ' for output lines.
    #-----------------------------------------------------------------------------
    # Translate device type into printable label.
    scsi_short_device_types = (
         'disk   ', 'tape   ', 'printer', 'process', 'worm   ', 'cd/dvd ',
         'scanner', 'optical', 'mediumx', 'comms  ', '(0xa)  ', '(0xb)  ',
         'storage', 'enclosu', 'sim dsk', 'opti rd', 'bridge ', 'osd    ',
         'adi    ', 'sec man', 'zbc    ', '(0x15) ', '(0x16) ', '(0x17) ',
         '(0x18) ', '(0x19) ', '(0x1a) ', '(0x1b) ', '(0x1c) ', '(0x1e) ',
         'wlun   ', 'no dev '
    )
    # main processing.
    args = parse_args(values)
    if args.json:
        output = dict()
    else:
        output = ''

    verbose, err_out = parse_qla2xxx_config_file()

    print_header()                  # Note: all_output contains the output.

    v1, o1, eo1 = parse_lspci()
    verbose += v1
    if type(o1) != str:
        output.update(o1)
    else:
        output += o1
    err_out += eo1
    # An extra new line between qla2xxx.conf line and following.
    if args.qla2xxx and (args.target or args.initiator) and not args.json:
        output += '\n'

    check_target_wwpn()             # Note: stderr printing/json and exit if errors.

#--    check_target_slots()

    o1 = print_output()
    if type(o1) != str:
        output.update(o1)
    else:
        output += o1

    if args.json:
        if verbose:
            output.update({'verbose':verbose})
        if err_out:
            output.update({'stderr':err_out})
    else:
        output = verbose + output
    return output, err_out
# End of main

#-----------------------------------------------------------------------------
# Execute the main routine.
if __name__ == '__main__':
    output, err_out = main(sys.argv[1:])
    if type(output) != str:
        print_json('', output)
    else:
        print(output, end='')
        print(err_out, file=sys.stderr, end='')
    exit(0)
#-----------------------------------------------------------------------------
# End of file get-slot-list.py
