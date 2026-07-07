# TI-99/4A Sound List Player

Disassembly of the sound list player in the console ROM (`994a_rom/994a_rom_8k.bin`),
addresses `0x09E8` to `0x0A64`. This is a subset of the VDP interrupt service
routine (ISR) that starts at `0x0900`.

---

## Entry

Called from the ISR at `0x095A`:

```
0958  sla   r1, 1           ; shift status byte (test bit 14 of >83C2)
095a  joc   >09e8           ; if carry set → enter sound player
```

At entry:
- **R1** holds the status byte (from `>83C2`) in its MSB
- **R14** holds the sound configuration/timer step value
- **R13** holds a base address for sound chip writes

---

## Disassembly

```asm
; ---------------------------------------------------------------------------
; SOUND PLAYER – entered once per VDP interrupt when sound is enabled
; ---------------------------------------------------------------------------
;       Test sound disable flag (bit 13 of status at >83C2)
09e8  sla   r1, 1           ; shift left – carry = old bit 13
09ea  joc   >0a66           ; if bit 13 was 1 → skip sound player

;       Check sound timer
09ec  movb  @>83ce, r2      ; r2 (msb) = sound timer
09ee  83ce
09f0  jeq   >0a66           ; timer == 0 → nothing playing, exit

;       Decrement timer by R14
09f2  sb    r14, @>83ce     ; timer -= r14
09f4  83ce
09f6  jne   >0a66           ; timer still > 0 → wait, exit

;       Timer expired – load sound list pointer
09f8  mov   @>83cc, r3      ; r3 = sound list vdp address (16-bit)
09fa  83cc

;       Select data path based on R14 bit 0
09fc  mov   r14, r5         ; r5 = r14
09fe  srl   r5, 1           ; test r14 bit 0
0a00  joc   >0a16           ; if r14 lsb == 1 → path b (direct vdp)

; ---------------------------------------------------------------------------
; PATH A – r14 LSB = 0: scratchpad buffer path
; ---------------------------------------------------------------------------
0a02  bl    @>0864          ; read one byte from sound list → scratchpad buffer
0a04  0864
0a06  li    r5, >0402       ; r5 = sound chip base offset
0a08  0402
0a0a  a     r13, r5         ; r5 += r13  → address in sound chip range
0a0c  movb  r3, *r5         ; write r3 lsb to sound chip (register select)
0a0e  movb  @>83e7, *r5     ; write >83e7 byte to sound chip (data)
0a10  83e7
0a12  mov   r13, r6         ; r6 = r13
0a14  jmp   >0a24           ; join common processing loop

; ---------------------------------------------------------------------------
; PATH B – r14 LSB = 1: direct VDP read path
; ---------------------------------------------------------------------------
0a16  li    r5, >8c02       ; r5 = vdp write-address port
0a18  8c02
0a1a  movb  @>83e7, *r5     ; set vdp address high byte (with mode bits)
0a1c  83e7
0a1e  movb  r3, *r5         ; set vdp address low byte
0a20  li    r6, >8800       ; r6 = vdp read-data port
0a22  8800

; ---------------------------------------------------------------------------
; COMMAND DISPATCH – first byte of sound list entry
; ---------------------------------------------------------------------------
0a24  movb  *r6, r8         ; r8 (msb) = first byte
0a26  jeq   >0a46           ; byte == 0 → set-pointer command
0a28  cb    @>0a9c, r8      ; compare with toggle marker (>FF)
0a2a  0a9c
0a2c  jeq   >0a42           ; byte == >FF → toggle command

; ---------------------------------------------------------------------------
; DATA BLOCK – byte is count N (1..254)
; ---------------------------------------------------------------------------
0a2e  srl   r8, 8           ; move count to lsb
0a30  a     r8, r3          ; r3 += N (advance pointer past data)
0a32  movb  *r6, @>8400     ; write one byte to sound chip (>8400)
0a34  8400
0a36  dec   r8              ; N--
0a38  jne   >0a32           ; loop until all N bytes written
0a3a  inct  r3              ; r3 += 2 (skip count byte + delay byte)
0a3c  movb  *r6, r2         ; r2 (msb) = delay byte from vdp
0a3e  jeq   >0a52           ; delay == 0 → no timer (r2 = 0)
0a40  jmp   >0a54           ; else keep r2 (non-zero delay)

; ---------------------------------------------------------------------------
; TOGGLE COMMAND  (first byte == >FF)
; ---------------------------------------------------------------------------
0a42  xor   @>0378, r14     ; toggle r14 bit 0  (switch between path a/b)
0a44  0378
;       fall through to set-pointer handler — reads 2 address bytes from VDP
;       and sets timer to >0100 (1-frame delay for the next entry)

; ---------------------------------------------------------------------------
; SET-POINTER COMMAND  (first byte == 0)
; ---------------------------------------------------------------------------
0a46  movb  *r6, r3         ; r3 lsb = low byte of new vdp address
0a48  li    r2, >0100       ; r2 = >0100  → timer = 1 (next frame)
0a4a  0100
0a4c  movb  *r6, @>83e7     ; >83e7 = high byte of new vdp address
0a4e  83e7
0a50  jmp   >0a54

; ---------------------------------------------------------------------------
; TIMER SET TO ZERO  (no delay)
; ---------------------------------------------------------------------------
0a52  sb    r2, r2          ; r2 = 0  (no timer – advance next frame)

; ---------------------------------------------------------------------------
; SAVE STATE AND EXIT
; ---------------------------------------------------------------------------
0a54  mov   r3, @>83cc      ; update sound list pointer
0a56  83cc
0a58  movb  r2, @>83ce      ; set sound timer (r2 msb → >83ce)
0a5a  83ce
0a5c  ci    r5, >8c02       ; was this path b? (vdpwa = >8c02)
0a5e  8c02
0a60  jeq   >0a66           ; if path b → skip cleanup
0a62  bl    @>0842          ; path a cleanup: write buffered bytes to sound chip
0a64  0842
; >0a66: returns to ISR (continues with sla r1, 1)
```

