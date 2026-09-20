# EXITTEST - API test I12

Test for non-local executable exit.

Covers: `fc_exit` and the `exit(x)` macro alias.

## Type

`OBSERVE`. Because `fc_exit` does not return, there is no summary line.

## Steps

Run the program twice, from the ForceCommand prompt:

```
EXITTEST        ; status 0: clean return, no error line
EXITTEST 1      ; status 1: ForceCommand prints "error result: 1"
```

The program prints the status it will use and waits for a key before exiting,
so the expected result is on screen first.

- The status `0` path calls `exit(0)` (the macro alias).
- The non-zero path calls `fc_exit(status)` directly.

## Expected result

- `EXITTEST` returns to the prompt with no error message.
- `EXITTEST 1` returns with an `error result: 1` line.

## Build and run

```
make -C example/gcc/exittest
```

Copy `EXITTEST` to a ForceCommand-visible device and run it both ways.
