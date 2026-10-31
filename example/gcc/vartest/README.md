# VARTEST - API test I5

Test for system variables.

Covers: `var_set`, `var_get`.

## Type

`AUTO`. No hardware dependence and no display mode changes.

## Semantics exercised

- `var_set` then `var_get` round-trips a value.
- Names are case-insensitive. `var_get` and `var_set` copy the name before
  uppercasing it, so string literals and mutable buffers are both safe.
- `var_get(name, buf, limit)` copies into the caller's buffer and returns 1
  when found, 0 when not found. It never returns a pointer into the dictionary.
- A small buffer is truncated safely and NUL-terminated.
- Setting an existing variable overwrites its value.
- An empty or `NULL` value removes the variable.
- An unset variable returns 0 and leaves the destination empty.
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
