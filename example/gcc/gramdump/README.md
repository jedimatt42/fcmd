# GRAMDUMP

Inspect and validate ForceCommand's F18A GPU base image at GRAM `>4000`.

ForceCommand installs its own 60-byte base image instead of relying on the
F18A's resident preload (1.C does not provide one). This diagnostic answers:

1. Does the GPU execute code at all (`gpu_ran`)?
2. What is in `>4000` before `f18a_gpu_info()` runs?
3. Does `f18a_gpu_info()` install and verify the base, and what does it report?
4. Does the base BLKCPY handle the copy lengths used by the terminal and
   graphics code?

## Build

```
make -C example/gcc/gramdump
```

Copy `example/gcc/gramdump/GRAMDUMP` to a device visible to ForceCommand.

## Run

From ForceCommand, in the managed F18A 80x30 mode:

```
GRAMDUMP
```

The program deliberately keeps its report inside one 80x30 screen, so it never
scrolls the terminal and never invokes ForceCommand's GPU scroll path.

To prove that terminal output works without touching the GPU:

```
GRAMDUMP safe
```

To run the BLKCPY and fill probe:

```
GRAMDUMP blk
```

## What it does

1. Checks that the current VDP is an F18A and the mode is
   `GFX_MODE_F18A_TEXT80X30`.
2. Uploads a 20-byte GPU routine to VRAM `>3F20`:

   ```
   MOV  @>3F40,R0      ; source
   MOV  @>3F42,R1      ; destination
   MOV  @>3F44,R2      ; count
   MOVB *R0+,*R1+
   DEC  R2
   JNE  loop
   IDLE
   ```

   The host cannot read GRAM, but the GPU can. This copies `>4000-41FF` to
   VRAM `>3000`, where the CPU reads it.
3. Dumps the first 96 bytes of `>4000` and reports whether the base signature
   (`>0340`, `>FC01`) is present.
4. Calls `f18a_gpu_info()`, which installs the base if it is missing, and
   prints the result, version, free address, host BLKCPY entry and current app
   entry.
5. Reads `>4000` again and confirms the base is now present.
6. With `blk`, runs `f18a_gpu_copy()` at 1, 80, 320 and 1280 bytes and
   `f18a_gpu_fill()`.

## Expected report

On a working F18A after `f18a_gpu_info()`:

```
F18A GRAM >4000 / ForceCommand GPU base
reading GRAM after init ...
>4000: 03 40 FC 01 00 00 06 A0 40 18 02 01 FF 00 D8 01
>4010: 3F 0E 03 40 04 60 40 18 C0 20 3F 02 C0 60 3F 04
>4020: C0 A0 3F 06 D0 E0 3F 01 13 05 D0 10 DC 40 06 02
>4030: 16 FD 10 03 DC 70 06 02 16 FD 04 5B 00 00 00 00
base: IDLE + >FC01 at >4000
info: rc=+0
v=FC01 free=>403C vt=>4006 uv=>0000
blk: skipped (use 'blk' to probe)
```

The first dump is taken before `f18a_gpu_info()`. On a cold 1.C boot it
normally starts with `03 40` followed by zeros, because 1.C has only an
`IDLE` stub at `>4000` and no resident preload. The second dump must show the
ForceCommand base and `>FC01`.

`free=>403C` is the first unused GPU GRAM address. `uv=>0000` means no
executable GPU blob is currently loaded; launch a program with a GPU blob to
see its entry there.

## Reading a failure

- `GPU code upload failed - VRAM port problem` means the bytes read back from
  `>3F20` did not match. The data port is not writing VRAM normally.
- `GPU did not run - dump is stale VRAM` means the marker byte in `>3000`
  never changed, even after the tool retried with a fresh F18A unlock. The GPU
  is not executing code.
- `info: rc=-2` means the display is not an F18A.
- `info: rc=-3` means the public GPU API does not consider the current mode a
  managed F18A 80x30 mode.
- `info: rc=-5` means the base upload or verification timed out.
- `base: IDLE stub, no ForceCommand >FC01` after `f18a_gpu_info()` succeeded
  is a contradiction: the API verified the base, so re-run the tool and check
  that the display did not reset the F18A between dumps.
- `base: absent (all zero)` before init is normal on 1.C. After init it means
  the upload did not reach GRAM.

## VRAM used

The test uses `>3000-31FF` as the transfer buffer, `>3F20-3F45` for the
uploaded routine and its parameters, and the normal GPU mailbox at `>3F00`.
Those are scratch in ForceCommand's 80x30 mode. It does not modify the screen
tables or the ForceCommand GPU base.
