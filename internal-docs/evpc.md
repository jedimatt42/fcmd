# EVPC DSR ROM and VDP Initialization

This document disassembles the DSR ROM from `ti99_evpc.zip` and documents the
power-on VDP initialization used by the SNUG Enhanced Video Processor Card
(EVPC). The short version: the EVPC power-up routine deliberately puts the
V9938 back into the TMS9918A memory layout, with all 9918 tables in the first
16 KB of VRAM, so unmodified TI-99/4A software sees the VDP it expects.

## Source

| Item | Value |
| --- | --- |
| Archive | `~/mame/roms/ti99_evpc.zip` |
| File | `evpc_dsr.u21` |
| Size | 65,536 bytes (64 KB) |
| MD5 | `602429bd37f42a9eaf45a435c0b25f22` |
| SHA-1 | `6e8060f86e3bb9c36f244d88825e3fe237bfe9a9` |
| MAME CRC | `a062b75d` |
| Banner | ` EVPC-DSR v4.16 H.Glaab 30.04.00` at CPU `>4020` |

The image matches MAME's `ti99_evpc` ROM definition. MAME models the card as
eight 8 KB DSR pages selected through the card's CRU bits at base `>1400`:

| CRU bit | Function |
| --- | --- |
| 0 | map the DSR ROM into `>4000->5FFF` |
| 1 | DSR page select, bit 0 |
| 3 | enable the 256-byte NOVRAM at `>5F00->5FEF` |
| 4 | DSR page select, bit 2 |
| 5 | DSR page select, bit 1 |

The page number is `CRU bit 1 | (CRU bit 5 << 1) | (CRU bit 4 << 2)`, giving
pages 0-7. Each page is 8 KB and maps over `>4000->5FFF`. The palette registers
are exposed at `>5FF0->5FFF`; the V9938 itself is at the normal console VDP
ports.

## ROM map

| Page | File range | Contents |
| --- | --- | --- |
| 0 | `0000-1FFF` | DSR header, lists, power-up routine, VDP helpers, DSR entries |
| 1 | `2000-3FFF` | empty |
| 2 | `4000-5FFF` | TEXT80 / 80-column code; linked for RAM `>A000` |
| 3 | `6000-7FFF` | empty |
| 4 | `8000-9FFF` | empty |
| 5 | `A000-BFFF` | GRAPH1/GRAPH2/GIF99 graphics code; linked for RAM `>A000` |
| 6 | `C000-DFFF` | 8x8 character generator data |
| 7 | `E000-FFFF` | empty |

Banks 2 and 5 are not executed in place. Their first instruction is a stub:

```
bank 2: SBZ 0 / CLR R12 / B @>BCE2
bank 5: SBZ 0 / CLR R12 / B @>A486
```

The DSR entry routines copy bank images into RAM at `>A000` and jump there.
This is why branches inside banks 2 and 5 use addresses such as `>BCE2` and
`>A486`: the bank is linked for a RAM load address of `>A000`, not for the
DSR window.

## DSR header and lists

The standard DSR header is at `>4000`:

```
>4000  AA 40      identifier, version/header byte >40
>4002  0000       reserved (zero)
>4004  4084       power-up list
>4006  0000       program list: none
>4008  4040       subprogram list
>400A  4070       DSR list
>400C  0000       ISR list: none
>400E  0000
>4010  994A 0416 0460 44F6 0460 0000 0460 0000
>4020  " EVPC-DSR v4.16 H.Glaab 30.04.00"
```

The `>4010` words are a secondary ID/vector block (`B @>44F6`, `B @>0000`,
`B @>0000`); they are not used by the normal DSR scan. The banner text occupies
the rest of the 64-byte header, so the lists begin at `>4040`.

Power-up list at `>4084`:

```
>4084  0000       link: end
>4086  40FC       power-up routine
```

Subprogram list at `>4040`:

| Address | Link | Routine | Name |
| --- | --- | --- | --- |
| `>4040` | `>404C` | `>45C4` | `TEXT80` |
| `>404C` | `>4058` | `>4D3A` | `DEFCOL` |
| `>4058` | `>4064` | `>4FD0` | `GRAPH1` |
| `>4064` | `0000` | `>4FD6` | `GRAPH2` |

DSR list at `>4070`:

| Address | Link | Routine | Name |
| --- | --- | --- | --- |
| `>4070` | `>407A` | `>4356` | `EVPC` |
| `>407A` | `>4088` | `>438E` | `GIF99` |
| `>4088` | `0000` | `>5702` | `BASIC` |

