#!/usr/bin/python3
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#-----------------------------------------------------------------------------
import sys
#-- import importlib
#-- name = importlib.import_module('get-slot-list')
name = __import__('get-slot-list')

#-----------------------------------------------------------------------------
# Execute the main routine.
if __name__ == "__main__":
    print(name.main(sys.argv[1:]), end='')
    print("-" * 70)

print(name.main(["qla2xxx", "-v"]), end='')
print("-" * 70)
print(name.main(["initiator"]), end='')
print("-" * 70)
print(name.main(["target"]), end='')
print("-" * 70)
print(name.main(["-vv", "--seen", "target"]), end='')
#-----------------------------------------------------------------------------
# End of file ga.py
