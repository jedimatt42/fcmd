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
- The first listed entry name and the entry count are printed.
- The scratch file is deleted afterwards.

## Opt-in: dsr_ea5_load

`dsr_ea5_load` transfers control to the loaded program, so it is never run by
the default test. To exercise it deliberately:

```
DSRCAT /ea5 <file>
```

## Callback rule

Callbacks passed into ForceCommand APIs that run in a banked routine (such as
`dsr_catalog`, which runs in bank 9) **must not call any other ForceCommand API
function**. The external API trampoline always returns to cartridge bank 0, so
an API call from inside a callback leaves bank 0 mapped and the subsequent
return into the banked caller crashes. Keep such callbacks as leaf functions and
use plain C only.

The callbacks here do their own case-insensitive compare and copy for that
reason. Any pre-existing `ZZCATFIL` is deleted first, and the file is deleted at
the end. If the test is interrupted, the leftover file can be deleted with
`delete ZZCATFIL`.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` on a writable device
that supports catalog.

## Build and run

```
make -C example/gcc/dsrcat
```

Copy `DSRCAT` to a ForceCommand-visible device and run `DSRCAT`.
