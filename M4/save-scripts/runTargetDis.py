#!/usr/bin/env python3
from hbadiscovery import HBADiscovery
hbaDisc=HBADiscovery()

if not hbaDisc.updateTargetHBAs():
    print("Update of HBAs failed")
    exit(1)