The `EVPC` and `GIF99` entries are bank loaders. They copy DSR bank images
into RAM with a small trampoline placed at `>3000`, then jump to the copied
code at `>A000`. The trampoline is copied from `>43E2` by `>4406`. It uses the
`X` (execute) instruction to run `SBO`/`SBZ` page-select instructions supplied
as inline data, then copies from `>4000` to RAM. The loader sequence for
`EVPC` is page 5 -> `>A000`, page 6 -> `>BFF0`, page 7 -> `>DFE0`; `GIF99`
uses pages 2, 3, and 5. The last entry, `BASIC`, is a normal bank-0 routine at
`>5702`.

## Disassembly conventions

The ROM is a TMS9900 image. Most of the code of interest is in page 0, where
the file offset is simply the CPU address minus `>4000`:

```
unzip -o ~/mame/roms/ti99_evpc.zip -d /tmp/evpc
unidasm /tmp/evpc/evpc_dsr.u21 -arch tms9900 \
        -basepc 0x40fc -skip 0x00fc -count 0x64
```

For the bank images, use the bank's file offset and the RAM load address:

```
# bank 2, linked for RAM >A000
unidasm /tmp/evpc/evpc_dsr.u21 -arch tms9900 \
        -basepc 0xA000 -skip 0x4000 -count 0x100

# bank 5, linked for RAM >A000
unidasm /tmp/evpc/evpc_dsr.u21 -arch tms9900 \
        -basepc 0xA000 -skip 0xA000 -count 0x100
```

Two TMS9900 details matter for reading the VDP helpers:

* `MOVB` to or from a register uses the **most significant byte** of the
  register, not the least significant byte.
* The V9938 control port uses the MSX/V9938 byte order: **data first**, then
  the `>80|register` byte. Address writes are low byte first, then high byte
  with `>40` set for a write. The helpers below are written to produce exactly
  these sequences.

The VDP port addresses used by the EVPC are the normal V9938 set:

| Address | Use |
| --- | --- |
| `>8800` | VDP read data |
| `>8802` | VDP status |
| `>8C00` | VDP write data (VRAM) |
| `>8C02` | VDP control: address setup and register writes |
| `>8C04` | VDP palette data |
| `>8C06` | VDP indirect register write |

The GPL workspace's `R15` is `>8C02`. Thus `*R15` is the control port,
`>FFFE(R15)` is `>8C00` (write data), and `>FBFE(R15)` is `>8800` (read data).
Palette writes go directly to `>8C04`.

## VDP helper layer

| Routine | Function |
| --- | --- |
| `>44AA` | Write one VDP register. `R0` = `(register << 8)` OR `value`. Emits value, then `>80` OR `register` to `>8C02`. |
| `>44B0` | Set a VDP write address. `R0` = 14-bit address. Emits low byte, then `high` OR `>40` to `>8C02`. |
| `>44B4` | Internal address-write engine used by `>44B0`, `>44BE`, `>44DA`, `>44EA`. |
| `>44BE` | Set write address `R0`, then write `R2` bytes from `(R1)` to `>8C00`. |
| `>44CE` | Set write address `R0`, then write one byte `R1` to `>8C00`. |
| `>44DA` | Set read address `R0`, then read `R2` bytes from `>8800` into `(R1)`. |
| `>44EA` | Set read address `R0`, then read one byte from `>8800` into `R1`. |
| `>448A` | Read the inline word after the `BL` and pass it to `>44AA` (register write). |
| `>448E` | `R14 = 0` (VRAM page 0). |
| `>4494` | `R14 = 1` (VRAM page 1). |
| `>449A` | `R14 = 4` (VRAM page 4). |
| `>44A0` | `R45 = >00` (VRAM bank, not expansion RAM). |
| `>44A6` | `R45 = >40` (expansion RAM bank). |

`>44AA` in detail, with the high-byte `MOVB` behavior:

```
44AA  ORI  R0,>8000     ; R0 = >80|reg in high byte, value in low byte
44AE  JMP  >44B4
44B4  SWPB R0           ; now value is in the high byte
44B6  MOVB R0,*R15      ; emit value
44B8  SWPB R0
44BA  MOVB R0,*R15      ; emit >80|register
44BC  B    *R11
```

`>44B4` likewise emits the address low byte first, then the high byte with
`>40` set, which is the V9938 write-address protocol.

## Power-up routine

The real power-up routine is at `>40FC`, reached through the list at `>4084`.
It is not at `>4084` itself. It is called by the console's power-up scan in the
GPL workspace, so `R15` is the VDP control port (`>8C02`). Annotated
disassembly:

