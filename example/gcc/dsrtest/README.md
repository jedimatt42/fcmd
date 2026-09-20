# DSRTEST - API test I14

Test for DSR file operations.

Covers: `dsr_find`, `dsr_open`, `dsr_close`, `dsr_write`, `dsr_read`,
`dsr_read_cpu`, `dsr_status`, `dsr_reset`, `dsr_delete`.

## Type

`AUTO`, requires a writable current device. If the device is not writable the
file part is skipped (one skip).

## Semantics exercised

- `dsr_find` returns the current DSR for its name/crubase and 0 for an unknown
  name.
- `dsr_open` / `dsr_close` a DISPLAY VARIABLE file named `ZZDSRTST` in the
  current path.
- `dsr_write` stores three records.
- `dsr_status` returns no error (errors are encoded as `result << 8`).
- `dsr_read_cpu` reads the records back in order.
- `dsr_reset` rewinds so the first record is read again.
- `dsr_delete` removes the file; a subsequent open fails.

## Notes

Any pre-existing `ZZDSRTST` is deleted first, and the file is deleted at the
end. If the test is interrupted, the leftover file can be deleted with
`delete ZZDSRTST`.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` on a writable device.

## Build and run

```
make -C example/gcc/dsrtest
```

Copy `DSRTEST` to a ForceCommand-visible device and run `DSRTEST`.
