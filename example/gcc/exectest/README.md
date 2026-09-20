# EXECTEST - API test I6

Test for executing a ForceCommand command from inside an executable.

Covers: `exec_cmd`.

## Type

`AUTO` for return codes and the assignment side effect. `OBSERVE` for the echo
output and the unknown-command error.

## Semantics exercised

- `exec_cmd("echo ...")` runs the built-in and returns 0.
- `exec_cmd("")` returns 0 and does nothing.
- `exec_cmd("NAME=value")` performs a variable assignment; the value is then
  readable with `var_get`.
- `exec_cmd("unknown")` returns non-zero and prints an error.

## Notes

`exec_cmd` preprocesses and tokenizes the command in place, so all commands are
passed as mutable buffers. Only built-in commands and an assignment are used, so
no executable is loaded while `api_exec` is set.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`.

## Build and run

```
make -C example/gcc/exectest
```

Copy `EXECTEST` to a ForceCommand-visible device and run `EXECTEST`.
