#!/usr/bin/python3
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4

def here():
    physical_slots = []                         # Physical slots of WWPN's.
    k = 1

    def parse_lspci():
        nonlocal physical_slots

        # k = 1
        print("#0 type(physical_slots)=", type(physical_slots), "physical_slots=", physical_slots)
        print("#0 type(k)=", type(k), "k=", k)
        #? physical_slots[k] = { "hi":k, "there": "."}
        physical_slots.append({ "hi":k, "there": "."})

    print("#1 type(physical_slots)=", type(physical_slots), "physical_slots=", physical_slots)
    k = 1
    parse_lspci()
    print("#2 type(physical_slots)=", type(physical_slots), "physical_slots=", physical_slots)

    k = 4
    parse_lspci()
    print("#3 type(physical_slots)=", type(physical_slots), "physical_slots=", physical_slots)

here()
