#!/usr/bin/python3
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#-----------------------------------------------------------------------------
import args
import sys
#-----------------------------------------------------------------------------
# Execute the main routine.
if __name__ == "__main__":
    args.main(sys.argv[1:])
    print("-" * 70)

args.main(["qla2xxx", "-v"])
print("-" * 70)
args.main(["initiator"])
print("-" * 70)
args.main(["target"])
print("-" * 70)
args.main(["-vv", "--seen", "target"])
#-----------------------------------------------------------------------------
# End of file args.py
