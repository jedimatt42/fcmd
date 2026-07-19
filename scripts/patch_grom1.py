#!/usr/bin/env python3
"""Patch stock GROM 1 to add a powerup routine that branches to FCMD.

Reads the 6K stock GROM 1, pads it to 8K, and patches:
  - Header bytes 4-5 → powerup list pointer
  - Powerup list → points to boot routine
  - Boot routine → writes CLR @>0000; B @_START to scratchpad
                   via GPL dst instructions, then xml >F0

The GPL routine at offset 0x1804 constructs the following TMS9900 code
in scratchpad (>8320):
  >8320: CLR @>0000     (select FCMD bank 0)
  >8324: B  @_START     (jump to FCMD entry point)
then stores >8320 at >8300 and executes xml >F0 to branch there.
"""

import argparse


def main():
    parser = argparse.ArgumentParser(
        description="Patch GROM 1 for console ROM replacement")
    parser.add_argument("--input", required=True,
                        help="Stock GROM 1 binary (994a_grom1.bin)")
    parser.add_argument("--start", required=True,
                        help="_start address in hex (e.g. 0238)")
    parser.add_argument("--output", required=True,
                        help="Output patched GROM 1 binary")
    args = parser.parse_args()

    start_addr = int(args.start, 16)

    with open(args.input, "rb") as f:
        grom = bytearray(f.read())

    # Pad from 6K to 8K with zeros
    grom.extend([0] * (8192 - len(grom)))

    # GROM 1 base address is >2000.
    # Header addresses are absolute GROM-space addresses.
    # Offset 0x1800 → GROM address >2000 + 0x1800 = >3800
    GROM1_BASE = 0x2000

    # ---- Header (bytes 4-5): powerup list pointer ----
    grom_addr = GROM1_BASE + 0x1800
    grom[4] = (grom_addr >> 8) & 0xFF
    grom[5] = grom_addr & 0xFF

    # ---- Powerup list at offset 0x1800 ----
    #   00 00              (null: no more entries)
    #   boot_addr          (GROM address of boot routine)
    boot_addr = GROM1_BASE + 0x1804
    grom[0x1800] = 0x00
    grom[0x1801] = 0x00
    grom[0x1802] = (boot_addr >> 8) & 0xFF
    grom[0x1803] = boot_addr & 0xFF

    # ---- Boot routine at offset 0x1804 ----
    # Write TMS9900 code to scratchpad, then XML >F0 to execute it.
    #
    # GPL DST writes 16-bit values to scratchpad RAM, constructing:
    #   >8320: 04E0 0000  → CLR @>0000    ; select FCMD bank 0
    #   >8324: 0460 START → B   @ START   ; jump to _start
    # then stores >8320 at >8300 for XML >F0.
    bo = 0x1804
    grom[bo]   = 0xBF; grom[bo+1] = 0x20; grom[bo+2] = 0x04; grom[bo+3] = 0xE0
    grom[bo+4] = 0xBF; grom[bo+5] = 0x22; grom[bo+6] = 0x00; grom[bo+7] = 0x00
    grom[bo+8] = 0xBF; grom[bo+9] = 0x24; grom[bo+10]= 0x04; grom[bo+11]= 0x60
    grom[bo+12]= 0xBF; grom[bo+13]= 0x26
    grom[bo+14]= (start_addr >> 8) & 0xFF; grom[bo+15]= start_addr & 0xFF
    grom[bo+16]= 0xBF; grom[bo+17]= 0x00; grom[bo+18]= 0x83; grom[bo+19]= 0x20
    grom[bo+20]= 0x0F; grom[bo+21]= 0xF0

    with open(args.output, "wb") as f:
        f.write(grom)

    print(f"Patched GROM 1 written to {args.output}")
    print(f"  _start address: >{args.start}")
    print(f"  Powerup list at offset 0x1800")
    print(f"  Boot routine at offset 0x1804")


if __name__ == "__main__":
    main()