```
40FC  C20B        MOV  R11,R8         save return address
40FE  04C0        CLR  R0
4100  06A0 44EA   BL   @>44EA         read VDP[0] into R1
4104  C081        MOV  R1,R2          save first byte
4106  06A0 448A   BL   @>448A         write register 14 = 2
410A  0E02        DATA >0E02          (inline data for >448A)
410C  04C0        CLR  R0
410E  06A0 44EA   BL   @>44EA         read VDP[0] again
4112  9081        CB   R1,R2
4114  160B        JNE  >412C          different -> V9938 detected
; ---- TMS9918A fallback path ----
4116  06A0 448A   BL   @>448A         write register 8 = 8
411A  0808        DATA >0808
411C  06A0 4454   BL   @>4454         write 128 bytes from >8300
4120  06A0 441A   BL   @>441A         16 KB VRAM read/write test
4124  0800        DATA >0800          (R8 = 0 for the test loop)
4126  0808        DATA >0808          (R8 = 8 for the test loop)
4128  06A0 4468   BL   @>4468         read back the expansion-RAM block
; ---- common 9918/9938 register initialization ----
412C  0201 4092   LI   R1,>4092        register/value table
4130  04C0        CLR  R0              write register 0 = 0
4132  06A0 44AA   BL   @>44AA
4136  C031        MOV  *R1+,R0         next register/value pair
4138  16FC        JNE  >4132           until >0000 terminator
413A  06A0 4282   BL   @>4282          read/validate NOVRAM or DIP config -> R9
413E  06A0 4320   BL   @>4320          adjust R9 (PAL/NTSC, 212 lines, interlace)
4142  06A0 4308   BL   @>4308          adjust R8 (black/white mode)
4146  06A0 422E   BL   @>422E          reserve VDP memory / update >8370
414A  06A0 4200   BL   @>4200          show "EVPC NOVRAM"/"EVPC DIP-SW"
414E  06A0 4160   BL   @>4160          load palette
4152  06A0 41CA   BL   @>41CA          reserved / no-op
4156  06A0 41E6   BL   @>41E6          set R18 (display adjust) from config
415A  06A0 448E   BL   @>448E          R14 = 0 (VRAM page 0)
415E  0458        B    *R8             return
```

### 9938 detection

The detection is the classic VRAM-page trick:

1. Read a byte from VDP address 0.
2. Write VDP register 14 (VRAM access base) = 2.
3. Read VDP address 0 again.

On a V9938, register 14 selects the 16 KB VRAM page, so the second read comes
from VRAM offset `>08000`. On a TMS9918A there is no R14, so the same byte is
read both times. If the bytes differ, the common V9938-compatible initialization
runs. If they match, the 9918 fallback path runs first and then the common
table is still applied. The detection is a heuristic; an accidental match only
adds the fallback VRAM test and does not prevent the common register setup.

The 9918 fallback path (`>4116`) writes R8, copies 128 bytes from scratchpad
`>8300` to the VDP/expansion-RAM window (`R45 = >40`, `R14 = 1`), runs a
read/write test over the 16 KB range using 128-byte blocks while toggling R8
between 0 and 8, and reads the expansion-RAM block back. This is
diagnostic/presence testing, not the compatibility setup itself.

## VDP register table at >4092

The table is a sequence of `(register, value)` byte pairs, terminated by
`>0000`. `R0` is written separately as zero before the loop. The table sits
immediately after the `BASIC` DSR name at `>4088`, so its first word is
`>0120` (R1 = `>20`).

| Register | Value | Meaning |
| --- | --- | --- |
| R0 | `>00` | Mode 0: Graphics 1, no external video, no interrupts. Written separately before the loop. |
| R1 | `>20` | IE0 set, display blank, 8x8 sprites, M1/M2 = 0. Matches the console GROM's default R1 table value. |
| R2 | `>00` | Name table base `>0000`. |
| R3 | `>0E` | Color table base `>0380` (`>0E << 6`). |
| R4 | `>01` | Pattern generator base `>0800` (`>01 << 11`). |
| R5 | `>06` | Sprite attribute base `>0300` (`>06 << 7`). |
| R6 | `>00` | Sprite pattern base `>0000`. |
| R7 | `>F7` | Text color 15 (white), backdrop 7. |
| R8 | `>08` | V9938 mode register 2: VRAM type bit set (128 KB VRAM). |
| R9 | `>02` | V9938 mode register 3: PAL timing (`*NT` set). Changed later by DIP/NOVRAM. |
| R10 | `>00` | Color table high bits. |
| R11 | `>00` | Sprite attribute high bits. |
| R12 | `>F7` | Blink text/background colors. |
| R13 | `>00` | Blink period. |
| R14 | `>00` | VRAM access base (page 0). |
| R15 | `>00` | Status register pointer. |
| R16 | `>00` | Palette address. |
| R17 | `>00` | Indirect register pointer. |
| R18 | `>00` | Display adjust (later replaced from configuration). |
| R19 | `>00` | Interrupt line. |
| R23 | `>00` | Display offset / vertical scroll. |

