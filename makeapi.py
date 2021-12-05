# python tool to generate api table and c header

import sys
import re

def get_api_names():
    aliases = { }
    # return the map of function names included in the api => client alias
    with open(sys.argv[1], 'r') as api_lst:
        for line in [ n.strip() for n in api_lst.readlines() if len(n) > 1 ]:
            parts = line.split(' ')
            if len(parts) > 1:
                aliases[parts[0]] = parts[1]
            else:
                aliases[parts[0]] = parts[0]

    return aliases

def get_api_banks():
    # return a map of function name => bank switch address
    bank_map = { }

    pat = re.compile(r"DECLARE_BANKED(_VOID)?\((\w+), BANK\((\d+)\)")

    with open(sys.argv[3], 'r') as api_banks:
        for line in api_banks.readlines():
            # b8_terminal.h:DECLARE_BANKED_VOID(tputc, BANK(8), bk_tputc, (int c), (c))
            m = pat.search(line)
            if m:
                bank_map[m.group(2)] = (int(m.group(3)) << 1) + 0x6000

    return bank_map

def get_api_decls():
    # return map of function name => DECLARE_BANKED statements
    sig_map = { }

    pat = re.compile(r"DECLARE_BANKED_VOID?\((\w+),\s*BANK\(\d+\),\s*\w+,\s*(\([^\)]*\)),\s*(\([^\)]*\))\)")
    # group 1 : function name
    # group 2 : argument signatures
    # group 3 : argument names

    retpat = re.compile(r"DECLARE_BANKED?\((\w+),\s*BANK\(\d+\),\s*([^,]+),\s*\w+,\s*(\([^\)]*\)),\s*(\([^\)]*\))\)")
    # group 1 : function name
    # group 2 : return type
    # group 3 : argument signatures
    # group 4 : argument names

    with open(sys.argv[3], 'r') as api_banks:
        for line in api_banks.readlines():
            m = pat.search(line)
            if m:
                sig_map[m.group(1)] = ('void', m.group(2), m.group(3))
            else:
                m = retpat.search(line)
                if m:
                    sig_map[m.group(1)] = (m.group(2), m.group(3), m.group(4))

    return sig_map

# Load all the names from api.lst
aliases = get_api_names()
names = aliases.keys()
# build a map of the function name to bank address
banks = get_api_banks()

# Compose and write the api function lookup table assembly file
with open(sys.argv[2], 'w') as api_asm:
    api_asm.write(f"\tref\tfc_api\n")
    api_asm.write(f"\tref\tstramp\n")
    for name in names:
        api_asm.write(f"\tref\t{name}\n")
    api_asm.write("\n")

    api_asm.write(f"\tdata fc_api\n")
    api_asm.write(f"\tdata stramp\n")
    for name in names:
        api_asm.write(f"\tdata {name},{banks[name]}\n")

# load the signatures
signatures = get_api_decls()

# Create the client gcc include file
with open(sys.argv[4], 'w') as client_api_h:
    # copy the 'template' preample
    with open('fc_api_template', 'r') as preamble:
        for line in preamble.readlines():
            client_api_h.write(line)

    fn = 0x6084
    for name in names:
        # For each function define the table address
        #
        # #define FC_TPUTC 0x6082
        def_name = f'fc_{aliases[name]}'.upper()

        client_api_h.write(f'#define {def_name} {hex(fn)}\n')
        fn += 4

    for name in names:
        # For each function write something like:
        #
        # // declare function: void fc_tputc(int value);
        # DECL_FC_API_CALL(FC_TPUTC, fc_tputc, void, (int value), (value))
        sig = signatures[name]
        fc_name = f'fc_{aliases[name]}'
        client_api_h.write(f'\n// function: {sig[0]} {fc_name}{sig[1]}\n')
        client_api_h.write(f'DECL_FC_API_CALL({fc_name.upper()}, {fc_name}, {sig[0]}, {sig[1]}, {sig[2]})\n')

    # end the include file ifndef
    client_api_h.write('\n#endif\n')
