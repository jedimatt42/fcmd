#!/usr/bin/env python3
"""Patch stock GROM 1 with the ForceCommand power-up GPL routine.

Reads the 6K stock GROM 1, pads it to 8K, sets header bytes 4-5 to the
power-up list at >3800, and copies the assembled GPL blob (src/gpl-powerup.g99)
to GROM1 offset 0x1800.

The GPL routine first checks whether F is being held down. If it is, it
returns to the console. Otherwise it constructs the following TMS9900 code in
scratchpad (>8320) and executes it:

  >8320: CLR @>0000     (select FCMD bank 0)
  >8324: B  @_START     (jump to FCMD entry point)

The GPL blob is assembled by the Makefile with xga99; _START is supplied from
the ForceCommand mapfile.
"""

import argparse


GROM1_BASE = 0x2000
POWERUP_OFFSET = 0x1800
POWERUP_ADDR = GROM1_BASE + POWERUP_OFFSET
GROM_SIZE = 8192


def main():
    parser = argparse.ArgumentParser(
        description="Patch GROM 1 for console ROM replacement")
    parser.add_argument("--input", required=True,
                        help="Stock GROM 1 binary (994a_grom1.bin)")
    parser.add_argument("--boot", required=True,
                        help="Assembled GPL power-up blob (gpl-powerup.grom)")
    parser.add_argument("--output", required=True,
                        help="Output patched GROM 1 binary")
    args = parser.parse_args()

    with open(args.input, "rb") as f:
        grom = bytearray(f.read())

    if len(grom) > GROM_SIZE:
        raise SystemExit(f"error: GROM 1 image is larger than {GROM_SIZE} bytes")

    # Pad from 6K to 8K with zeros.
    grom.extend([0] * (GROM_SIZE - len(grom)))

    with open(args.boot, "rb") as f:
        boot = f.read()

    if not boot:
        raise SystemExit("error: assembled GPL power-up blob is empty")
    if POWERUP_OFFSET + len(boot) > GROM_SIZE:
        raise SystemExit(
            f"error: GPL power-up blob ({len(boot)} bytes) does not fit in GROM 1")

    # ---- Header (bytes 4-5): power-up list pointer ----
    grom[4] = (POWERUP_ADDR >> 8) & 0xFF
    grom[5] = POWERUP_ADDR & 0xFF

    # ---- Power-up list and boot routine at offset 0x1800 ----
    grom[POWERUP_OFFSET:POWERUP_OFFSET + len(boot)] = boot

    with open(args.output, "wb") as f:
        f.write(grom)

    print(f"Patched GROM 1 written to {args.output}")
    print(f"  Power-up list at >{POWERUP_ADDR:04X}")
    print(f"  GPL routine at >{POWERUP_ADDR + 4:04X}")
    print(f"  GPL blob size: {len(boot)} bytes")


if __name__ == "__main__":
    main()
