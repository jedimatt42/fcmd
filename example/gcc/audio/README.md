# AUDIO - API test I10

Test for the built-in terminal sounds.

Covers: `audio_beep`, `audio_honk`.

## Type

`OBSERVE`. Both functions are void and block until the tone finishes, so
correctness is judged by listening.

## Expected result

- One short high-pitched beep after the first prompt.
- One lower honk after the second prompt.
- The summary line is `pass=0 fail=0 skip=0`; there are no machine-checkable
  results in this increment.

## Build and run

```
make -C example/gcc/audio
```

Copy `AUDIO` to a ForceCommand-visible device and run `AUDIO`.
