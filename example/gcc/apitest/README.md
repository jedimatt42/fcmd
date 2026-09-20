# APITEST - API test harness

`fctest.h` is the shared, header-only self-check harness used by every API test
demo under `example/gcc/`. It is not an API test itself; `selftest.c` proves the
harness works so the other tests can trust it.

## Result protocol

Each assertion prints one line:

```
[PASS] <description>
[FAIL] <description> got=<n> want=<n>
[SKIP] <reason>
[OBSERVE] <what the human should look for>
[WAIT] <prompt>
pass=<n> fail=<n> skip=<n>
```

`main()` returns `fc_summary()`, so a test with any failure returns a non-zero
status and ForceCommand reports it as an error.

## Macros

| Macro | Use |
| --- | --- |
| `FC_CHECK(cond)` | Pass/fail for a boolean expression |
| `FC_CHECK_EQ(got, want)` | Pass/fail, printing got/want on failure |
| `FC_SKIP(reason)` | Capability is not available on this machine |
| `FC_OBSERVE(what)` | A human must confirm the visual/audio result |
| `FC_WAIT(prompt)` | Block until a fresh key press, then continue |

## Using the harness in a new test

1. Copy this directory's `_header.asm`, `linkfile`, and `Makefile` into the new
   test directory.
2. Add `-I$(abspath ../apitest)` to `CFLAGS`.
3. `#include "fctest.h"` and build assertions in `main`.
4. Finish with `return fc_summary();`.

`fctest.h` counts passes, failures, and skips in `static` storage. Because each
test is a single translation unit, the counters are private to that program.

## Notes

- `str_from_uint` returns a shared static buffer, so the harness formats and
  prints one integer at a time.
- `FC_WAIT` polls `term_kscan(5)` and checks `KSCAN_STATUS` for a new key press,
  so a held key does not immediately satisfy the wait.
- Tests that change VDP modes must use a non-screen-safe header (the third
  header word must not be `0xFCFC`). See `../graphicsdemo/_header.asm`.

## Build

```
make -C example/gcc/apitest
```

This program does not change the display, so it declares that it leaves the
screen safe for ForceCommand on return.
