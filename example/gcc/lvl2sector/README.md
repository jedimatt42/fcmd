# LVL2SECTOR - API test I18

Test for level-2 raw sector I/O.

Covers: `lvl2_sector_read`, `lvl2_sector_write`.

## Type

`AUTO`; `COND` for devices without level-2 sector I/O support.

## Semantics exercised

- `lvl2_sector_read` reads sector 0 into a 256-byte buffer.
- `lvl2_sector_write` writes the exact same bytes back, so disk content is
  unchanged.
- `lvl2_sector_read` reads it again and the two buffers are compared.

## Notes

`lvl2_format` is intentionally not tested: it requires a real floppy controller
and does not work under the available emulators. Sector write is done
read-modify-write with identical data to avoid changing the disk.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0` on a supporting device,
or one skip where sector I/O is unavailable.

## Build and run

```
make -C example/gcc/lvl2sector
```

Copy `LVL2SECTOR` to a ForceCommand-visible device and run `LVL2SECTOR`.
