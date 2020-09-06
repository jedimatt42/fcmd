# python tool to generate api table and c header

import sys
import re

def get_api_names():
    with open(sys.argv[1], 'r') as api_lst:
        return [ n.strip() for n in api_lst.readlines() if len(n) > 1 ]

def get_api_banks():
    bank_map = { }

    pat = re.compile(r"DECLARE_BANKED(_VOID)?\((\w+), BANK\((\d+)\)")

    with open(sys.argv[3], 'r') as api_banks:
        for line in api_banks.readlines():
            # b8_terminal.h:DECLARE_BANKED_VOID(tputc, BANK(8), bk_tputc, (int c), (c))
            m = pat.search(line)
            if m:
                bank_map[m.group(2)] = (int(m.group(3)) << 1) + 0x6000

    return bank_map


names = get_api_names()
banks = get_api_banks()

with open(sys.argv[2], 'w') as api_asm:
    api_asm.write(f"\tref\tfc_api\n")
    for name in names:
        api_asm.write(f"\tref\t{name}\n")
    api_asm.write("\n")

    api_asm.write(f"\tdata fc_api\n")
    for name in names:
        api_asm.write(f"\tdata {name},{banks[name]}\n")
