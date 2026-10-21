# TUIDRAW - API test I33

Interactive test for the TUI drawing primitives.

## Type

`AUTO` for `tui_init` errors and the screen metrics; `OBSERVE` for the drawn
text, lines, boxes, titles, and fills. Each mode announces what to look for,
waits, draws, waits, then returns to TEXT40 for the checks.

## Flow

1. TEXT40: initializes the TUI pool (`tui_init` with size 1 returns `-1`, with
   the pool size returns 0) and checks width 40 / height 24 / no per-character
   color.
2. For each mode, in TEXT40 it prints `[OBSERVE] <MODE>: <what to look for>`,
   waits, enters the mode, draws, waits, then returns to TEXT40 and prints the
   checks:
   - **TEXT40** (every VDP): `tui_gotoxy`, `tui_puts`, `tui_putc`, `tui_hline`,
     `tui_vline`, `tui_box`, `tui_box_title`, `tui_fill`.
   - **TEXT80** (V9938/V9958/F18A, else `SKIP`): 80-column text with the same
     box/title/fill calls; `tui_has_color()` is 0.
   - **F18A_TEXT80X30** (F18A, else `SKIP`): `tui_set_color` then colored text
     and a colored box/fill; `tui_has_color()` is 1, 80×30.
3. Calls `tui_done`, prints the summary, and waits for a key before returning.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` (up to two `SKIP`s when
TEXT80/F18A are not available). Failed checks, if any, are listed just before
the summary.

## Build and run

```
make -C example/gcc/tuidraw
```

Copy `TUIDRAW` to a ForceCommand-visible device and run `TUIDRAW`.
