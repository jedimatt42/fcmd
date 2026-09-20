# STRTEST - API test I2

Self-checking test for the string utility functions.

Covers: `str_len`, `str_cmp`, `str_cmp_icase`, `str_copy`, `str_ncopy`,
`str_cat`, `str_set`, `str_index_of`, `str_last_index_of`, `str_startswith`,
`str_endswith`, `str_to_int`, `str_token`, `str_token_next`, `str_token_peek`.

## Type

`AUTO`. No hardware dependence and no display changes.

## Notes

- `str_token` and `str_token_peek` share an internal cursor, so they are driven
  in sequence within one test.
- `str_token` mutates its input in place, so mutable buffers are used.
- `str_set` does not null terminate; the test verifies a sentinel past the limit
  is left untouched.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`.

## Build and run

```
make -C example/gcc/strtest
```

Copy `STRTEST` to a ForceCommand-visible device and run `STRTEST`.
