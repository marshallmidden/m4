#!/usr/bin/python3
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4

#-----------------------------------------------------------------------------
def here():
    a = None

    if a:
        print("if a:  is true")
    else:
        print("if a:  is false")

    if not a:
        print("if not a:  is true")
    else:
        print("if not a:  is false")
#-----------------------------------------------------------------------------
here()
