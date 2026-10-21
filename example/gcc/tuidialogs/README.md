# TUIDIALOGS - API test I37

Interactive test for the TUI dialogs and menus.

## Type

`AUTO` for menu creation/selection/destroy and dialog return codes;
`OBSERVE` for the modal dialogs.

## Flow

1. **Menu (automatic + interactive):**
   - `tui_menu_create` with null items or zero count returns 0;
   - a 3-item menu opens with the first item selected;
   - `tui_menu_get_selected(0)` returns -1;
   - UP/DOWN move the menu selection (via the focused list and
     `tui_get_event`); **Q** continues. The final selection must be 0–2;
   - `tui_menu_destroy` (and a null destroy) close it.
2. **Message box:** ENTER on OK (or BREAK). Always returns 0.
3. **Confirm box:** ENTER on Yes returns 1; FCTN-8 then ENTER on No, or BREAK,
   returns 0.
4. **Input box:** prefilled with `hello`; edit then ENTER returns 1 and copies
   the field, or BREAK returns 0. The resulting buffer is printed.
5. Prints the checks, `tui_done`, summary, keypress wait.

The TUI pool is reinitialized before each dialog because widget allocation has
no free.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`. Failed checks are listed
just before the summary.

## Build and run

```
make -C example/gcc/tuidialogs
```

Copy `TUIDIALOGS` to a ForceCommand-visible device and run `TUIDIALOGS`.
