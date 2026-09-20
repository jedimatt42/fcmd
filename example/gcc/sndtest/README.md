# SNDTEST - API test I11

Test for the ForceCommand sound list player.

Covers: `snd_start`, `snd_tick`, `snd_playing`, `snd_stop`, `snd_play`.

## Type

`AUTO` for the player state machine; `OBSERVE` for the audible beep.

## Sound list format

- Bytes `1..254`: data block. That many bytes are written to the sound chip,
  followed by one delay byte. A delay of `0` ends playback.
- Byte `0x00` or `0xFF`: set pointer; two address bytes follow.

## Semantics exercised

- `snd_start` activates the player; `snd_playing` reports it; `snd_stop`
  deactivates it.
- `snd_tick` consumes a data block, writes its bytes, and applies the delay.
- A delay of `0` ends playback; ticking while inactive is a no-op.
- Delay bytes are counted down one per tick.
- `snd_play` drives the player to completion and leaves it stopped.

The AUTO lists contain only "volume off" register writes, so they are silent.
The final `probe_tone` is audible.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`, and one short beep is
heard after the observe prompt.

## Build and run

```
make -C example/gcc/sndtest
```

Copy `SNDTEST` to a ForceCommand-visible device and run `SNDTEST`.
