# GFXPAGES - API test I29

Interactive test for `gfx_set_page` and `gfx_copy`.

## Type

`AUTO` for return codes and sampled pixels; `OBSERVE` for the copied content.

## Flow

1. Starts in TEXT40 and verifies `gfx_set_page` returns
   `GFX_ERR_UNSUPPORTED` there.
2. For each mode, in TEXT40 it prints `[OBSERVE] <MODE>: <what to look for>`,
   waits for a key, enters the mode and draws, waits for a key, then returns to
   TEXT40 and prints that mode's AUTO checks:
   - **GRAPHICS2** (every VDP): `gfx_set_page` and `gfx_copy` return
     `GFX_ERR_UNSUPPORTED`.
   - **GRAPHICS4** on V9958/V9938 (skipped otherwise): a solid box drawn on
     page 1 is copied across pages to page 0; an overlapping same-page copy of a
     gradient line is checked to prove the copy direction is overlap-safe;
     error paths (`null` → `GFX_ERR_INVALID`, out-of-range rect/page →
     `GFX_ERR_RANGE`, `display != draw` → `GFX_ERR_UNSUPPORTED`).
   - **GRAPHICS5** on V9958/V9938 (skipped otherwise): page independence
     (a box on page 1, an empty page 0).
3. Prints the summary and waits for a key before returning.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` (one `SKIP` when
GRAPHICS4/GRAPHICS5 are not available). Failed checks, if any, are listed just
before the summary.

## Build and run

```
make -C example/gcc/gfxpages
```

Copy `GFXPAGES` to a ForceCommand-visible device and run `GFXPAGES`.
