#!/usr/bin/python3
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#-----------------------------------------------------------------------------
import argparse
import sys

#-----------------------------------------------------------------------------
# Global option variables follow.
#-----------------------------------------------------------------------------
def parse_args(values):
    global args, initiator, target, qla2xxx

    init = ('i', 'in', 'ini', 'init', 'initi', 'initia', 'initiat', 'initiato', 'initiator', 'initiators')
    targ = ('t', 'ta', 'tar', 'targ', 'targe', 'target', 'targets')
    qla2 = ('q', 'ql', 'qla', 'qla2', 'qla2x', 'qla2xx', 'qla2xxx')

    initiator = False
    target = False
    qla2xxx = False

    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog= "List Linux host numbers, Fibre Channel WWPNs, Card type and Firmware versions,\n" +
                "PCI slot numbers, Speed and Supported Speeds, and Link States.\n" +
                "May be useful for setting up the file: /etc/modprobe.d/qla2xxx.conf\n")
    parser.add_argument('--verbose', '-v', action='store_true',
                        help = 'Print each line of qla2xxx.conf file.')
    parser.add_argument('--vv', '-vv', action='store_true',
                        help = 'Print each Fibre line of \'lspci -vmmD\' output.')
    parser.add_argument('--seen', '-s', action='store_true',
                        help = 'Print rports seen on target ports. (Positional arguments should not be present. See (nothing) above.)')
    parser.add_argument('rest', nargs='*',
                        metavar="initiator|target|qla2xxx",
                        help='Optional output or format limiting.')
    args = parser.parse_args(values)
    error = False
    for a in args.rest:
        if a in init:
            initiator = True
        elif a in targ:
            target = True
        elif a in qla2:
            qla2xxx = True
        else:
            if not error:
                print('-' * 78, file=sys.stderr)
            print("ERROR - unrecognized argument '{}'".format(a), file=sys.stderr)
            error = True
    if error:
        print('-' * 78, file=sys.stderr)
        parser.print_help()
        print('-' * 78, file=sys.stderr)
        print("ERROR - read line(s) above help message!", file=sys.stderr)
        exit(1)
# End of parse_args
#-----------------------------------------------------------------------------
def print_args():
    global args, initiator, target, qla2xxx

    print(type(args.verbose), "args.verbose=", args.verbose)
    print(type(args.vv), "args.vv=", args.vv)
    print(type(args.seen), "args.seen=", args.seen)
    print(type(args.rest), "args.rest=", args.rest)
    print(type(initiator), "initiator=", initiator)
    print(type(target), "target=", target)
    print(type(qla2xxx), "qla2xxx=", qla2xxx)
# End of print_args
#-----------------------------------------------------------------------------
# Main script processing.
def main(values):
    print("values=", values)
    parse_args(values)
    print_args()
# End of main
#-----------------------------------------------------------------------------

# Execute the main routine.
if __name__ == "__main__":
    main(sys.argv[1:])
    exit(0)
#-----------------------------------------------------------------------------
# End of file args.py
