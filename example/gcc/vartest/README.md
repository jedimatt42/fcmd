# VARTEST - API test I5

Test for system variables.

Covers: `var_set`, `var_get`.

## Type

`AUTO`. No hardware dependence and no display mode changes.

## Semantics exercised

- `var_set` then `var_get` round-trips a value.
- Names are case-insensitive; `var_set` and `var_get` uppercase the name in
  place, so only mutable name buffers are used.
- Setting an existing variable overwrites its value.
- An empty or `NULL` value removes the variable.
- An unset variable reads back as `(char*) -1`.
- Values up to 81 characters are accepted; longer values are rejected and the
  previous value is kept.
- Names longer than 11 characters are rejected and not stored.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`. The name/value limit
checks print an error line from `var_set`, which is expected.

## Build and run

```
make -C example/gcc/vartest
```

Copy `VARTEST` to a ForceCommand-visible device and run `VARTEST`.
