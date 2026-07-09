# Console ROMs

The TI-99/4A console contains two 4K ROMs that correspond to addresses
\>0000 to \>1FFF. One of them holds the even-address bytes, the other
the odd-address bytes. Together they allow for full 16-bit access from
the TMS9900 CPU.

[Pinout
](#Pinout)[Content](#Contents)

## Pinout

             +----+--+----+
          A7 |1 o  T    24| Vcc              TMS 4732 / 2352
          A6 |2    M    23| A8
          A5 |3    S    22| A9
          A4 |4    4    21| CS2*
          A3 |5    7    20| CS1*
          A2 |6    3    19| A10
          A1 |7    2    18| A11
          A0 |8    /    17| D7
          D0 |9    2    16| D6
          D1 |10   3    15| D5
          D2 |11   5    14| D4
         Vss |12   2    13| D3
             +------------+

Power supply
**Vcc** +5V
**Vss** Ground
CPU interface
**A0-A11** Address bus. Connected to the address bus, lines A3 to
A14 in the console.
**D0-D7** Data bus. In the TI-99/4A, one chip is connected to lines D0
to D7, the second chip to lines D8-D15.

**CS1\*** Chip select. In the TI-99/4A, active (low) when memory in the
range \>0000-1FFF is accessed. The selection is performed by a 74LS138
decoder, with A0, A1 and A2 as inputs, and MEMEN\* as enabling input
G2A\*. Output Y0\* is connected to CS1\* of both ROM chips via a 1K
resistor.
**CS2\*** Chip select. Ditto. Hardwired to ground in the TI-99/4A..

##  Contents

The TMS9900 microprocessor uses the beginning of its address space for
special purposes. The console ROMs contain the adequate values for (some
of) these features.

Address \>0000: vector for power-up (WS = \>83E0, PC = \>0024)

Addresses \>0000 to \>003F contain vectors for interrupts levels 0 to
15. In the TI-99/4A, all interrupts are hard-wired as level 1, so only
this vector is relevant. Nevertheless, a vector for level 2 is also
present (and level 0 is the same as power-up).
Address \>0004: Interrupt vector 1 (WS = \>83C0, PC = \>0900)
Address \>0008: Interrupt vector 2 (WS = \>83C0, PC = \>0A92)

Addresses \>0040 to \>007F contain vectors for the 16 XOP opcodes. Only
the first two are implemented for sure, although some console carry a
third one. The remaining addresses contain assembly language, which may
or may not result in meaningfull vectors (see discussion of the [XOP
instruction](tms9900.htm#XOP) in my TMS9900 page).
Address \>0040: XOP0 (WS = \>280A, PC = \>0C1C) Calls a never-released
extended GPL card (i.e. crashes)
Address \>0044: XOP1 (WS = \>FFD8, PC = \>FFF8) User defined
operation.
Address \>0048: XOP2 (WS = \>83A0, PC = \>8300). User-defined operation.
Not on all consoles!.

After these special addresses, the console ROMs mainly contain:

The [GPL](gpl.md) interpreter,
The [keyboard scanning routine](keyboard.htm#Built-in),
The [interrupt service routine](ints.htm#ISR),
Routines that scan the GROM and card ROM headers for subprograms, DSR,
etc.
Some [mathematical](reals.htm#routines) subroutines (many others are in
GROM),
Low-level routines for [cassette](cassette.htm#ROM%20routines)
operations (DSRs are in GROM).

Here is a more detailed description of the ROM contents. For a commented
listing, see Heiner Martin's book: TI-99/4A Intern ([pdf
560k](http://99er.hispeed.com/files/tiintern.pdf)).


R9).
interrupts).
(&gt;04B2).
R6).
CARRY, OVF, H, GT,
B, BS, BR, ABS, NEG, CLR, INV, FETCH, CASE, PUSH, DECT, INCT, INC, SUB,
DEC, ADD,
AND, OR, XOR, ST, EX, SRA, SLL, SRL, SRC, MPY, and DIV (see #GPL%20tabletable below).
ICOL, RPTB,
FEND, LOOP, ROW, COL, SCRO, HTEX (see table
below).
RTNC, CALL,
PUSH, and various subroutines used by the interpreter (see #GPL%20tabletable below).
&gt;094A: VDP interrupts.
power-up routines (= DSRLNK).
routines.
zero.
bytes in &gt;8354.
of &gt;8376).
needed).
integer.
TMS9901 timer interrupts only).
&gt;13 (&gt;1648) and XML &gt;14 (&gt;164E).
Basic statement.
PARSE), GO, ON, GOSUB, GOTO,
RETURN, IF, LET, NEXT.
stack.
Alternate entry at &gt;1E9C, returns with CONT.
value stack

Detailed table of GPL interpreter routines

|  |  |  |  |  |  |
|----|----|----|----|----|----|
| Address | Opcode | Address | Opcode | Address | Opcode |
| \>0024 | EXIT | \>019A | XOR | \>05C8 | I/O |
| \>004E | SWGR | \>019E | ST | \>05D6 | I/O sound |
| \>00CC | CGTE | \>01A2 | EX | \>05E8 | I/O cru in |
| \>00D6 | CH | \>01B0 | SRA | \>05EA | I/O cru out |
| \>00DA | CHE | \>01B4 | SLL | \>0608 | XML |
| \>00DE | CGT | \>01B8 | SRL | \>061E | MOVE |
| \>00E2 | CLOG | \>01C2 | SRC | \>06D2 | COIN |
| \>00EA | CZ | \>01CE | MPY | \>07AA | Arguments decoding |
| \>00EC | CEQ | \>01EA | DIV | \>082C | RTGR |
| \>00F4 | CAR, OVF, HIGH, GT | \>017A | RND | \>0838 | RTN |
| \>0104 | B | \>029E | BACK | \>083E | RTNC |
| \>010E | BS | \>02AE | SCAN | \>0842 | Pop address from substack |
| \>011A | BR | \>04DE | FMT Subinterpreter | \>085A | CALL |
| \>0136 | ABS | \>0502 | Fmt VCHA | \>0864 | Push address on stack |
| \>013A | NEG | \>0504 | Fmt HCHA | \>0880 | Set screen ptr to write |
| \>013E | CLR | \>0508 | Fmt VTEX | \>0884 | Set screen ptr to read |
| \>0140 | INV | \>050A | Fmt HTEX | \>08AA | Set address in multicolor mode |
| \>0144 | FETC | \>0532 | Fmt IROW | \>0C0C | Extended-GPL opcodes |
| \>0162 | CASE | \>0534 | Fmt ICOL | \>0C1A | Extended-GPL opcode \>1F |
| \>016E | PUSH | \>053A | Fmt RPTB | \>1346 | I/O cassette write |
| \>0182 | DECT | \>0550 | Fmt LOOP, FEND | \>1426 | I/O cassette verify |
| \>0184 | INCT | \>0584 | Fmt ROW, COL | \>142E | I/O cassette read |
| \>0186 | INC, SUB | \>058E | Fmt SCRO | \>18C8 | PARS |
| \>0188 | DEC, ADD | \>0596 | Fmt HSTR | \>1920 | CONT |
| \>0190 | AND | \>05A2 | ALL | \>1968 | EXEC |
| \>0196 | OR |   |   | \>19F0 | RTB |
Revision 1. 3/2/00 Preliminary.
Revision 2. 3/4/00 OK to release.
[Back to the TI-99/4A Tech Pages](titechpages.md)
