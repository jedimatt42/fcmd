# SYSINFO - API test I1

Self-checking test for system information, SAMS information, the clock, and hex
conversions.

Covers: `sys_info`, `sys_sams_info`, `time_get`, `hex_from_uint`, `hex_to_int`.

## Type

`AUTO` for hex conversion, `sys_info`, and SAMS structure checks. `OBSERVE` for
the printed date/time.

## Expected result

- `hex_from_uint` produces the four uppercase hex digits of the input and
  `hex_to_int` parses them back to the original value.
- `hex_to_int` handles a leading space, `+`, `-`, invalid digits, and an empty
  string.
- `sys_info` reports a populated DSR list with `currentDsr` pointing inside it.
- `sys_sams_info` reports a non-negative page count. On a machine without SAMS
  the SAMS checks are skipped.
- `time_get` returns in-range fields, or the test skips when no clock is present.

The final line is `pass=N fail=N skip=N`. Any failure makes the program return
non-zero so ForceCommand reports it.

## Build and run

```
make -C example/gcc/sysinfo
```

Copy `SYSINFO` to a ForceCommand-visible device and run `SYSINFO`.