---

## Sound List Format

Sound lists are stored in **VDP memory**. The pointer at `>83CC` (low 16 bits)
and `>83E7` (high byte) form the VDP address of the current entry.

Each entry begins with a one-byte command:

| Byte 0 | Command | Description |
|--------|---------|-------------|
| `>00`  | **Set pointer** | Read 2 more bytes as a new 16-bit VDP address; set timer to `>0100` (next frame). Chains or loops to another sound list. |
| `>FF`  | **Toggle** | XOR `R14` with `>0001` (from ROM at `>0378`), swapping between Path A and Path B. Falls through to set-pointer handler — reads 2 address bytes and sets timer to `>0100` (next frame). |
| `>01..>FE` | **Data block** | N data bytes follow. Write all N bytes to sound chip `>8400`, then read 1 delay byte → if non-zero, set timer to that value; if zero, timer = 0 (end of list — player exits). |

### Data Block Layout

```
+--------+--------+--------+--- ... ---+--------+
|  N     | byte1  | byte2  |   byteN   | delay  |
+--------+--------+--------+--- ... ---+--------+
  count     N bytes written to sound chip    timer
  (1-254)                                   (1-255 or 0)
```

- Total bytes consumed: `N + 2`
- The sound chip (SN76489/TMS9919) accepts 8-bit writes. Each data byte is
  written directly to the chip at `>8400`.
- The delay byte sets the sound timer `>83CE` (in units of VDP interrupts).
  A delay of 0 terminates playback — the sound player exits at the top
  of the next interrupt (timer check at `09F0`), processing no further
  entries.

### Set-Pointer Layout

```
+--------+--------+--------+
|  >00   | addr_L | addr_H |
+--------+--------+--------+
  command    new VDP address (16-bit)
```

- The new address is built from `addr_L` (low byte → `R3`/`>83CC`) and
  `addr_H` (high byte → `>83E7`).
- Timer is set to `>0100` (= 256 counts, but only the MSB `>01` is stored,
  so effectively a 1-frame delay).

### Toggle Layout

```
+--------+
|  >FF   |
+--------+
```

- Toggles `R14` bit 0, which switches between the two output paths.
  This allows interleaving two sound sources that may be addressed differently.
- Falls through into the set-pointer handler which reads 2 address bytes
  and sets timer to 1 (`>0100`). The command at the new address is processed
  on the next frame.

---

## Key Memory Locations

| Address | Size | Description |
|---------|------|-------------|
| `>83CC` | word | Sound list VDP address (low 16 bits of pointer) |
| `>83CE` | byte | Sound timer — counts down by `R14` each VDP interrupt; when it reaches 0, **no further entries are processed** (the player exits at `09F0`). A command is only dispatched when the timer *just became* 0 from a non-zero value (decremented from 1 at `09F2`). |
| `>83E7` | byte | VDP address high byte (address bits 13-8, combined with `>83CC`) |
| `>83C2` | byte | Status flags — **bit 14** enables the sound player; **bit 13** disables it |
| `>8400` | - | Sound chip write port (SN76489/TMS9919) |
| `>8800` | - | VDP read data port |
| `>8C02` | - | VDP write address/register port |
| `>0378` | word | XOR mask for toggling `R14` (value: `>0001` in console ROM; can be overridden by cartridge) |

## Register Usage

| Register | Role |
|----------|------|
| R1 | Status byte from `>83C2` (tested for enable/disable) |
| R2 | Sound timer value (MSB stored to `>83CE`) |
| R3 | Sound list pointer (tracked in software; stored at `>83CC`) |
| R5 | VDP address port / sound chip write address |
| R6 | VDP read data port / sound chip address |
| R8 | Current command byte / data byte counter |
| R13 | Base offset for sound chip write addressing |
| R14 | Timer step value; bit 0 selects data path |
| `>83E7` | VDP address high byte (used as temporary) |

## Data Paths

The sound player supports two data paths, selected by `R14` bit 0:

**Path A (bit 0 = 0):** Uses a scratchpad buffer at `>8300` (tracked by
counter at `>8373`). Bytes are read from the sound list one at a time via the
subroutine at `>0864`, buffered in scratchpad RAM, then flushed to the sound
chip by the subroutine at `>0842`.

**Path B (bit 0 = 1):** Reads directly from VDP memory. The VDP address is
set from `>83CC`/`>83E7`, and bytes are streamed directly from the VDP read
port to the sound chip at `>8400`.

The `>FF` (toggle) command flips bit 0 of `R14`, switching between the two
paths. This allows alternating between two different sound data sources.
