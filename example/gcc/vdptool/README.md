# VDPTOOL - API test I9

Test for direct VDP cursor addressing and character writes.

Covers: `vdp_cursor_addr`, `vdp_setchar`.

## Type

`AUTO` for address strides and byte round-trips; `OBSERVE` for the visible `Q`.

## Semantics exercised

- The test begins with `term_cls` so every row is blank and known.
- `vdp_cursor_addr` tracks `term_gotoxy`: adjacent columns differ by one cell,
  and moving down a row adds the full row stride (32, 40, or 80).
- `vdp_setchar` writes a byte at a VDP address; the value is read back through
  the data port using `vdp_readchar` from `ioports.h`.
- The character to observe is written at row 20 and the cursor is moved to row 5
  before any status text is printed, so the test output cannot cover it.
- The original byte is saved and restored so the screen is left unchanged.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`. A `Q` appears at row 20
column 1 until a key is pressed, after which that cell returns to blank.

## Build and run

```
make -C example/gcc/vdptool
```

Copy `VDPTOOL` to a ForceCommand-visible device and run `VDPTOOL`.
