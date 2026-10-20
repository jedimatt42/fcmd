# GFXPAL - API test I31

Conditional test for `gfx_palette_set` (F18A only).

## Type

`COND` / `AUTO` / `OBSERVE`. On non-F18A VDPs every call must return
`GFX_ERR_UNSUPPORTED`; the palette update and swatches run only on the F18A.
On the F18A the test is interactive (default swatches, then new colors).

## Flow

- **Non-F18A (9918A, V9938/V9958):** three calls (valid and invalid arguments)
  all return `GFX_ERR_UNSUPPORTED`; a `SKIP` notes that palette changes require
  the F18A.
- **F18A:**
  1. In TEXT40 it explains that six swatches will show the default palette,
     then the new colors after a key.
  2. Enters GRAPHICS2 and draws six filled swatches in colors 8–13.
  3. Waits for a key to observe the default colors.
  4. Sets palette entries 8–13 to red/green/blue/yellow/cyan/magenta and
     checks `gfx_palette_set(63, ...)` is accepted; index/r/g/b out of range
     return `GFX_ERR_RANGE`.
  5. Waits for a key to observe the changed colors, returns to TEXT40, prints
     the results, then waits before returning.

The swatches use color indices 8–13 so the harness text (white on black) is not
affected by the palette change.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` (one `SKIP` on non-F18A
VDPs). Failed checks, if any, are listed just before the summary.

## Build and run

```
make -C example/gcc/gfxpal
```

Copy `GFXPAL` to a ForceCommand-visible device and run `GFXPAL`.
