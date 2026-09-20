# STRFMT - API test I3

Test for numeric and BASIC string formatting.

Covers: `str_from_uint`, `str_from_float`, `str_from_basic`, and hexadecimal
edge cases of `hex_from_uint`.

## Type

`AUTO` for integer, hex, and BASIC-string checks. `OBSERVE` for the float text.

## Notes

- `str_from_uint` supports the full unsigned 16-bit range (0-65535) and prints
  `0` for zero.
- `str_from_basic` reads a TI BASIC string (length byte followed by characters),
  copies the characters to the destination, null terminates, and returns the
  length.
- `str_from_float` uses the radix-100 to decimal conversion in `ftoa`; only the
  zero case, sign, and leading digit are asserted automatically. Representative
  values are printed and must be confirmed by eye.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`, and the six printed float
values should look like sensible decimal numbers.

## Build and run

```
make -C example/gcc/strfmt
```

Copy `STRFMT` to a ForceCommand-visible device and run `STRFMT`.
