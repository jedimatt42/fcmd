# MOUSETEST - API test I19

Test for the TIPI mouse.

Covers: `mouse_read`, `mouse_show`, `mouse_move`, `mouse_set_pointer`,
`mouse_hide`.

## Type

`COND` / `OBSERVE` / `AUTO`. The whole test skips unless the display is F18A in
80x30 mode.

## Semantics exercised

- `mouse_read` fills the first three bytes of `MouseData`; the button bits are
  within `MB_LEFT|MB_RIGHT|MB_MID`.
- `mouse_show` places the pointer at (128, 96) and enables the sprites.
- `mouse_move` follows the mouse and clamps `pointerx` to 8-247 and `pointery`
  to 0-238; the AUTO check verifies the clamp bounds while the pointer is
  observed following the mouse.
- Pressing a button prints an immediate `[OBSERVE] buttons = N LEFT/RIGHT/MID`
  line as the state changes, and a summary of which buttons were seen is printed
  when the movement phase ends. The public mask is AUTO-checked to contain only
  `MB_*` bits.
- `mouse_set_pointer` switches to the busy style and back.
- `mouse_hide` disables the pointer.

## Notes

`mouse_read` talks to the TIPI. The TIPI forwards the raw `/dev/input/mice` PS/2
packet, whose first byte carries the button bits plus an always-set bit and the
dx/dy sign/overflow bits; ForceCommand masks that byte to the `MB_*` bits before
returning it, so `buttons` only ever contains `MB_LEFT|MB_RIGHT|MB_MID`.
Without a TIPI or mouse the reads are no-ops and the pointer stays at center,
but the bound checks still hold. The program uses a non-screen-safe header so
ForceCommand restores the display and sprites on return.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`, or one skip when not in
F18A 80x30 mode.

## Build and run

```
make -C example/gcc/mousetest
```

Copy `MOUSETEST` to a ForceCommand-visible device and run `MOUSETEST`.
