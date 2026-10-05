# GFXCIRCLE - API test I27

Interactive test for `gfx_circle`, `gfx_ellipse`, and their error codes.

## Type

`AUTO` for return codes and sampled points on/inside the shapes; `OBSERVE` for
the drawn shapes. Each mode announces what to look for and waits, then draws,
then waits again before returning to TEXT40.

## Flow

1. Starts in TEXT40 and verifies `gfx_circle`/`gfx_ellipse` return
   `GFX_ERR_WRONG_MODE`.
2. For each mode, in TEXT40 it prints `[OBSERVE] <MODE>: <what to look for>`,
   waits for a key, enters the mode and draws, waits for a key, then returns to
   TEXT40 and prints that mode's AUTO checks:
   - **GRAPHICS2** (every VDP): a circle and an ellipse; the four extremes are
     set and the center is background; zero/negative radii return
     `GFX_ERR_INVALID`, out-of-range geometry and colors return `GFX_ERR_RANGE`.
   - **MULTICOLOR** (every VDP): a small circle.
   - **GRAPHICS4** on V9958/V9938 (skipped otherwise): a circle and an ellipse.
   - **GRAPHICS5** on V9958/V9938 (skipped otherwise): a circle whose pixel
     width is doubled (aspect correction for the 512-wide mode) plus an ellipse.
3. Prints the summary and waits for a key before returning.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` (one `SKIP` when
GRAPHICS4/GRAPHICS5 are not available).

## Build and run

```
make -C example/gcc/gfxcircle
```

Copy `GFXCIRCLE` to a ForceCommand-visible device and run `GFXCIRCLE`.
