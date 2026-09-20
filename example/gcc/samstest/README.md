# SAMSTEST - API test I13

Test for SAMS page allocation and mapping.

Covers: `sys_sams_info`, `sams_alloc_pages`, `sams_free_pages`,
`sams_map_page`, `sams_read_page`.

## Type

`COND` / `AUTO`. On a machine without SAMS the whole test is skipped and the
summary reports one skip.

## Semantics exercised

- `sys_sams_info` reports non-negative page counts.
- `sams_alloc_pages(1)` returns the previous `next_page` and advances it.
- `sams_free_pages(1)` returns the new top and moves `next_page` back.
- `sams_read_page` returns `-1` for addresses outside `0xA000`-`0xF000`.
- `sams_map_page` maps distinct physical pages into the `0xF000` window; each
  page keeps its own data after remapping.
- The original page mapped at `0xF000` is read first and restored.

## Safety

The test window is the top 4K (`0xF000`). A small program does not reach that
far, so it is safe scratch space. ForceCommand also restores its own SAMS
snapshot after the executable returns.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`, or `skip=1` on a
machine without SAMS.

## Build and run

```
make -C example/gcc/samstest
```

Copy `SAMSTEST` to a ForceCommand-visible device and run `SAMSTEST`.
