# TUIWIN - API test I34

Interactive test for TUI windows and the window pool.

## Type

`AUTO` for window creation/OOM and screen-mode return codes; `OBSERVE` for the
drawn windows, titles, borders, colors, text, and scrolling.

## Flow

1. TEXT40: a pool too small for a window makes `tui_win_open` return 0 (OOM),
   then the full pool is reinitialized.
2. For each mode, announce what to look for, wait, draw, wait, then return to
   TEXT40 for the checks:
   - **TEXT40** (every VDP): window A opened, titled, moved, resized, border
     changed to top+bottom only, redrawn with a new title, filled with lines,
     `tui_win_printf` (`%s=%d`), `tui_win_putc`/`tui_win_puts`, then scrolled up
     one line; a second window B is opened overlapping A; both are closed.
   - **TEXT80** (V9938/V9958/F18A, else `SKIP`): a titled 80-column window with
     text.
   - **F18A_TEXT80X30** (F18A, else `SKIP`): a window with `tui_win_set_colors`
     (yellow on dark blue) and per-character color text.
3. `tui_done`, summary, keypress wait.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` (up to two `SKIP`s when
TEXT80/F18A are not available). Failed checks, if any, are listed just before
the summary.

## Build and run

```
make -C example/gcc/tuiwin
```

Copy `TUIWIN` to a ForceCommand-visible device and run `TUIWIN`.
