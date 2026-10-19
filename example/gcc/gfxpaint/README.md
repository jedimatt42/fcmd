# GFXPAINT - API test I28

Interactive test for `gfx_paint` (flood fill) and `gfx_draw` (vector commands).

## Type

`AUTO` for return codes and sampled pixels; `OBSERVE` for the filled box and
vector lines. Each mode announces what to look for and waits, then draws, then
waits again before returning to TEXT40.

## Flow

1. Starts in TEXT40 and verifies `gfx_paint`/`gfx_draw` return
   `GFX_ERR_WRONG_MODE`.
2. For each mode, in TEXT40 it prints `[OBSERVE] <MODE>: <what to look for>`,
   waits for a key, enters the mode and draws, waits for a key, then returns to
   TEXT40 and prints that mode's AUTO checks:
   - **GRAPHICS2** (every VDP): a hollow box (border 7) flood-filled with 5;
     interior/outside samples; repeat fill is a no-op; vector commands
     `M x,y`, `R`, `D`, `E`, `B` (move) and a default distance of 1; invalid
     command/`M` without coordinates/null string → `GFX_ERR_INVALID`; invalid
     color → `GFX_ERR_RANGE`; unbounded fill → `GFX_ERR_BUSY`.
   - **MULTICOLOR** (every VDP): paint with the border preserved (per-dot
     colors) and a vector line.
   - **GRAPHICS4** on V9958/V9938 (skipped otherwise): paint with preserved
     border and a diagonal vector line.
3. Prints the summary and waits for a key before returning.

In GRAPHICS2 the color is per 8x8 group, so the fill also sets the border
pixels' group color; the border-keeps-color assertions are only made in the
per-pixel modes. `gfx_paint` uses a fixed 256-entry stack, so the "successful"
fill regions are deliberately small; the `GFX_ERR_BUSY` case is an unbounded
fill.

Any failed checks are listed again just before the `pass=/fail=` summary, so
they remain visible on the final screen.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` (one `SKIP` when
GRAPHICS4 is not available).

## Build and run

```
make -C example/gcc/gfxpaint
```

Copy `GFXPAINT` to a ForceCommand-visible device and run `GFXPAINT`.
