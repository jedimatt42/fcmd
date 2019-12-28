# python tool to generate api table and c header

import sys
import re

def get_api_names():
    with open(sys.argv[1], 'r') as api_lst:
        return [ n.strip() for n in api_lst.readlines() if len(n) > 1 ]

def get_api_banks():
    bank_map = {}
    bank_addrs = {}

    with open('banks.h', 'r') as banksh:
        bankdef = re.compile('^#define (BANK_\d) (0x60\d\d)')
        for line in banksh.readlines():
            m = bankdef.search(line)
            if m:
                bank_addrs[m.group(1)] = m.group(2)

    with open(sys.argv[2], 'r') as api_banks:
        for line in api_banks.readlines():
            bank_map[line.split(',')[0].strip()] = bank_addrs.get(line.split(',')[1].strip(), '0x6000')
    return bank_map

names = get_api_names()
banks = get_api_banks()

with open(sys.argv[3], 'w') as api_asm:
    for n in names:
        api_asm.write("\tref\t{}\n".format(n))
    api_asm.write("\n")
    for n in names:
        api_asm.write("\tdef\tapi_{}\n".format(n))
    api_asm.write("\n")
    for n in names:
        api_asm.write("api_{}\tdata {},{}\n".format(n,n, banks.get(n, '0x6000')))
