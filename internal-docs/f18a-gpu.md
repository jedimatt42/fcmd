# F18A GPU Preload Interface

The F18A firmware ships a small set of GPU routines in its private 2 KB GRAM
at `>4000`. The assembly source is `gpu_preload.asm` from `gpu_preload.zip`,
posted in the "F18A programming, info, and resources" thread on AtariAge. It
is present in F18A firmware v1.4 and later and is also implemented by the
PICO9918 F18A core.

The routines use a parameter block in normal VRAM and a vector table in the
GPU's GRAM:

| Address | Contents |
| --- | --- |
| `>3F00` | vector index (high byte) |
| `>3F01` | routine-specific flags |
| `>3F02-03` | source address |
| `>3F04-05` | destination address |
| `>3F06-07` | byte count |
| `>3F10` | scratch byte used by the ForceCommand block-fill helper |

The built-in vectors are:

| Index | Routine | Purpose |
| --- | --- | --- |
| 0 | `BLKCPY` | block copy or block fill |
| 1 | `FONTLD` | load a font from the F18A flash catalog |
| 2 | `GETINF` | return catalog version, free memory, and vector addresses |
| 3 | `GETIDX` | return a catalog index entry |
| 4 | `BLOBLD` | load a catalog data blob |

`BLKCPY` copies `>3F06-07` bytes from `>3F02-03` to `>3F04-05`. If `>3F01`
is zero, the source address increments and the data is copied. If `>3F01`
is nonzero, the source is read once and the same byte is written to every
destination byte (a block fill).

## Calling The Dispatcher

The dispatcher is at GPU address `>4026`. It reads the vector index from
`>3F00`, looks it up in the vector table at `>4006`, and branches to the
routine. When the routine returns, the dispatcher executes `IDLE`.

To call a routine:

1. Leave the F18A unlocked (the enhanced registers are required).
2. Write the parameter block at `>3F00`.
3. Point the GPU at the dispatcher by writing `>40` to VR54 and `>26` to
   VR55, or trigger the already-idle dispatcher by writing `>01` to VR56.
4. Wait for GPU idle by selecting status register 2 in VR15 and polling
   bit 7 of the status port.

ForceCommand wraps this in `src/f18a_gpu.c`. The vector index and
parameters are passed to `f18a_gpu_exec()`, `f18a_gpu_call()` runs a vector
whose parameter block is already staged, and `f18a_gpu_blkcpy()` /
`f18a_gpu_blkfill()` provide the two BLKCPY forms.

The 80x30 text scroll routine uses four BLKCPY calls: move the text rows,
clear the last text row, move the attribute rows, and clear the last attribute
row. This replaces the older custom GPU scroll program that was uploaded to
`>3F00` at runtime.
