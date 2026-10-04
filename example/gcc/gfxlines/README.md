# GFXLINES - API test I26

Automated test for `gfx_line` and its styles.

Covers: `gfx_line` in normal, box, filled, and box+filled styles, out-of-range
coordinates, invalid styles/colors, and read-backs through `gfx_point`.

## Type

`AUTO` for every return code and sampled pixel; `OBSERVE` for the drawn shapes.
The test is interactive: each mode announces what to look for and waits, then
draws, then waits again before returning to TEXT40.

## Flow

1. Starts in TEXT40 and verifies `gfx_line` returns `GFX_ERR_WRONG_MODE`.
2. For each mode, in TEXT40 it prints `[OBSERVE] <MODE>: <what to look for>`,
   waits for a key, enters the mode and draws, waits for a key, then returns to
   TEXT40 and prints that mode's AUTO checks:
   - **GRAPHICS2** (available on every VDP): a horizontal line, a diagonal line,
     a hollow box, and two filled rectangles. `GFX_LINE_FILL` and
     `GFX_LINE_BOX|GFX_LINE_FILL` draw the same filled area (the box border is a
     redundant repaint of the fill), so the two are expected to look identical.
     Out-of-range endpoints, invalid style bits, and invalid color are rejected.
   - **MULTICOLOR** (available on every VDP): a single horizontal line.
   - **GRAPHICS4** on V9958/V9938 (skipped otherwise): a diagonal line.
3. Prints the summary and waits for a key before returning.

No terminal output happens while a graphics mode is active, so all checks are
captured during the mode and reported after returning to text.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` (one `SKIP` when
GRAPHICS4 is not available). The program waits for a key after printing the
summary, because ForceCommand restores the screen when it returns.

## Build and run

```
make -C example/gcc/gfxlines
```

Copy `GFXLINES` to a ForceCommand-visible device and run `GFXLINES`.
