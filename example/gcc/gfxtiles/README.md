# GFXTILES - API test I30

Interactive test for tile and sprite functions.

## Type

`AUTO` for return codes; `OBSERVE` for the tile row and sprite. Each mode
announces what to look for and waits, then draws, then waits again before
returning to TEXT40.

## Flow

1. Starts in TEXT40 and verifies the tile/sprite calls return
   `GFX_ERR_WRONG_MODE` (and `gfx_sprite_status(0)` returns `GFX_ERR_INVALID`).
2. For each mode, in TEXT40 it prints `[OBSERVE] <MODE>: <what to look for>`,
   waits for a key, enters the mode and draws, waits for a key, then returns to
   TEXT40 and prints that mode's AUTO checks:
   - **GRAPHICS1** (every VDP): define a tile pattern and draw a row of tiles;
     define a sprite pattern, place/move the sprite, change only its
     color/pattern with `gfx_sprite_color`/`gfx_sprite_pattern_index`, enable
     sprites and read the status; error codes for bad data size/null
     data/pattern range, tile range/color, sprite
     number/coordinates/pattern/color, partial-update ranges, and hide range.
   - **GRAPHICS2** (every VDP): `gfx_pattern_define`/`gfx_tile` return
     `GFX_ERR_WRONG_MODE`; a sprite is defined and shown.
3. Prints the summary and waits for a key before returning.

`gfx_sprite_status` collision/overflow bits are hardware-dependent; the test
checks the call succeeds and observes the sprite rather than asserting a
collision.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`. Failed checks, if any,
are listed just before the summary.

## Build and run

```
make -C example/gcc/gfxtiles
```

Copy `GFXTILES` to a ForceCommand-visible device and run `GFXTILES`.
