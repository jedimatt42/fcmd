#!/usr/bin/env python3
"""Generate linkfile.m4 from source files and linkfile.m4.in template.

Bank assignment is determined per-source-file:
- .c files: parsed for #define MYBANK BANK(N)
- .asm files: explicitly listed in ASM_BANKS dict
"""

import os
import re
import sys

SRC_DIR = 'src'
LIBTI99_DIR = os.path.join(SRC_DIR, 'libti99')
TEMPLATE = 'linkfile.m4.in'

MYBANK_RE = re.compile(r'#define\s+MYBANK\s+BANK\s*\(\s*(\d+)\s*\)')

EXPLICIT_ASM = {
    'header_cart.asm', 'header_console.asm',
    'trampoline.asm', 'trampoline_alt.asm',
}

ASM_BANKS = {
    'fc_cc_runtime.asm': 0,
    'sams_tramp.asm': 0,
    'fg99.asm': 1,
    'oem_high_chars.asm': 3,
    'oem_low_chars.asm': 3,
    'pointer.asm': 7,
}

GENERATED_OBJS = {
    'api.o': 0,
    'fcbanner.o': 3,
}


def get_bank(name, source_path):
    if source_path.endswith('.c'):
        with open(source_path) as f:
            content = f.read()
        m = MYBANK_RE.search(content)
        if m:
            return int(m.group(1))
    return None


def scan_sources():
    banks = {i: [] for i in range(16)}

    for basedir in (SRC_DIR, LIBTI99_DIR):
        if not os.path.isdir(basedir):
            continue
        for f in sorted(os.listdir(basedir)):
            if not f.endswith(('.c', '.asm')):
                continue
            if f in EXPLICIT_ASM:
                continue
            name = os.path.splitext(f)[0]
            source = os.path.join(basedir, f)
            if f in ASM_BANKS:
                bank = ASM_BANKS[f]
            else:
                bank = get_bank(name, source)
            if bank is not None:
                banks[bank].append(name + '.o')

    for obj, bank in GENERATED_OBJS.items():
        if obj not in banks[bank]:
            banks[bank].append(obj)

    return banks


def generate_bank_sections(banks):
    sections = []
    for i in range(16):
        objs = banks.get(i, [])
        if not objs:
            continue
        lines = [f'    .bank{i} {{']
        for obj in sorted(objs):
            lines.append(f'      OBJDIR/{obj}(.text)')
        if i == 0:
            lines.append('      __LOAD_DATA = .;')
        lines.append('    }')
        sections.append('\n'.join(lines))
    return '\n'.join(sections)


def main():
    output = sys.argv[1] if len(sys.argv) > 1 else 'linkfile.m4'

    banks = scan_sources()
    bank_content = generate_bank_sections(banks)

    with open(TEMPLATE) as f:
        template = f.read()

    result = template.replace('@BANK_SECTIONS@', bank_content)

    with open(output, 'w') as f:
        f.write(result)

    total = sum(len(v) for v in banks.values())
    print(f'Generated {output}: {total} objects across {sum(1 for v in banks.values() if v)} banks')


if __name__ == '__main__':
    main()
