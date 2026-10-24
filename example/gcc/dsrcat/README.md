# DSRCAT - API test I15

Test for DSR directory cataloging.

Covers: `dsr_catalog` with `vol_entry_cb` / `dir_entry_cb`, and an opt-in path
for `dsr_ea5_load`.

## Type

`AUTO`, requires a writable current device. If the device cannot create the
scratch file or does not support catalog the test skips.

## Semantics exercised

- A scratch file `ZZCATFIL` is created in the current directory.
- `dsr_catalog` lists the directory.
- The volume callback fires exactly once.
- The file callback fires at least once and sees `ZZCATFIL`.
- The first listed entry name is printed from inside the directory callback, and
  the entry count is printed afterwards.
- The scratch file is deleted afterwards.

## Opt-in: dsr_ea5_load

`dsr_ea5_load` transfers control to the loaded program, so it is never run by
the default test. To exercise it deliberately:

```
DSRCAT /ea5 <file>
```

## Callbacks and API calls

Executable callbacks are entered with bank 0 mapped (through the common bank
trampoline), so they can call ForceCommand API functions normally. Both
callbacks here call `hex_from_uint` and check the result, and `on_dir` prints
the first directory entry directly with `term_puts`/`term_putc` instead of
copying it into a global.

Callback signatures are still limited to seven arguments, the same as any
banked-callable function. Any pre-existing `ZZCATFIL` is deleted first, and the
file is deleted at the end. If the test is interrupted, the leftover file can be
deleted with `delete ZZCATFIL`.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` on a writable device
that supports catalog.

## Build and run

```
make -C example/gcc/dsrcat
```

Copy `DSRCAT` to a ForceCommand-visible device and run `DSRCAT`.
