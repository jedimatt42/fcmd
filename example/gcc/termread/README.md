# TERMREAD - API test I8

Test for interactive terminal input and display helpers.

Covers: `term_gets`, `term_kscan`, `term_drop_down`, `term_set_identify_hook`.

## Type

`AUTO` for the identify hook and the input buffer bound. `OBSERVE` for the
entered text, the scan code, and the drop-down bar.

## Steps

1. The identify hook is installed and a `CSI 6n` request is emitted. The hook
   must be called exactly once with the `0x8000` marker set.
2. `term_gets` prompts for text. Enter a few characters and optionally use
   backspace, then press ENTER. The echoed text must match.
3. `term_kscan(5)` waits for a fresh key and prints its mode-5 scan code.
4. `term_drop_down(3)` draws three framed blank rows and a bar row spanning the
   window; press a key to continue.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`.

## Build and run

```
make -C example/gcc/termread
```

Copy `TERMREAD` to a ForceCommand-visible device and run `TERMREAD`.