Registers 20-22 (color burst) are not written; they keep their V9938 power-on
values (`>00`, `>3F`, `>05`). Registers 24-31 do not exist. Registers 32-46
(command unit) are not touched by the power-up routine.

The important compatibility effect is in R2-R6: every 9918 table is placed in
the first 16 KB of VRAM. The console GROM's default table contains `1` bits in
the high, 9918-unused bits of these registers (`R2 =>F0`, `R4 =>F9`,
`R5 =>86`, `R6 =>F8`). On a V9938 those bits select high VRAM pages, which is
exactly what breaks 9918 software. The EVPC DSR writes the masked values
instead:

```
console GROM default     EVPC DSR
R2  F0  -> masked 00     R2  00
R4  F9  -> masked 01     R4  01
R5  86  -> masked 06     R5  06
R6  F8  -> masked 00     R6  00
```

This is the core of the "9938 in 9918 mode" initialization.

## Configuration, palette, and display adjust

After the register table the power-up routine applies the card configuration.

### NOVRAM / DIP switch configuration (`>4282`)

`>4282` enables the NOVRAM (CRU bit 3), reads the 228-word configuration area
at `>5F00`, validates its checksum, and produces a flag word in `R9`:

* `R9 = 0` means DIP-switch mode or invalid NOVRAM; use defaults.
* `R9` with bit 15 set (`>8000`) means a valid NOVRAM configuration.
* Other bits in `R9` select video standard, 212-line mode, interlace, palette
  source, and black/white mode.

In DIP mode the card's switches are read directly through CRU bits. MAME's
`ti99_evpc` input definitions map switch 1 to PAL/NTSC, switch 3 to the
character set, switch 4 to the VDP RAM shift, and switch 8 to DIP vs NOVRAM.

### R9 and R8 adjustments (`>4320`, `>4308`)

`>4320` starts from `>0902` (R9 = 2) and applies:

| Source | Mask | Effect |
| --- | --- | --- |
| `>43DA` | `>0002` | clear R9 bit 1 (select NTSC) |
| `>4784` | `>0080` | set R9 bit 7 (212-line mode) |
| `>5776` | `>0008` | set R9 bit 3 (interlace) |

`>4308` starts from `>0808` (R8 = 8) and, when the configuration requests it,
ORs `>0001` into R8 (black-and-white mode).

The resulting R9 is also saved at `>8502` for later DSR use.

### VDP memory reservation (`>422E`)

`>422E` reads the console's "highest free VDP address" from scratchpad `>8370`,
moves a 6-byte block at the top of VDP memory down by one, writes a `>AA`
marker at VDP `>3FFF`, and updates `>8370`. This reserves a small area at the
top of the 16 KB VDP memory map for the card. It is not part of the mode setup,
but it is part of the power-up sequence and can matter to software that relies
on the highest-free-address pointer.

### Status message (`>4200`)

`>4200` writes an 11-character message to VDP `>00C2`:

* `EVPC NOVRAM` when a valid NOVRAM configuration was found.
* `EVPC DIP-SW` otherwise.

The message is written directly into the name table area, so it appears on the
title screen for a moment during power-up.

### Palette load (`>4160`)

`>4160` loads a 16-entry palette through the V9938 palette port:

* It writes `R16` (palette index) with `>44AA`.
* It writes two bytes per entry directly to `>8C04`.
* It repeats 16 times, for 32 bytes total.

The source table is selected from the low bits of `R9`:

| `(R9 >> 11) & 6` | Source | Description |
| --- | --- | --- |
| `0` | `>40BC` | EVPC default 16-color palette (9918-like colors). |
| `2` | `>40DC` | 16-step grayscale palette. |
| `4` | `>5F10` | Custom palette stored in NOVRAM. |
| `6` | `>5F10` | Custom palette stored in NOVRAM. |

The default palette at `>40BC` is:

```
00 00  00 00  00 06  33 07  07 00  27 03  50 00  07 06
70 00  73 03  60 06  64 06  30 04  65 03  55 05  77 07
```

Each entry is the V9938 9-bit palette pair: first byte contains red in bits
6-4 and blue in bits 2-0; second byte contains green in bits 2-0. The result is
an approximation of the TMS9918A palette, with the standard white-on-cyan text
colors from R7. The grayscale table at `>40DC` is:

