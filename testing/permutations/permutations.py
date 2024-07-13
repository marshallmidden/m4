#!/usr/bin/python3 -B
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4

import sys
import itertools

my_str_z = "sbtlz"
my_str_a = "sbtla"

def the_perms(pre, mystr):
    chars = list(mystr)
    d = {}
    for i in range(1,len(mystr)):
        for comb in itertools.combinations(chars, i):
            d[pre + ''.join(comb)] = 1
        # rof
    # rof

    d[pre + mystr] = 1
    return d
# End of the_perms

D = the_perms('', my_str_z)
D.update(the_perms('d', my_str_z))
D.update(the_perms('dd', my_str_z))
D.update(the_perms('', my_str_a))
D.update(the_perms('d', my_str_a))
D.update(the_perms('dd', my_str_a))
print("type(D)={} D={}".format(type(D), D), file=sys.stderr)

sys.exit(0)
