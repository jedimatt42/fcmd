# LVL2DIR - API test I17

Test for level-2 directory operations.

Covers: `lvl2_mkdir`, `lvl2_rmdir`, `lvl2_rename`, `lvl2_rendir`,
`lvl2_protect`.

## Type

`AUTO`; `COND` for devices without level-2 directory support (TIPI, SCSI, IDE,
HFDC have it).

## Semantics exercised

- `lvl2_mkdir` creates `ZZLVL2A`.
- `lvl2_rendir` renames `ZZLVL2A` to `ZZLVL2B`; `ZZLVL2A` is then free to
  create again.
- `lvl2_rmdir` removes both directories.
- A DSR-created file `ZZLVL2F` is renamed to `ZZLVL2G` with `lvl2_rename`;
  `ZZLVL2G` opens and `ZZLVL2F` does not.
- `lvl2_protect` sets and clears protection on `ZZLVL2G`.

## Notes

The level-2 layer pads names in place to 10 characters, so every name buffer is
at least 11 bytes and mutable. `lvl2_setdir` selects the parent directory and
the operations take bare names. Scratch entries are cleaned up first and last.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` on a supporting device.

## Build and run

```
make -C example/gcc/lvl2dir
```

Copy `LVL2DIR` to a ForceCommand-visible device and run `LVL2DIR`.
