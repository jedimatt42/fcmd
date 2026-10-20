# GFXATTR - API test I32

Conditional test for text attributes and tile semantics.

## Type

`COND` / `AUTO` / `OBSERVE`. Tile functions are checked against each text /
graphics mode; the F18A per-character attribute display runs only on the F18A.

## Flow

1. TEXT40: `gfx_tile` and `gfx_pattern_define` return `GFX_ERR_WRONG_MODE`.
2. For each available mode, in TEXT40 it prints `[OBSERVE] <MODE>...`, waits,
   enters the mode, draws, waits, returns to TEXT40, then prints the checks:
   - **MULTICOLOR** (every VDP): tile functions return `GFX_ERR_WRONG_MODE`.
   - **TEXT80** (V9938/V9958/F18A only; otherwise `SKIP`): 80 columns, no
     `GFX_CAP_TILES`/`GFX_CAP_ATTRIBUTES`; tile functions return
     `GFX_ERR_WRONG_MODE`.
   - **F18A_TEXT80X30** (F18A only; otherwise `SKIP`): 80×30 with
     `GFX_CAP_TEXT|TILES|ATTRIBUTES`; `gfx_tile` writes letters A–F in six
     different colors (per-character attributes); out-of-range x/y/pattern/color
     return `GFX_ERR_RANGE`.
3. Prints the summary and waits for a key before returning.

GRAPHICS1 tile drawing is covered by I30 (`GFXTILES`).

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` (up to two `SKIP`s when
TEXT80/F18A are not available). Failed checks, if any, are listed just before
the summary.

## Build and run

```
make -C example/gcc/gfxattr
```

Copy `GFXATTR` to a ForceCommand-visible device and run `GFXATTR`.