```
00 00  11 01  22 02  33 03  44 04  55 05  66 06  77 07
00 00  11 01  22 02  33 03  44 04  55 05  66 06  77 07
```

If `R9` is a small positive value, the routine skips the palette load
entirely. The current configuration routine produces `R9 = 0` (DIP mode or
invalid NOVRAM) or `R9` with bit 15 set (valid NOVRAM), so in normal operation
the palette is loaded; `R9 = 0` selects the default table at `>40BC`.

### Display adjust (`>41E6`)

`>41E6` reads the first NOVRAM field at `>5F00` (8 bytes), packs its low
nibbles into a word, and writes the high byte of that word to R18 (display
adjust). In DIP mode or with no meaningful NOVRAM contents this is normally
zero.

## What the initialization does and does not do

The EVPC power-up routine:

* Detects a V9938 by testing R14 page selection.
* Rewrites the full 9918-compatible register set, with all tables in the low
  16 KB.
* Sets R8 for 128 KB VRAM (`VR = 1`).
* Selects PAL or NTSC, optional 212-line mode and interlace from DIP/NOVRAM.
* Loads a 9918-like palette, a grayscale palette, or a custom NOVRAM palette.
* Reserves a small area at the top of VDP memory.
* Loads R18 from configuration.
* Leaves R14 = 0.

It does **not**:

* Enable any V9938 bitmap or text mode (G4-G7, TEXT2). The VDP stays in
  Graphics 1 / mode 0.
* Touch the V9938 command registers (R32-R46).
* Load a character set. The console GROM's normal character set remains in
  VRAM; the EVPC's own bank 6 font is for the 80-column TEXT80 code.
* Clear the screen or otherwise disturb the existing 9918 display beyond the
  status message and the top-of-memory reservation.

In other words, the routine is a compatibility reset: it makes the V9938 look
like a 9918A with 16 KB of VRAM, then leaves it alone.

## Relationship to js99er's V9938 initialization

js99er solves the same problem in two different ways, depending on the selected
VDP. The plain `V9938` mode patches the console GROM's VDP register default
table at GROM `>0451` before the console initializes the VDP:

```ts
const vdpRegisterData = 0x0451;
grom.setByte(vdpRegisterData + 2, grom.getByte(vdpRegisterData + 2) & 0x0f); // R2
grom.setByte(vdpRegisterData + 4, grom.getByte(vdpRegisterData + 4) & 0x07); // R4
grom.setByte(vdpRegisterData + 5, grom.getByte(vdpRegisterData + 5) & 0x7f); // R5
grom.setByte(vdpRegisterData + 6, grom.getByte(vdpRegisterData + 6) & 0x07); // R6
```

That produces exactly the same table bases as the EVPC DSR:

```
R2 = 00, R3 = 0E, R4 = 01, R5 = 06, R6 = 00, R7 = F7
```

The `V9938/F18A` (CaDD Hybrid) mode no longer patches the GROM. It starts in a
9918A compatibility mode inside the V9938 core: table addresses use the 9918
masks and VRAM addresses are 14 bits. The first access above VRAM `0x3FFF`
switches the card to full V9938 behavior, at which point the unpatched R2-R6
high bits become active. That is the same "lazy 9938" idea as the EVPC DSR,
but implemented in the VDP instead of by rewriting registers after the console
has run.

See `js99er-9938.md` for the full js99er analysis. The EVPC approach is a
runtime register rewrite in the DSR power-up routine; the plain js99er V9938
approach is a static patch of the console's default table; the CaDD approach
is a 9918A compatibility mode that ends on the first extended-VRAM access. All
three keep the V9938 in 9918-compatible Graphics 1 mode with all tables in the
first 16 KB.

## Appendix: raw register table

```
CPU >4092:
01 20  02 00  03 0E  04 01  05 06  06 00  07 F7
08 08  09 02  0A 00  0B 00  0C F7  0D 00  0E 00
0F 00  10 00  11 00  12 00  13 00  17 00  00 00
```

Note that the table skips R20-R22 and R24-R31, and jumps from R19 to R23.
The terminator is the `00 00` after the R23 entry.

## References

* MAME `src/devices/bus/ti99/peb/evpc.cpp` (CRU page selection, NOVRAM,
  palette port, ROM definition).
* MAME `src/devices/video/v9938.cpp` (V9938 register and port behavior).
* Yamaha V9938 MSX-Video Technical Data Book (register definitions and
  power-on palette).
* TI-99/4A Tech Pages, "Headers" (DSR header, power-up, subprogram and DSR
  list formats).
