# GFXMODES - API test I24

Interactive test for graphics mode selection and capability reporting.

Covers: `gfx_screen`, `gfx_get_info`, `gfx_color`, `gfx_clear`.

## Type

`AUTO` for return codes, mode echo, capabilities, and error paths; `OBSERVE`
for each mode's representative picture.

## Flow

1. The test switches to the common-safe **40-column text mode** first.
2. It reports the detected VDP type and lists every mode as supported or
   unsupported for that VDP.
3. For each mode it announces the next mode, waits for a key, enters it, and
   draws representative content (text, tiles, or lines/circle).
4. Text modes wait for a key to return. Graphics modes are shown for about three
   seconds and then return automatically; the console keyboard scan is not safe
   while a graphics mode is active, so it is only used in text modes.
5. It returns to TEXT40, then prints the AUTO checks for that mode.
6. After the walk it runs the error-path checks and finishes in TEXT40.

Useful when swapping emulator VDP configurations: the availability list tells
you what to expect before each mode is entered.

## Semantics exercised

- Supported modes return `GFX_OK` and report the requested mode and the expected
  capability bits; unsupported modes return `GFX_ERR_UNSUPPORTED` (including the
  V9958-only YJK modes on other VDPs).
- `gfx_color` accepts valid colors and rejects out-of-range ones.
- `gfx_clear` clears supported modes and returns `GFX_ERR_UNSUPPORTED` for YJK
  modes.
- Error paths: invalid sprite mode returns `GFX_ERR_INVALID`; an unknown mode
  and unknown flag bits return `GFX_ERR_UNSUPPORTED`; on the F18A,
  `F18A_TEXT80X30` without 16x16 sprites is rejected.

## Notes

The program uses a non-screen-safe header so ForceCommand restores the display
on return. Terminal text is only printed while in a text mode.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`.

## Build and run

```
make -C example/gcc/gfxmodes
```

Copy `GFXMODES` to a ForceCommand-visible device and run `GFXMODES`.
