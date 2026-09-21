# SPEECHTEST - API test I23

Test for the speech synthesizer.

Covers: `speech_detect`, `speech_reset`, `speech_say_vocab`, `speech_start`,
`speech_continue`, `speech_wait`.

## Type

`COND` / `OBSERVE` / `AUTO`. Skips when no synthesizer is detected.

## Semantics exercised

- `speech_detect` returns non-zero and installs the safe-read routine.
- `speech_reset` resets the synthesizer.
- `speech_say_vocab(VOCAB_HELLO)` speaks the built-in phrase; `speech_wait`
  returns when it finishes.
- `speech_start` sends up to the first 16 LPC bytes and advances
  `ctx.addr`/`ctx.remaining`; `speech_continue` feeds the rest as the FIFO
  drains. The accounting is AUTO-checked with a short dummy buffer.
- `speech_reset` silences the synthesizer afterwards.

## Notes

The dummy LPC buffer is all zeros, so the start/continue phase may make a brief
noise; it is reset immediately after. `speech_say_data` is covered by the `say`
example, which uses real LPC data.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`, or one skip without a
synthesizer. The word "hello" should be audible.

## Build and run

```
make -C example/gcc/speechtest
```

Copy `SPEECHTEST` to a ForceCommand-visible device and run `SPEECHTEST`.
