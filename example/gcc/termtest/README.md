# TERMTEST - API test I7

Test for terminal output and colors.

Covers: `term_puts`, `term_putc`, `term_gotoxy`, `term_cls`,
`term_set_text_color`, `term_set_bg_color`, `term_set_border_color`.

## Type

`OBSERVE` for positions and colors; `AUTO` for the color-setter return codes.

## Semantics exercised

- `term_cls` clears the screen and homes the cursor.
- `term_gotoxy` uses 1-based coordinates and clamps out-of-range values.
- `term_putc` writes one character and interprets control codes such as `\n`.
- `term_set_text_color` / `term_set_bg_color` return the previous color.
- `term_set_border_color` returns 0 and changes only the border.
- The original colors are captured at startup and restored on exit.

## Expected result

- The final line is `pass=N fail=N skip=N` with `fail=0`.
- "top left" appears at row 1 column 1 and "middle" at row 5 column 20.
- Row 3 reads `putc: ABC`.
- The border turns magenta, waits for a key, then returns to black.
- On 80x30 F18A the two color rows differ; on global-color modes all text takes
  the last color set.

## Build and run

```
make -C example/gcc/termtest
```

Copy `TERMTEST` to a ForceCommand-visible device and run `TERMTEST`.
