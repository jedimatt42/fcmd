# GFXPIXELS - API test I25

Automated test for the pixel primitives and the pixel cursor.

Covers: `gfx_set_cursor`, `gfx_get_cursor`, `gfx_pset`, `gfx_preset`,
`gfx_point`, and the `GFX_OP_PSET`/`PRESET`/`AND`/`OR`/`XOR` operators.

## Type

`AUTO` for the return codes, round-trips and operator results; `OBSERVE` for
the GRAPHICS2 dot pattern and the restored text screen.

## Flow

1. Starts in TEXT40 and verifies `gfx_pset`/`gfx_point` return
   `GFX_ERR_WRONG_MODE` there (no pixel plane in text mode).
2. **GRAPHICS2** (available on every VDP), 16 colors:
   - each operator is applied to an isolated pixel in its own 8x8 color group
     and read back with `gfx_point` (`PSET`, `PRESET`, `OR`, `AND`, `XOR`);
   - a `GFX_COLOR_DEFAULT` write uses the current foreground;
   - `gfx_set_cursor` moves the cursor and `gfx_pset` advances it;
   - out-of-range coordinates, invalid colors, and null pointers are rejected.
3. **MULTICOLOR** (available on every VDP): round-trip and XOR.
4. **GRAPHICS4** on V9958/V9938 (skipped otherwise): adjacent pixels share a
   byte, so each nibble is written and read independently.
5. Returns to TEXT40 and prints the results.

No terminal output happens while a graphics mode is active, so all checks are
captured during the mode and reported after returning to text.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` (one `SKIP` when
GRAPHICS4 is not available). The program waits for a key after printing the
summary, because ForceCommand restores the screen when it returns.

## Build and run

```
make -C example/gcc/gfxpixels
```

Copy `GFXPIXELS` to a ForceCommand-visible device and run `GFXPIXELS`.
