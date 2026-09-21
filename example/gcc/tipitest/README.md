# TIPITEST - API test I20

Test for TIPI messaging.

Covers: `tipi_on`, `tipi_off`, `tipi_sendmsg`, `tipi_recvmsg`, `tipi_log`.

## Type

`COND` / `AUTO` / `OBSERVE`. Skips when no TIPI is detected.

## Semantics exercised

- `tipi_on` detects and enables the TIPI and returns non-zero.
- `tipi_off` disables it; `tipi_on` enables it again.
- `tipi_sendmsg` / `tipi_recvmsg` perform a benign round trip using the log
  message (`0x25`); the contract returns exactly one byte, which is printed.
- `tipi_log` is the convenience wrapper; a line should appear in the TIPI log.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` on a TIPI system, or one
skip without a TIPI.

## Build and run

```
make -C example/gcc/tipitest
```

Copy `TIPITEST` to a ForceCommand-visible device and run `TIPITEST`.
