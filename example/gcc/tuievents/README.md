# TUIEVENTS - API test I36

Interactive test for TUI events, focus, and widget visibility/lifecycle.

## Type

`AUTO` for the dispatch/argument checks and focus transitions; `OBSERVE` for the
focus ring and the BREAK window close.

## Flow

1. TEXT40: `tui_dispatch_event(0)` and a `TUI_EV_NONE` event return 0; a
   `TUI_EV_KEY` event returns 1; `tui_get_event(0)` returns 0.
2. Two windows are opened: an empty `EMPTY` window first (oldest) and an
   `EVENTS` window with widgets on top.
3. Focus / lifecycle checks:
   - `tui_set_focus` moves focus between two buttons (`tui_get_focus`);
   - `tui_widget_disable` clears focus; `enable` restores usability;
   - `hide`/`show` do not clear focus;
   - `tui_widget_destroy` removes a widget and clears focus if it was focused.
4. Interactive event loop:
   - **FCTN-8 (REDO)** moves focus through the visible/enabled widgets (two
     buttons and a checkbox; the focused button shows `<...>` and the focused
     checkbox shows `< >`; SPACE toggles it);
   - **BREAK** closes the empty window (it is the oldest/tail window with no
     children);
   - **ENTER/SPACE** activates the focused widget (button/checkbox);
   - **Q** ends the loop.
5. Windows are closed, `tui_done`, summary, keypress wait.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`. Failed checks are listed
just before the summary.

## Build and run

```
make -C example/gcc/tuievents
```

Copy `TUIEVENTS` to a ForceCommand-visible device and run `TUIEVENTS`.
