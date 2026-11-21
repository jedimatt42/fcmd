# F18A GPU Support

ForceCommand owns the F18A GPU GRAM. It does not depend on the F18A's
resident preload: 1.C does not provide one, and 1.9's catalog routines are not
used.

The base image is `src/gpu_base.asm`, a 60-byte data blob copied into GRAM
after F18A detection and 80x30 mode setup. A 32-byte bootstrap copies the
image and initializes the GPU stack pointer.

## GRAM layout

| Address | Use |
| --- | --- |
| `>4000` | `IDLE` |
| `>4002` | magic `>FC01` |
| `>4004` | reserved |
| `>4006` | host BLKCPY entry: call core, set done, `IDLE` |
| `>4014` | GPU-callable BLKCPY entry: `B @>4018` |
| `>4018` | BLKCPY core, returns via `B *R11` |
| `>403C` | first free address (`GPU_FREE_INIT`) |
| `>4700-47FF` | stack reserve, `R15 = >47FE` |

`>4000` is a safe `IDLE` for a stray power-on trigger. Host code always
triggers a specific entry; nothing relies on the default PC.

The base image is uploaded with a VRAM bootstrap at `>3E00`:

1. Unlock the F18A: clear VR57, then write `>1C` twice. Clearing first makes
   the sequence work whether the F18A was locked or already unlocked.
2. Copy the base image and bootstrap from FCMD ROM to VRAM.
3. Trigger the bootstrap at `>3E00`; it copies the base to GRAM `>4000`, sets
   `R15 = >47FE`, writes `>FF` to `>3F0E`, and idles.
4. Verify by calling the host BLKCPY entry to copy `>4000-403B` back to VRAM
   and checking `IDLE` and the `>FC01` magic.

`f18a_gpu_init()` is idempotent. If the base magic is still present, it keeps
the current `gpu_free` and `gpu_entry`; otherwise it uploads again and resets
the free pointer. This matters when `setupScreen` resets the F18A but the GRAM
contents survive.

## Mailbox

The CPU-accessible mailbox is in VRAM:

| Address | Use |
| --- | --- |
| `>3F00` | unused |
| `>3F01` | BLKCPY flags / app function ID |
| `>3F02-03` | arg0 / BLKCPY source |
| `>3F04-05` | arg1 / BLKCPY destination |
| `>3F06-07` | arg2 / BLKCPY length |
| `>3F08-0D` | arg3-arg5 |
| `>3F0E` | done: `0` busy, `>FF` done |
| `>3F0F` | status: `0` OK |
| `>3F10` | BLKCPY fill byte / scratch |
| `>3F11-1D` | app scratch |
| `>3F1E-1F` | app blob base (R14) |
| `>3F20-2F` | return block |

The done/status handshake is in VRAM. ForceCommand does not use S2/VR15 to
wait for the GPU; that path is unreliable on 1.C.

## BLKCPY

The base BLKCPY uses the 1.9 parameter semantics:

- `>3F01 = 0`: copy, source increments.
- `>3F01 != 0`: fill, source is read once and repeated.
- `>3F02-03`: source.
- `>3F04-05`: destination.
- `>3F06-07`: byte count.

The host calls the entry at `>4006`; it sets `>3F0E` and idles. GPU code calls
`>4014` with `BL`, which returns through R11. The routine clobbers R0-R3, so
save any live registers around the call.

## Executable GPU blobs

GPU routines are position-independent. The compiler links a blob relative to
0; the loader copies it into GRAM at the current free address without
patching.

The app entry is at offset 0:

```
        LI   R15,>47FE
        MOV  @>3F1E,R14      ; R14 = blob base
        ; dispatch on >3F01
        ; ...
        CLR  @>3F0F          ; status first...
        LI   R1,>FF00
        MOVB R1,@>3F0E       ; ...then done
        IDLE
```

- Data references are `@offset(R14)`.
- Control flow is relative.
- Fixed ABI addresses are allowed: the mailbox, VDP registers, and
  `BL @>4014` for BLKCPY.
- Internal routines use `BL` / `B *R11`, with R15 as the stack and R14
  preserved.
- Write the status byte before the done byte. The host polls done and then
  reads status, so the reverse order is a race.

The loader keeps a bump allocator:

```
gpu_free    // first unused GRAM byte, starts at >403C
gpu_entry   // current executable's blob base
```

On executable start it saves both values, copies the blob from the filesystem
file buffer to GRAM at `gpu_free`, sets `gpu_entry`, and advances `gpu_free`.
On return it restores both. The child's code remains above the parent's code
but is considered free; the next push overwrites it. There is no code
save/restore and no vector-table update.

If the blob does not fit (`gpu_free + len > >4700`), the push fails, the
loader sets `gpu_entry = 0`, and the executable runs without GPU support.

## FC executable header

The original header is four words:

```
>A000  >FCFC
>A002  sams page count (0 = flat)
>A004  safe
>A006  main
```

The extended header sets bit 15 of the `sams` word and adds two words:

```
>A000  >FCFC
>A002  >8000 | sams page count     ; FCH_EXT
>A004  safe
>A006  main
>A008  gpu_blob_block
>A00A  gpu_blob_length
>A00C  code...
```

Old executables leave bit 15 clear; their code starts at `>A008` and no GPU
fields are read. New executables set `FCH_EXT`; code starts at `>A00C`. If
`gpu_blob_block` or `gpu_blob_length` is zero, there is no GPU blob.

The GPU blob is appended to the executable file on a 256-byte block boundary:

```
CPU image
padding to 256 bytes
GPU blob (block gpu_blob_block, length gpu_blob_length)
```

The loader reads `ceil(length/256)` blocks from that block into the filesystem
file buffer, remembers that address, then pushes it into GRAM after the
executable's screen setup is complete. Remembering the address matters because
`setupScreen` can relocate the filesystem buffers. The CPU image load stops
before the blob block, so the blob is never loaded as CPU code.

## Public API

`f18a_gpu_info`, `f18a_gpu_copy`, `f18a_gpu_fill` and `f18a_gpu_exec` are
retained. `f18a_gpu_exec` only supports vector 0 (BLKCPY). `f18a_gpu_info`
returns the base magic, free address, host BLKCPY entry and current app entry.

`f18a_gpu_call` runs a compiled GPU routine:

```c
struct GpuCall {
    unsigned char func;
    unsigned char status;
    unsigned int arg[6];
    unsigned int result[6];
};

int f18a_gpu_call(struct GpuCall* call);
```

It writes the function ID to `>3F01`, args to `>3F02-0D`, the blob base to
`>3F1E`, clears done/status, unlocks the F18A, triggers the current app entry,
polls `>3F0E`, and reads `>3F0F` plus the return block.

## 1.C and 1.9 differences

- 1.9 has the resident preload at `>4000`; 1.C leaves only an `IDLE` stub and
  no preload. ForceCommand overwrites `>4000` with its own base either way.
- The F18A can be locked while the 80x30 ECM display is still active. Every GPU
  trigger clears VR57 and then writes `>1C` twice. The clear is required:
  writing `>1C` twice when the F18A is already unlocked re-locks it.
- GPU completion is detected through the VRAM done byte, not S2.
- If the base initialization fails, ForceCommand installs the CPU scroll
  routine (`scrn_scroll_default`) instead of the GPU scroll.
