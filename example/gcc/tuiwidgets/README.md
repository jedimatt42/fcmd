# TUIWIDGETS - API test I35

Interactive test for the TUI widget creation/accessor APIs.

## Type

`AUTO` for creation, list index/count behavior, text field/checkbox values, and
the widget OOM path; `OBSERVE` for the rendered widgets.

## Flow

1. TEXT40: a pool large enough for a window but not a widget makes
   `tui_button_create` return 0 (OOM); the full pool is then reinitialized.
2. **TEXT40** (every VDP): one window contains:
   - a button (`OK` changed to `Go`) and a label (`Name:` changed to `Name`);
   - a list: three adds return 0/1/2, count 3; insert at 1 returns 1; select 2;
     remove(2) drops the selection to -1; clear gives count 0/-1; adding to a
     non-list and inserting past the end return -1; final items `one`/`two`/
     `three` with item 2 selected;
   - a text field set to `hello` and read back; cursor clamped to the end;
   - a checkbox set 1/0/5 and read back (1/0/1);
   - a progress bar at 75%.
3. **F18A_TEXT80X30** (F18A, else `SKIP`): a colored window with a label,
   button, checked checkbox, and progress bar.
4. `tui_done`, summary, keypress wait.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` (one `SKIP` when F18A is
not available). Failed checks are listed just before the summary.

## Build and run

```
make -C example/gcc/tuiwidgets
```

Copy `TUIWIDGETS` to a ForceCommand-visible device and run `TUIWIDGETS`.
