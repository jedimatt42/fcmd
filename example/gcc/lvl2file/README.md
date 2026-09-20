# LVL2FILE - API test I16

Test for level-2 direct file I/O.

Covers: `path_to_iocode`, `lvl2_setdir`, `lvl2_input`, `lvl2_input_cpu`,
`lvl2_output`, `lvl2_output_cpu`, and the `AddInfo` fields.

## Type

`AUTO`; the direct I/O round-trip is `COND` and skips on devices without
level-2 direct input/output support (TIPI, SCSI, IDE, HFDC have it).

## Semantics exercised

- `path_to_iocode` maps device prefixes to iocodes: TIPI/DSK, DSK1/DSK2, RD,
  WDS, SCS, IDE, HDX.
- `lvl2_setdir` selects the current directory.
- A FIXED 128 source file with four records (`A`,`B`,`C`,`D`) is created with
  DSR.
- `lvl2_input` with blockcount 0 fills `AddInfo`; `lvl2_output` with blockcount
  0 creates a destination of the same shape.
- Sector 0 is copied with the VDP-buffer variants (`lvl2_input` /
  `lvl2_output`), sector 1 with the CPU-buffer variants (`lvl2_input_cpu` /
  `lvl2_output_cpu`).
- The destination is read back with DSR and compared record by record.
- Both scratch files are deleted.

## Notes

Direct I/O pads filenames in place to 10 characters, so filename buffers are at
least 11 bytes and mutable. `AddInfo` must be in scratchpad (`0x8320`). The VDP
variant uses `sys_info().vdp_io_buf`.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` on a supporting device.

## Build and run

```
make -C example/gcc/lvl2file
```

Copy `LVL2FILE` to a ForceCommand-visible device and run `LVL2FILE`.
