# LISTTEST - API test I4

Test for the list stack.

Covers: `list_init`, `list_push`, `list_pop`, `list_get`, including the ceiling
and overflow path.

## Type

`AUTO` for ordering, truncation, and bounds. `OBSERVE` for how many entries
survive an overflow.

## Semantics exercised

- `list_init` sets an empty list between `addr` and `ceiling`.
- `list_push` inserts at the head, so `list_get(list, 0)` is the newest entry
  and `list_pop` removes newest first.
- `list_pop` copies `min(entry.length, limit)` bytes and does not null terminate
  when truncated, but still removes the whole entry.
- `list_pop` on an empty list is a no-op.
- `list_get` returns 0 past the end.

## Overflow note

The eviction walk in `src/list.c` compares an entry length against the list end
address cast to `int`. Because the test buffer is in upper expansion memory,
that value is negative and eviction discards all older entries instead of only
the oldest. The test asserts the invariants that must hold either way (newest
entry survives, `end` stays within `[addr, ceiling]`) and prints the surviving
count. A future implementation that evicts one entry at a time would still pass.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`.

## Build and run

```
make -C example/gcc/listtest
```

Copy `LISTTEST` to a ForceCommand-visible device and run `LISTTEST`.
