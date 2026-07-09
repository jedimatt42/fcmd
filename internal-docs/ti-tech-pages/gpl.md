# GPL: Graphic Programming Language

[Scratch-pad usage
](#pad)[Status byte
](#status)[Stacks
](#stacks)[Operands
](#operands)[Standard opcodes
](#opcodes%20table)[Fmt opcodes
](#fmt%20table)[Common assembly instructions](#instr%20table)

[Using the XML instruction
](gpl2.md)[Using the I/O instruction
](gpl2.htm#IO)[Using the COIN instruction
](gpl2.htm#COIN)[Using sprites](gpl2.htm#Dealing%20with%20sprites)

[Scratch-pad dedicated addresses
](gpl2.htm#PAD%20basic)[GPL routines in console GROMs
](gpl2.htm#GROM%20subroutines)[Usefull data in console
GROMs](gpl2.htm#GROM%20data)

##  Introduction

GPL is the language of the GROMs. Since these map at a single byte in
CPU memory they are obviously not suitable to contain assembly language.
Texas Instruments thus designed a special, low-level interpreted
language they called GPL, for "Graphic Programming Language".

GPL is an 8-bit language: instructions are 1-byte long (not counting the
operands), and they deal with bytes. But since the TI-99/4A is a 16-bit
machine, GPL was designed so that most instructions can also handle 2
bytes at a time. However, there is no requirement for these two bytes to
be aligned at a word boundary, on an even address, as is the case in
assembly.

As its name suggests, GPL is mainly screen-oriented. Accessing the VDP
memory is very easy: `CLR V@>0020` will clear a byte in VDP memory at
address \>0020. No need to pass the address to the VDP: this is taken
care of by the interpreter. Furthermore, the GPL instruction FMT will
put the interpreter in a special mode where every instruction deals with
screen ouput.

Finally, GPL is more compact than assembly language: a two-byte GPL
instruction can do the job of a whole assembly subroutine (because it
calls such a routine in the interpreter). This is slightly less obvious
when dealing with CPU memory: the prefered target memory is the
scratch-pad, i.e. a 16-bit memory located at addresses \>8300-83FF.
Although GPL can access the whole memory range, addresses out of the
scratch-pad take more room in the program and thus slow its execution.

There is a drawback though (there always is): GPL is slow when compared
to assembly language. This is of course because assembly language is
executed directly by the TMS9900 microprocessor, whereas GPL must be
interpreted by an assembly program located in the console ROMs. In fact,
most of the ROM in the TI-99/4A is dedicated to the GPL interpreter.

Nevertheless, GPL is a nice language to learn. If you already know
assembly, you'll master it in a couple hours. And if you don't know
assembly, GPL is a more gentle, user-friendly way to approach low-level
languages. Of course, if you want to program in GPL, you'll nee two
things:

- A GRAM device, into which you can place your GPL programs to test
  them. There are ways around it, programs that will let you execute GPL
  from cpu memory, but you'd better go for the real thing.
- A GPL assembler, i.e. a program that translates the human managable
  opcodes and operands into the serie of bytes that constitutes a GPL
  program. There are several such assemblers around. Unfortunately,
  information on GPL was initially hard to find (to say the least:
  personnally I had to disassemble the interpreter to figure out the
  meaning of some opcodes), so the programmers often adopted different
  names for the same instruction: `MUL `vs `MPY `for multiply, etc.

Amoug the first GPL assemblers was that of Michael Weiand: it had its
limitations but it set a standard. A more recent one is the RAG
macro-assembler: as usual with R.A. Green, this is a finely chiseled
piece of software (I shouldn't praise a competitor but had I been aware
of this assembler earlier, I woud never have written mine...). Finally,
there is one in my GPL package, that you can
[download](download.htm#gplassm) from this site.
And now let's see some basic notions on GPL:

A GPL instruction consists in a short mnemonic, called **opcode**
(operation code), optionally followed by one or two **operands**. The
opcode defines the operation to be performed: add, substract, scan the
keyboard, call a subroutine, etc. The operand(s) tell the GPL
interpreter where to find the data for this operation: e.g. add what to
what.

Most assembler let you place a **label** left of the opcode. This is a
convenient way to branch at a given place in your program, without
having to calculate at which memory address it corresponds. Instead of
typing `BR @>6086`, you would type `BR @PLACE1` where PLACE1 is a label
you placed in your program. You could also define labels that correspond
to constants, or to addresses in CPU or VDP memory, but this requires a
special instruction (generally [`EQU`](#EQU)).

Finally, most assemblers allow you to place **comments** after the last
operand. Programs written in low-level languages can be fairly hard to
understand, even when it's your program, that you set aside for a few
months. Therefore it is critical that you comment almost every line,
explaining to yourself what you are doing.

A GPL **source file** consists in a series of GPL instructions, one per
line, and possibly of some assembly-time instructions (i.e. commands for
the assembler). The GPL assembler reads this file and uses it to create
a **code file**, that contains the numeric equivalent of your program:
an opcode is translated into a single byte, operands may require one or
more bytes. A GPL loader then places this program in your GRAM device
where it can be executed. Finally, you could dump the content of your
GRAM device into a **memory-image file**, so that it can be quickly
reloaded without the loader having to process the code file again.

##   Scratch-pad usage

The GPL interpreter makes a heavy use of the scratch-pad memory at
addresses \>8300-\>83FF. Many bytes have a [special
function](gpl2.htm#PAD%20general) and/or are directly used by the
interpreter (for instance, \>83E0-83FF are the workspace registers of
the interpreter). Others are free for you to use: mainly, these are
adresses from \>8300 to \>836D. You've much less choice withTI-Basic or
Extended Basic that reserve [most of these](gpl2.htm#PAD%20basic) for
their own use.

##  Status byte

The GPL interpreter does not use the status register in the TMS9900 CPU,
instead is uses a status byte located in the scratch-pad at address
\>837C. Its stucture is the following:

|      |     |     |     |     |      |      |      |
|------|-----|-----|-----|-----|------|------|------|
| High | Grt | Cnd | Car | Ovf | n.u. | n.u. | n.u. |

**High** means logically higher, i.e. \>FF is higher than \>01.
**Grt** means greater than for signed operations, i.e \>01 is greater
than \>FF (-1).
**Car** means carry.
**Ovf** means overflow.
**Cnd** stands for condition: this bit is tested by conditional branch
instructions (BR and BS).
The last 3 bits are not used.

The status byte may or may not be affected by GPL instructions:

- Status unchanged with:
  `ABS NEG CLR INV FETC CASE PUSH MPY ST EX SRA SRL SLL SRC RND BACK ALL FMT XML CONT EXEC RTB MOVE`
  and fmt opcodes
- Cnd bit reset with: `B BR BSS CALL EXIT RTGR RTN SWGR`
- Cnd modified with:
  `CAR CEQ CGT CGTE CH CHE CLOG COIN GT HIGH OVF RTNC SCAN `
- Status modified with: `ADD AND CZ DEC DECT DIV INC INCT SUB OR XOR `

The Cnd bit can be tested with the `BR `and `BS `instructions. The other
bits can transfered into the Cnd bit with dedicated opcodes
(`HIGH, GT, CAR, OVF`) and then tested with `BR `or `BS`. This transfer
is implicit with some opcodes (e.g. `CGT `sets Cnd if the source operand
is greater that the second operand)

##  GPL Stacks

GPL uses two stacks, located in the scratch-pad memory: the subroutine
stack and the data stack.

#### Subroutine stack

The first one serves to store the return address of procedures called
with `CALL`. The address will be retrieved from the stack by a `RTN `or
`RTNC `instruction and used to return to the calling point. `SWGR `and
`RTGR `also make use of this stack but store the GROM base in addition
to the return address.

The stack pointer is found in byte \>8373. It points to the current
return address, i.e. `CALL `must increment it by two before to save a
new return address, whereas `RTN `reads the address directly and
decrement \>8373 by two afterwards.

The subroutine stack normally grows upwards from \>8380, but this is not
an absolute requirement: theoretically it could be located anywhere in
the scratch-pad. Note that the GPL interpreter does not check for stack
overflow, nor for empty stack: a long serie of nested `CALL `may
therefore crash the interpreter...

####  Data stack

This stack is meant for use by the programmer. Its pointer is to be
found in byte \>8372 and the user can push a byte on it with the GPL
opcode `PUSH`. Here also, \>8372 points at the current byte, i.e. the
pointer is incremented (by one) before saving the byte. For some strange
reason, the symetrical `POP `instruction does not exist (although my
assembler simulates it). You'll have to retrieve the byte "manually"
with `ST *>8372,`*`dest`* and then decrement the pointer with
`DEC @>8372`.

This stack is normally located at \>83A0 and grows upwards with no check
for overflow. Be carefull with that one: bytes \>83C4-C5 contain the
interrupt routine hook. Placing a non-zero value in there will cause the
interrupt service routine to branch at the corresponding address in CPU
memory (now there's an idea...). Again, it is possible to relocate this
stack by just changing the value of the pointer in \>8372, but it has to
be in the scratch-pad since the pointer is a byte.

##  Operands

Most GPL opcodes can use a source and a destination arguments, the major
difference between these two being that a source argument can be
immediate, whereas a destination cannot.

There are many types of arguments, each consist in several symbols
indicating the argument type (@,\*,V@,V\*, etc) followed by an
expression.

#### Symbolic: @\>8342

Specified address in cpu memory.

Uses 1 byte if the address is between \>8300 and \>837F, two bytes if
it's between \>8380 and \>91FF, three bytes otherwise.

    Example:
           CLR  @>8300       Clears byte >8300

####  Indirect: \*\>8342

Scratch-pad address whose least significant byte is found at the
specified address.Note that the final address *must* be in the scratch
pad.

Uses 2 bytes if the specified address is between \>8300 and \>81FF, 3
bytes otherwise.

    Example: If >8342 contains >56, then
           CLR  *>8342       clears byte >8356.

####   Indexed symbolic: @\>0005(@\>8342)

Address calculated by adding the symbolic value (5 in the example) to
the content of the index address (in our example \>8342-8343).

The index address *must* be in the scratch-pad, and can therefore be
abreviated as (@\>42) with some assemblers.

Uses 3 or 4 bytes, according if the symbolic value is between \>8300 and
\>91FF or not.

    Example: if >8342-8343 contain the value >8300, then
           CLR  @5(@>42)       clears byte >8305 (i.e. >8300 + >0005).

####  Indexed indirect: \*\>0005(@\>42)

Scratch-pad address whose least significant byte is to be found at the
address calculated by adding the symbolic value (here \>0005) to the
content of the index address (here \>8342, the \>83 being optional for
indexes).

Uses 3 or 4 bytes, according to symbolic value.

    Example: if >8342-8343 contains >8300, and >8305 contains >12, then
           CLR  *5(@>42)        is equivalent to
           CLR  *>8305          which clears byte >8312

####  VDP symbolic: V@\>0128

Specified address in VDP memory (here \>0128).

Uses 2 bytes if below \>0F00, 3 bytes otherwise.

    Example:
           CLR  V@>0123       clears VDP byte >0123

####  VDP indirect: V\*\>8301

VDP address to be found in scratch-pad, at the specified address (here
\>8301-8302).

Uses 2 bytes if specified address is between \>8300 and \>91FF, 3 bytes
otherwise.

    Example: if >8301-02 contains >0123, then
           CLR  V*>8301       clears VDP byte >0123

####  VDP indexed symbolic: V@\>1111(@\>8301)

VDP address calculated by adding the symbolic value to the content of
the index.

Uses 3 or 4 bytes, according if the symbolic value is below \>0F00 or
not.

    Example: if >8301-8302 contain >2222, then
           CLR  V@>1111(@>01)       clears VDP byte >3333

####  VDP indexed indirect: V\*\>8344(@\>01)

VDP address to be found in the scratch-pad address calculated by adding
the symbolic value to the content of the index.

Uses 3 or 4 bytes, according to the symbolic value.

    Example: If >8301-8302 contain >0010, and >8354-8355 contain >1000, then
           CLR  V*>8344(@>01)       is equivalent to
           CLR  V*>8354             which clears VDP byte >1000

####  Immediate value: \>1234

This may be a byte or a word value, according if the opcode is Doubled
or not. This type of argument cannot be used for destination arguments.

    Examples:
           ST    >14,@>8300        Stores the value >14 in byte >8300
           DST  >1234,@>8300       Stores the value >1234 in bytes >8300-8301

####  Grom address: G@\>0010

Some opcodes require an address in GROM/GRAM memory as an operand. These
are:

the branching instructions: `B, BR, BS, CALL`, and `SWGR`.

the `LOOP `instruction in FMT mode.

the `FETC `instruction (used by subroutines to fetch data bytes
following the `CALL`)

the `MOVE `instruction.

In principle, the syntax is G@*address*, however since these these
instuction have no other possible address mode (except for `MOVE`), some
assemblers will let you ommit the G or even the G@.

####  Move arguments

The `MOVE `opcode can use special argument types, to access G memory or
VDP registers. See the [MOVE](#MOVE) opcode for more details.

##  GPL OPCODES

|  |  |  |  |  |
|----|----|----|----|----|
| Opcode | Arguments | Dob | Stat | Meaning |
| [ABS](#ABS) | *dest* | D | = | Absolute value of *destination* |
| [ADD](#ADD) | *source,dest* | D | S | Addition of *source* to *destination* (a.k.a. A) |
| [ALL](#ALL) | *byte* | . | = | Fills screen with *byte* (\>000-2FF in VDP memory) |
| [AND](#AND) | *source,dest* | D | S | Bitwise "and" of *source* and *destination* operands |
| [B](#B) | *address* | . | 0 | Imperative branch to *address* in GROM |
| [BACK](#BACK) | *byte* | . | = | Set text/screen colors: *byte* = \>TS |
| [BR](#BR) | *address* | . | 0 | Branch to *address* in same GROM if cnd=0 |
| [BS](#BS) | *address* | . | 0 | Branch to *address* in same GROM if cnd=1 |
| [CALL](#CALL) | *address* | . | 0 | Call subprogram at *address* |
| [CAR](#CAR) | . | . | c | Transfer carry bit in cnd (a.k.a. CARR) |
| [CASE](#CASE) | *dest* | D | = | Skip *dest* words in program |
| [CEQ](#CEQ) | *source,dest* | D | c | Are *source* and *dest* equal? |
| [CGT](#CGT) | *source,dest* | D | c | Is *source* greater than *dest*? |
| [CGTE](#CGTE) | *source,dest* | D | c | Is *source* greater or equal to dest (a.k.a. CGE) |
| [CH](#CH) | *source,dest* | D | c | Is *source* higher than *dest*? |
| [CHE](#CHE) | *source,dest* | D | c | Is *source* higher or equal to *dest*? |
| [CLOG](#CLOG) | *source,dest* | D | c | Set cnd if *source "*and" *dest* = 0 |
| [CLR](#CLR) | *dest* | D | = | Clear *dest* |
| [COIN](#COIN) | *source,dest* | D | c | Check for screen objects coincidence |
| [CONT](#CONT) | . | . | = | Continue Basic execution |
| [CZ](#CZ) | *dest* | D | c | Compare *dest* to zero |
| [DEC](#DEC) | *dest* | D | S | Decrement *dest* |
| [DECT](#DECT) | *dest* | D | S | Decrement *dest* by two |
| [DIV](#DIV) | *source,dest* | D | S | Divides *dest* by *source* |
| [EX](#EX) | *source,dest* | D | = | Exchanges *source* and *dest* |
| [EXEC](#EXEC) | . | . | = | Begins execution of Basic |
| [EXIT](#EXIT) | . | . | 0 | System reset |
| [FETC](#FETC) | *dest* | D | = | Fetch next GRAM byte into *dest* |
| [FMT](#FMT) | . | . | = | Enter Format sub-interpreter |
| [GT](#GT) | . | . | c | Transfer gt bit in cnd |
| [HIGH](#HIGH) | . | . | c | Transfer high bit in cnd (= H) |
| [INC](#INC) | *dest* | D | S | Increment *dest* |
| [INCT](#INCT) | *dest* | D | S | Increment *dest* by two |
| [INV](#INV) | *dest* | D | = | Invert *dest* |
| [I/O](#I/O) | *source,dest* | D | = | Special operation *source*, operands in *dest* |
| [MOVE](#MOVE) | *source,msrc,mdest* | . | . | Moves *source* bytes from *msrc* to *mdest* (may be G or Vreg) |
| [MPY](#MPY) | *source,dest* | D | = | Multiply *dest* by *source* (a.k.a. MUL a.k.a. M) |
| [OR](#OR) | *source,dest* | D | S | Bitwise "or" of *source* and *dest* |
| [OVF](#OVF) | . | . | c | Transfer overflow bit in cnd |
| [PARS](#PARS) | *byte* | . | . | Parse basic statement until token *byte* is encountered |
| [PUSH](#PUSH) | *dest* | . | = | Save *dest* on VDP stack |
| [RAND](#RND) | *byte* | . | = | Get random number from 0 to *byte* (a.k.a.RND) |
| [RTGR](#RTGR) | . | . | 0 | Return from a SWGR |
| [RTN](#RTN) | . | . | 0 | Return from a CALL with cnd cleared |
| [RTNC](#RTNC) | . | . | = | Return from a CALL with cnd intact |
| [SCAN](#SCAN) | . | . | c | Keybord scan |
| [SLA](#SLA) | *source,dest* | D | = | Shift *dest* left by *source* bits (a.k.a. SLL) |
| [SRA](#SRA) | *source,dest* | D | = | Shift *dest* right arithmetically, by *source* bits |
| [SRC](#SRC) | *source,dest* | D | = | Shift *dest* right circular, by *source* bits |
| [SRL](#SRL) | *source,dest* | D | = | Shift *dest* right logically, by *source* bits |
| [ST](#ST) | *source,dest* | D | = | Copy *source* in *dest* |
| [SUB](#SUB) | *source,dest* | D | S | Substract *source* to *dest* (a.k.a. S) |
| [SWGR](#SWGR) | *source,dest* | D | 0 | Branch to address in *dest*, using GROM port in *source* |
| [XG-0 to XG-3](#XG0-3) | *dest* | D | ? | Extended GPL. Do not use |
| [XG-4 to XG-E](#XG4-E) | ? | D | ? | Extended GPL. Do not use |
| [XG-F or XG-G](#XG-F) | *source,dest* | D | ? | Extended GPL. Do not use |
| [XG-\$](#XG-$) | . | . | ? | Extended GPL, special entry point. Do not use |
| [XML](#XML) | *byte* | . | = | Execute machine language procedure |
| [XOR](#XOR) | *source,dest* | D | S | Bitwise "exclusive or" between *source* and *dest* |

**Status  **
= Status not affected
S Status modified
c Cnd bit modified
0 Cnd bit reset
? Unknown

**Dob  **
D opcodes can affect 2 bytes if a leading D is added (e.g. DINC instead
of INC).

###   FMT subinterpreter opcodes

These opcodes become active once a FMT instruction has been executed and
remain so until a FEND is encoutered. During this time all the regular
opcodes become invalid. The interpreter thus enters a special mode that
deals exclusively with screen access.

|  |  |  |  |
|----|----|----|----|
| Opcode | Argument | Mode | Meaning |
| [COL](#COL) | *value* | Std | Set column pointer |
| [COL=](#COL=) | *value* | Std | Adjust column pointer \* |
| [FEND](#FEND) | . | Both | Exit format sub-interpreter |
| [FOR](#RPTB) | *value* | Both | Repeat block of instructions *value* times (a.k.a. RPTB) |
| [HCHA](#HCHA) | *value,char* | Both | Display *value* occurences of *char* |
| [HSTR](#HTEX) | *value,dest* | Both | Display *value* chars from *dest* (a.k.a. HTEX value,dest) |
| [HTEX](#HTEX) | *string* | Both | Display *string* |
| [HTEX](#HTEX) | *value,dest* | Both | Display *value* chars from *destination* operand |
| [ICOL](#COL+) | *value* | Both | Increment column pointer by *value* (a.k.a. COL+) |
| [IROW](#ROW+) | *value* | Std | Increment row pointer by *value* (a.k.a. ROW+) |
| [LOOP](#LOOP) | \[*address*\] | Both | End of a RPTB loop. Loop to *address* (a.k.a. NEXT) |
| [NEXT](#LOOP) | \[*address*\] | Both | End of a FOR loop. Loop to *address* (a.k.a. LOOP) |
| [ROW](#ROW) | *value* | Std | Set row pointer to *value* |
| [ROW=](#ROW=) | *value* | Std | Adjust row pointer to *value* \* |
| [RPTB](#RPTB) | *value* | Both | Repeat block of instructions *value* times (a.k.a. FOR) |
| [SCRO](#SCRO) | *byte* | Both | Set screen offset as *byte* |
| [SCRO](#SCRO) | *dest* | Both | Set screen offset from *destination* operand |
| [TCOL](#TCOL) | *value* | Text | Adjust column pointer to *value* \* |
| [TPOS](#TPOS) | *value1,value2* | Text | Set row and column pointers to *value1* and *value2* \* |
| [TROW](#TROW) | *value* | Text | Increment row pointer by *value* \* |
| [VCHA](#VCHA) | *value,char* | Both | Display vertically *value* occurences of *char* |
| [VTEX](#VTEX) | *string* | Both | Display *string* vertically |

**Mode  **
Std: Standard mode or multicolor mode
Text: 40-columns text mode

**\*** Only valid with my GPL assembler, version II.

###  Typical assembly-time instructions

|  |  |  |
|----|----|----|
| Instruction | Arguments | Usage |
| [AORG](#AORG) | *address* | Defines the absolute *address* of the program in GRAM |
| [BYTE](#BYTE) | *byte\[,byte...\]* | Inserts one or more *bytes* of data in GRAM |
| [COPY](#COPY) | *quoted_string* | Switches to source file named in *quoted_string* |
| [END](#END) | . | Signals the end of the program |
| [EQU](#EQU) | *value* | Defines an assembly-time constant as a label equal to *value* |
| [STRI](#STRI) | *quoted_string* | Places an ASCII *string*, with length byte, in GRAM |
| [TEXT](#TEXT) | *quoted_string* | Ditto, without length byte. |

------------------------------------------------------------------------

### Simple opcodes

#### ReTurN

Syntax:

\[label\] RTN \[comment\]

Use:

Exits a procedure called by CALL and returns to the caller. The return
address is fetched from subroutine stack (pointed at by \>8373).

Status byte: cond bit reset.

Opcode: \>00

####  ReTurN with Cnd bit

Syntax:

\[label\] RTNC \[comment\]

Use:

Exits a procedure called by CALL and returns to the caller, keeping
status byte value unchanged. The return address is fetched from
suroutine stack (pointed at by \>8373).

Status byte: unchanged.

Opcode: \>01

####  RANDom

Syntax:

\[label\] RND expression \[comment\]

\[label\] RAND expression \[comment\]

Use:

Generates a random number between 0 and the value of expression. This
number is placed at @\>8378.

Status byte: unchanged.

Opcode: \>02

    Example:
    DICE   RND  5          Generates a random number between 0 and 5
           INC  @>8378     Makes it a number between 1 and 6 (to emulate a dice)

####  SCAN keyboard

Syntax:

\[label\] SCAN \[comment\]

Use:

Scans keyboard in mode found in \>8374. If a key is pressed, or hold
down, its code is placed in \>8375 (otherwise it contains \>FF).
Joystick values (\>04, \>00 and \>FC) are placed in \>8376 and \>8377
for the horizontal and vertical displacement respectively.

Status byte: cnd set if a NEW key has been detected.

Opcode: \>03

    Example:
           ST   >03,@>8374        Keyboard type 3
           CLR  @>83C8            To enable repeats
    L1     SCAN                   Scan keyboard
           BR   L1                No new key pressed
           ST   @>8375,V*>8304    Key pressed: pass it to VDP address in >8304-05

####  BACKground color

Syntax:

\[label\] BACK expression \[comment\]

Use:

Set screen color (and text background in text mode). The color value (0
to 15) is defined in the expression.

Status byte: unchanged.

Opcode: \>04

    Example:
           BACK 7       cyan
           BACK 5       light blue

####  Branch

Syntax:

\[label\] B G@expression \[comment\]

Use:

Branches to the address specified in expression. The correct syntax uses
G@ to indicate GROM memory, but since B always branches to this memory,
the G and even the @ may be ommited with my GPL assembler.

Status byte: cnd bit reset.

Opcode: \>05

    Examples:
           B    G@>A026
           B    @THERE
           B    L1

####  CALL

Syntax:

\[label\] CALL G@expression \[comment\]

Use:

Branches to the address specified in expression, saving current address
on the subroutine stack (pointed at by \>8373) for return with RTN, RTNC
or RTNS.

Since addresses always represent GRAM memory, the G and the @ are
dispensable.

Status byte: cnd bit reset.

Opcode: \>06

    Example:
          CALL G@DISP       Calls routine "DISP"
          CALL >0010        Calls routine located at >0010 in GROM

####  ALL screen

Syntax:

\[label\] ALL expression \[comment\]

Use:

Fills the whole screen with the character specified in expression, by
copying it \>0300 times in VDP memory, starting at V@\>0000.

Status byte: unchanged.

Opcode: \>07

    Example:
    CLSC  ALL 32        Clears screen by filling it with spaces

####  ForMaT sub-interpreter

Syntax:

\[label\] FMT \[comment\]

Use:

Enters the format sub-interpreter: from that point on the regular GPL
opcodes become invalid and are replaced with Fmt opcodes, until an FEND
is encoutered. Interrupts are disabled to avoid troubles with VDP
access.

The Format sub-interpreter is designed for easy screen handling, and is
[described](#FMT%20sub-interpreter) at the end of this section. The
commands are summarized in [this table](#fmt%20table).

Status byte: unchanged.

Opcode: \>08

    Example:
          FMT                       Enter sub-interpreter
          HTEX 'Hello, world'       Displays horizontal text
          FEND                      Exits sub-interpreter

####  HIGHer than

Syntax:

\[label\] HIGH \[comment\]

\[label\] H \[comment\]

Use:

Copies the content of the 'high' bit (logically higher) into cnd bit.

Status byte: high --\> cnd.

Opcode: \>09

    Example:
           DEC  @>8300         Decrement this byte
           HIGH                Thansfer the "High" bit in the "Cnd" bit
           BS   G@NOZERO       Branch if "Cnd" is set, i.e. it >8300 is higher than 0.

####  Greater Than

Syntax:

\[label\] GT \[comment\]

Use:

Copies the content of the 'gt' bit (arithmetically greater than) into
the cnd bit.

Status byte: gt -\> cnd.

Opcode: \>0A

    Example:
           DADD  @COUNT,@TOTAL      Adds these two word variables
           GT                       Transfer the "GT" bit into the "Cnd" bit
           BR    SK21               Branches is "Cnd" is 0, i.e. if Total is not greater than 0.

####  EXIT gpl

Syntax:

\[label\] EXIT \[comment\]

Use:

Exits GPL program and resets the computer. This is similar to pressing
the 'quit' key.

Status byte: cnd bit reset.

Opcode: \>0B

    Example:
    BYEBYE  EXIT       End of my program

####  CARry bit

Syntax:

\[label\] CAR \[comment\]

\[label\] CARR \[comment\]

Use:

Copies the content of the carry bit into the cnd bit.

Status byte: car -\> cnd.

Opcode: \>0C

    Example:
           INC  *>8300       Increments the byte whose address is found in byte >8300
           CAR               Tansfer the "Carry" bit into the "Cnd" bit
           BS  G@TOOBIG      Branch if "Cnd" = 1, i.e if there was a carry

####  OVerFlow

Syntax:

\[label\] OVF \[comment\]

Use:

Copies the content of the overflow bit into the cnd bit.

Status byte: ovf -\> cnd.

Opcode: \>0D

    Example:
          DDIV  12,@>834A        Divides >834A-4B by 12
          OVF                    Transfer the "Ovf" bit into the "Cnd" bit
          BS    G@ERR12          Branch if "Cnd" is set, i.e. if there was an overflow

####  PARSe

Syntax:

\[label\] PARS expression \[comment\]

Use:

Processes the current basic statement until a 'token' value smaller than
expression, or a comma, is encoutered.

Opcode: \>0E

    Example:
           PARS   >B6         Parse expression. Stops of a ")" is encountered (token >B6)

####  eXecute Machine Language

Syntax:

\[label\] XML expression \[comment\]

Use:

Calls a machine language routine. *Expression* should be a byte value:
the first nibble is the table number, the second nibble is the number of
the procedure in this table. See table locations and content in
[here](gpl2.htm#XML).

Status byte: unchanged, unless modified by the assembly routine.

Opcode: \>0F

    Example:
           XML   >10             convert string to number
           DST   >A012,@>A000    store address >A012 in word >A000
           XML   >B0             branch to routine, at address found in >A000 (i.e to >A012)

####  CONTinue

Syntax:

\[label\] CONT \[comment\]

Use:

Exits a Basic subroutine and continues execution of the main basic
program.

Status byte: unchanged.

Opcode: \>10

    Example:
           CONT

####  EXECute

Syntax:

\[label\] EXEC \[comment\]

Use:

Begins execution of a Basic program.

Status byte: unchanged.

Opcode: \>11

    Example:
    MYPG  EXEC

####  ReTurN to Basic

Syntax:

\[label\] RTNB \[comment\]

\[label\] RTB \[comment\]

Use:

Exits a GPL subroutine and returns to Basic program, fetching return
address (in ROM or GROM) from the subroutine stack. For ROM routines,
add 2 to the address, for GROM routines (address bit 0 set) add 1 to the
address.

Status byte: unchanged.

Opcode: \>12

    Example:
           RTNB       We are done

####  ReTurn to GRom

Syntax:

\[label\] RTGR \[comment\]

Use:

This is used to return from a SWGR call, retrieving base and address
from the stack.

CAUTION: due to the original GROM design, a read/write instruction must
precede address setup. Assuming that there will never be such things as
GRAMs, Texas Instruments programmers used a *write* instructions inside
RTGR (and a read inside SWGR, but that's no problem). Thus, each time
you use RTGR, you will erase one byte, at the address corresponding to
the base value (i.e. at G@\>9800 for base \>9800, etc). I know it's a
drag, but there's no way around it! Just make sure that the given byte
does not contain anything important (but a RTN opcode, that turns out to
be \>00).

####  eXtended Gpl: XG-4 to XG-E

Syntax:

\[label\] XG-x \[unknown operand type\] \[comment\]

Use:

These opcodes were reserved for future extension: calling them results
in turning on a putative peripheral card installed at CRU \>1B00 and
branching to address \>4020 in its ROM memory. Such a card does not
exist (to my knowledge) and branching to this address will screw up the
system, since the interpreter does not check for the presence of the
card!

However, now that we have cards, such as Horizon Ramdisk, that do have
available RAM at \>4000, may be somebody will volunteer to design XGPL
(I won't).

Status byte: ?

Opcode: \>14 to \>1E

    Example: You better not use these!

####  eXtended Gpl Special entry point

Syntax:

\[label\] XG-\$ \[comment\]

Use:

This works as the above, but branches to address \>401C in the card ROM.

Status byte: ?

Opcode: \>1F

    Example: Avoid that one, too!

####  MOVE

Syntax:

\[label\] MOVE source,move_source,move_dest \[comment\]

Use:

Copies the number of bytes specified in the first operand(source), from
move_source to move_destination. Move_source and move_dest can be the
regular destination argument types (@, \*, V@ ,V\* with or without
index) or any of the following special types:

\- **G@\>A123** GROM/GRAM address specified in the expression (here
\>A123)

\- **G@\>0012(@\>834C)** Address specified (\>0012), plus the value
found in the index word (\>834C-4D).

\- **G\*\>834A** is a shortcut for G@\>0000(@\>834A) with my assembler.

\- **\#2** Destination is the VDP register specified (legal values: 0 to
7). Can't be used as a source operand.

\- **R@2** is an alternative syntax for the above VDP register
addressing..

Status byte: unchanged.

Opcode: \>20 to \>3F according to arguments types.

    Example:
           ST    >80,V@>0000                    char 32 in Basic (biased by >60)
           MOVE  >0300,V@>0000,V@>0001          clear screen
           MOVE  @>8300,G@PROMPT(@>02),V@>8304  display text found at PROMPT plus the value of >8302
                                                at VDP address found in >834A

Notes:

With surprising insight, the TI designers allowed G@ as a destination
operand. Did they foresee the apparition of GRAM devices?

The VDP registers being write-only registers, R@ cannot be used as a
source operand. Therefore GPL assemblers won't let you do it (unless you
force it with a BYTE instruction, which produces interesting
results...)..

####   Branch if Reset

Syntax:

\[label\] BR G@expression \[comment\]

Use:

Branches to the address specified in expression, if cnd bit is reset.
The G@ is more correct, but may be dispensed with when using my
assembler.

Since many opcodes reset the cnd bit, BR is often identical to B,
excepts that it only requires 2 bytes of memory (versus 3 for B). The
drawback is that the address should not trespass GROM boundaries (0 to
\>1FFF, \>2000 to \>3FFF, etc).

Status byte: cnd bit reset

Opcode: \>40 to \>5F according to address most significant byte.

    Example:
          CZ   @>8300           Compare byte >8300 to zero
          BR   G@ZERO           Branch if it's zero, else reset the "Cnd" bit
          BR   G@NOZERO         Branch anyhow since "Cnd" has been reset

####  Branch if Set

Syntax:

\[label\] BS G@expression \[comment\]

Use:

Branches to the address specified in expression, if cnd bit is set. The
G@ is more correct, but may be dispensed with.The address should not
trespass GROM boundaries (0 to \>1FFF, \>2000 to \>3FFF, etc).

Status byte: cnd bit reset.

Opcode: \>60 to \>7F according to address most significant byte.

    Example:
          CEQ  @>834A,@>8304      Compare byte >834A and >8304
          BS   L12                Branch if they are equal

------------------------------------------------------------------------

### Dobble opcodes

The following opcodes deal with bytes, however by adding a leading D one
can create equivalent opcodes that deal with two consecutive bytes
(irrespective of wich of the two has an even address). For instance,
`NEG @>8301` negates \>8301, whereas `DNEG @>8301` negates \>8301 and
\>8302.

These opcodes can be divided in two groups: those that have an unique
destination operand, and those that have two operands: source and
destination. In this case, the source operand can be an immediate value
(i.e. a constant embedded in the program).

#### ABSolute value

Syntax:

\[label\] ABS destination \[comment\]

Use:

Replaces the content of destination with its absolute value (i.e.
negative numbers become positive).

Status byte: unchanged.

Opcode: \>80 and \>81

    Example:
           ABS   @>8300         Absolute value of byte >8300
           DABS  V*>8304        Absolute values of 2 VDP bytes which addess is in >8304-05

####  NEGate

Syntax:

\[label\] NEG destination \[comment\]

Use:

Negates the content of destination (i.e. sign changes).

Status byte: unchanged.

Opcode: \>82 and \>83

    Example:
           ST    1,V@>1000
           NEG   V@>1000              result is >FF (-1)
           DCLR  V@>1000(@>02)
           DNEG  V@>1000(@>02)        result is 0

####  INVert

Syntax:

\[label\] INV destination \[comment\]

Use:

Inverts each bit in destination (1-\>0 and 0-\>1).

Status byte: unchanged.

Opcode: \>84 and \>85

    Example:
           ST   1,V@>1000
           NEG  V@>1000          result is >FE (-2)
           DCLR V@>1000(@>02)
           DNEG V@>1000(@>02)    result is >FFFF (-1)

####  CLeaR

Syntax:

\[label\] CLR destination \[comment\]

Use:

Clears the content of destination (i.e. puts 0 in it).

Status byte: unchanged.

Opcode: \>88

    Example:
           CLR  *2(@>8303)
           DCLR @COUNT        clears COUNT and COUNT+1

####  FETCh

Syntax:

\[label\] FETC destination \[comment\]

Use:

Fetches the data byte immediately following the previous CALL statement
and puts in into destination. The return address is incremented by one.
This is used to pass parameters to subroutines.

NB: Although it is accepted by some assemblers DFETC is not properly
interpreted (the left byte is always 0, the right byte is the only one
fetched): to fetch 2 bytes, use 2 FETC statements. You could use DFETC
to substitute for FETC and DSRL 8, but that's risky: there may be
versions of the GPL-interpreter that have this bug fixed.

Status byte: unchanged.

Opcode: \>88

    Example:
           CALL  G@SUBR         this is the caller
           BYTE  >02,'T'        it has 2 bytes of data after the call

    **                          this is the called subroutine
    SUBR   FETC  @>8300         fetch first byte: >8300 will contain >02
           FETC  @>8301         ftech second byte: >8301 will contain 'T'

####  CASE jump

Syntax:

\[label\] CASE destination \[comment\]

Use:

Skips several 2-bytes long instructions, according to the value of
destination. These instructions are generally BR statements.

Status byte: unchanged.

Opcode: \>8A and \>8B

    Example:
           ST    2,@>8300
           CASE  @>8300
           BR    G@ONE             this statement is skipped
           BR    G@TWO             this branch is taken
           BR    G@THREE

####  PUSH on stack

Syntax:

\[label\] PUSH destination \[comment\]

Use:

Increments data stack pointer (\>8372) and puts the content of
destination on the stack. This byte can be read back by using \*\>8372
as source operand (generally, you will also `DEC @>8372` afterward.

NB: Although accepted by most assemblers DPUSH is not interpreted
correctly: only the least significant byte of destination is pushed. To
push a word on stack, use two PUSH statements.

Status byte: unchanged.

Opcode: \>8C

    Example:
           PUSH  @VALUE
           PUSH  @VALUE+1

####  Compare to Zero

Syntax:

\[label\] CZ destination \[comment\]

Use:

Compares the content of destination to 0 and return result of the
comparison into the status byte. The cnd bit can be immediately used by
BR and BS, other bits are first to be transfered into cnd with the GT
and HIGH instructions.

Status byte: modified.

Opcode: \>8E and \>8F

    Example:
           CZ    @>8300
           DCZ   V*>8300(@>03)

####  INCrement by one

Syntax:

\[label\] INC destination \[comment\]

Use:

Adds 1 to the content of destination. The result is automatically
compared to 0

Status byte: modified.

Opcode: \>90 and \>91

    Example:
           INC   @>8301            Increment >8301. If it becomes 0...
           BS    G@ZERO            this branch is taken

####  DECrement by one

Syntax:

\[label\] DEC destination \[comment\]

Use:

Substracts 1 from the content of destination. The result is
automatically compared to zero.

Status byte: modified.

Opcode: \>92 and \>93

    Example:
           DEC   @>A000
           DDEC  @DELAY(@TIMER)

####  INCrement by Two

Syntax:

\[label\] INCT destination \[comment\]

Use:

Adds 2 to the content of destination. The result is automatically
compared to 0

Status byte: modified.

Opcode: \>94 and \>95

    Example:
           INCT  @>834A          Increment byte >834A by two
           DINCT @>2004          Increment word >8304-05 by two

####  DECrement by Two

Syntax:

\[label\] DECT destination \[comment\]

Use:

Substracts 2 from the content of destination. The result is
automatically compared to zero.

Status byte: modified.

Opcode: \>96 and \>97

    Example:
    L1     DEC   @COUTER
           BR    L1                 repeat until COUTER equals 0

####  eXtended Gpl XG-0 to XG-3

Syntax:
\[label\] XG-n destination \[comment\]

Use:

These opcodes were intended for use with a GPL expansion card that was
never released. Using them will result in locking the system, since the
interpreter doesn't check for the presence of the card before branching
to it.

Opcode: \>98 and \>99 to \>9E and \>9F

    Example:
           Don't use these! Most assemblers don't accept them anyhow (mine does).

------------------------------------------------------------------------

Opcodes with 2 operands.

#### ADDition

Syntax:

\[label\] ADD source,destination \[comment\]

\[label\] A source,destination \[comment\]

Use:

Adds the content of source to the content of destination and compares
the result to zero.

Status byte: modified.

Opcode: \>A0 to \>A3

    Example:
           ADD   12,@>8300
           DADD  @>8300,V*>834A

####  SUBstract

Syntax:

\[label\] SUB source,destination \[comment\]

\[label\] S source,destination \[comment\]

Use:

Substracts the content of source to the content of destination and
compares the result to zero.

Status byte: modified.

Opcode: \>A4 to \>A7

    Example:
           SUB   10,@TIMER
           BS    G@DONE                branch is taken if TIMER is now 0
           DSUB  @>8300,@>8302

####  MULtiply

Syntax:

\[label\] MPY source,destination \[comment\]

\[label\] MUL source,destination \[comment\]

\[label\] M source,destination \[comment\]

Use:

Multiplies the content of source and destination and puts the result in
destination and the following byte (or bytes for DMPY).

Status byte: unchanged.

Opcode: \>A8 to \>AB

    Example:
           MPY   10,@NUMBER
           DST   @NUMBER,@RESULT
           DMPY  10,@NUMBER
           DST   @NUMBER+2,@RESULT

####  DIVide

Syntax:

\[label\] DIV source,destination \[comment\]

Use:

Divides the content of destination and the following byte (or
bytes for DDIV) by the content of source. If no overflow occured,
the result is placed into destination and the remainder into the next
byte(s).

Status byte: unchanged (ovf set if overflow occured).

Opcode: \>AC and \>AD

    Example:
           DIV   24,@LINES
           ST    @LINES+1,@>8300
           DDIV  >0016,@LINES
           DST   @LINES,@PAGES

####  AND operation

Syntax:

\[label\] AND source,destination \[comment\]

Use:

Performes a bitwise and (0a0=0, 0a1=0, 1a0=0, 1a1=1) with the contents
of source and destination. The result is placed into destination and
compared to zero.

Status byte: modified.

Opcode: \>B0 to \>B3

    Example:
           ST    >37,@>8300
           AND   >F1,@>8300                 result is >31

####  OR operation

Syntax:

\[label\] OR source,destination \[comment\]

Use:

Performes a bitwise or (0o0=0, 0o1=1, 1o0=1, 1o1=1) with the contents of
source and destination. The result is places into destination and
compared to zero.

Status byte: modified.

Opcode: \>B4 to \>B7

    Example:
           DST   >0037,@>8300
           DOR   >4512,@>8300            the result is >4537

####  eXclusive OR operation

Syntax:

\[label\] XOR source,destination \[comment\]

Use:

Performs a bitwise xor operation (0x0=0, 0x1=1, 1x0=1, 1x1=0) with the
contents of source and destination. The result is placed into
destination and compared to zero.

Status byte: modified.

Opcode: \>B8 to \>BB

    Example:
           DST   >0037,@>8300
           DXOR  >4512,@>8300             the result is >4525

####  STore

Syntax:

\[label\] ST source,destination \[comment\]

Use:

Copies the content of source into destination.

Status byte: unchanged.

Opcode: \>BC to \>BF

    Example:
           ST    @>8300,@>2240
           DST   'GO',V@>0202

####  EXchange

Syntax:

\[label\] EX source,destination \[comment\]

Use:

Exchanges the contents of source and destination.

NB: source should not be immediate, since nothing will be written in it!

Status byte: unchanged.

Opcode: \>C0 to \>C3

    Example:
           DST   >0102,@>8300           This puts >01 in >8300 and >02 in >8301
           EX    @>8300,@>8301          Now >8300 = >02 and >8301 = >01

####  Compare High

Syntax:

\[label\] CH source,destination \[comment\]

Use:

Sets cnd bit if source is logically greater than destination.

Status byte: cnd modified

Opcode: \>C4 to \>C7

    Example:
           ST    >45,@>8300
           CH    >FF,@>8300               cnd=1
           CH    >03,@>8300               cnd=0

####  Compare High or Equal

Syntax:

\[label\] CHE source,destination \[comment\]

Use:

Sets cnd bit if source is logically higher or equal to destination.

Status byte: cnd modified

Opcode: \>C8 to \>CB

    Example:
           ST    >45,@>8300
           CHE   >FF,@>8300            cnd=1
           CHE   >03,@>8300            cnd=0

####  Compare Greater Than

Syntax:

\[label\] CGT source,destination \[comment\]

Use:

Sets cnd bit if source is arithmetically greater than destination.

Status byte: cnd modified.

Opcode: \>CC to \>CF

    Example:
           ST    >45,@>8300
           CGT   >FF,@>8300          cnd=0 (>FF is -1)
           CGT   >65,@>8300          cnd=1

####  Compare Greater Than or Equal

Syntax:

\[label\] CGTE source,destination \[comment\]

\[label\] CGE source,destination \[comment\]

Use:

Sets cnd bit if source is arithmetically greater or equal to
destination.

Status byte: cnd modified

Opcode: \>D0 to \>D3

    Example:
           CGTE  '3',@KEY
           BS    SK2

####  Compare EQual

Syntax:

\[label\] CEQ source,destination \[comment\]

Use:

Sets cnd bit if source equals destination.

Status byte: cnd modified

Opcode: \>D4 to \>D7

    Example:
           DCEQ  >AA55,@>2000

####  Compare LOGical

Syntax:

\[label\] CLOG source,destination \[comment\]

Use:

Performs a bitwise "and" with the contents of source and destination
(leaving destination unchanged) and sets cnd bit if the result is zero.

Another way to put it is to say that cnd is set if all bits that are "1"
in source are "0" in destination.

Status byte: cnd modified

Opcode: \>D8 to DB

    Example:
           ST    >82,@>8300
           CLOG  >80,@>8300        cnd =0
           BR    J1                branches if first bit is set in >8300 (it is)
           CLOG  >41,@>8300        cnd =1

####  Shift Right Arithmetic

Syntax:

\[label\] SRA source,destination \[comment\]

Use:

Shifts the content of destination to the right by the number of bits
specified in source, padding the left bits with copies of the sign bit
(first bit).

Status byte: unchanged.

Opcode: \>DC to \>DF

    Example:
           ST    >80,@>8300       i.e. 1000 0000 binary
           SRA    2,@>8300        The result is >E0, i.e. 1110 0000

####  Shift Left Logical or Shift Left Arithmetic

Syntax:

\[label\] SLL source,destination \[comment\]

\[label\] SLA source,destination \[comment\]

Shifts the content of destination to the left by the number of bits
specified in source, padding the right bits with 0.

NB: SLA is identical to SLL some assembler may accept only one of these.

Status byte: unchanged.

Opcode: \>E0 to \>E3

    Example:
           ST    >10,@>8300      i.e. 0001 0000 binary
           SLA   2,@>8300        The result is >40, i.e. 0100 0000
           SLL   @>8304,@>8300

####  Shift Right Logical

Syntax:

\[label\] SRL source,destination \[comment\]

Shifts the content of destination to the right by the number of bits
specified in source, padding the left bits with 0.

Status byte: unchanged.

Opcode: \>E4 to \>E7

    Example:
           ST    >80,@>8300      i.e. 1000 0000 binary
           SRL   2,@>8300        The result is >20, i.e. 0010 0000

####  Shift Right Circular

Syntax:

\[label\] SRC source,destination \[comment\]

Shifts the content of destination to the right by the number of bits
specified in source, padding the left bits with the bits deleted to the
right.

NB: there is no shift left circular, but the equivalent of SLC x is SRC
16-x.

Status byte: unchanged.

Opcode: \>E8 to \>EF

    Example:
           ST    >81,@>8300      i.e. 1000 0001 binary
           SRC   2,@>8300        The result is >60, i.e. 0110 0000
           SRC   14,@>8300       Now it became >81 again (equals SLC 2,@>8300)

####  COINcidence

Syntax:

\[label\] COIN source,destination \[comment\]

BYTE map

DATA table

Use:

Checks for screen objects coincidence. Source and destination contain
the coordinates of two objects (row/column). The COIN statement must be
followed by three bytes of data: the first contain the mapping value (0:
perfect match, 1: 2 by 2 pixels, 2: 4 by 4 pixels), the next two bytes
contain the address of the coincidence table. This is a table that you
provide, with all possible positions of the two touching/overlapping
objects, and for each position a bit indicating if you consider it
coincident or not.

For more details and examples on how to use this complicated but usefull
instruction, click [here](gpl2.htm#COIN).

Since arguments must contain 2 bytes each, only DCOIN is adequate.
However my assembler is smart enough to accept COIN, and considers it as
DCOIN.

Status byte: cnd modified

Opcode: \>EC to \>EF

####  eXtended Gpl XG-F and XG-G

Syntax:

\[label\] XG-n source,destination \[comment\]

Use:

These opcodes were intented for use with a never released GPL expansion
board. They should not be used since the interpreter does not check for
the presence of the card before branching to it, resulting in lock-ups.

Opcode: \>F0 to \>F4 and \>FC to \>FF

    Example: Don't use these, chaps!

####  Input/Ouput

Syntax:

\[label\] I/O source,destination \[comment\]

Use:

Source contains a number from 0 to 6 indicating the type of i/o
operation:

- 0: play sound list, from GROM.
- 1: play sound list, from VDP. For these two, the destination operand
  contains two bytes:
  - 1+2 contain the address of the sound list. Note that the address is
    passed as a word (assembly MOV), so the destination operand must be
    even (for once word boundaries do matter).
- 2: CRU input
- 3: CRU output For these two, the destination operand contains a list
  of 4 bytes:
  - 1+2 contain the CRU address divided by 2 (e.g. \>0820 for \>1400)
  - 3 the number of bits to transfer
  - 4 the scratch-pad address of the data byte(s).
- 4: cassette write
- 5: cassette read
- 6: cassette verify For these three, the destination contains a list of
  4 bytes:
  - 1+2 contain the number of data bytes
  - 3+4 the address of these bytes in VDP memory. Cnd bit will be set if
    an error occurs.
    N.B. the cassette motor must be turned on/off from GPL by writing 1
    or 0 to CRU bits 22 (CS1) or 23 (CS2).

DI/O is identical to I/O (except that the operation number should be
\>0000 to \>0006) since destination, being larger than 2 bytes, is
fetched by pointer rather than by using the standard interpreter data
fetching routine.

Follow these links for more detailed information on
[sound](gpl2.htm#sound), [CRU handling](gpl2.htm#cru) and [cassette tape
routines](gpl2.htm#cassette).

Status byte: cnd set upon cassette read or verify error.

Opcode: \>F4 to F7

    Example:

           DST   22,@>834A        CRU bit 22: address >002C divided by 2.
           ST    1,@>834C         1 bit to be transfered
           ST    >04,@>834D       from/to >8304
           ST    >FF,@>8304       data: all bits "1"
           I/O   3,@>834D         write 1 bit (CS1 motor on)

           DST   >0100,@>8300     256 bytes
           DST   >1000,@>8302     located at >1000 in VDP mem
           I/O   4,@>8300         to be written to cassette

           CLR   @>8304           change data: all bits "0"
           I/O   3,@>834D         write 1 bit (CS1 motor off)

####  SWitch GRoms

Syntax:

\[label\] SWGR source,destination \[comment\]

Use:

Branches to the address found in destination, changing grom base to the
value found in source. The return address and current bases are pushed
on the subroutine stack to allow returns via the RTGR opcode.

Be carefull: source should contain a valid port value: no checking is
done and wrong values result in lock-ups!

Theoretically, SWGR (as opposed to DSWGR) cannot be used since the most
significant byte of the port value would be \>00 (or \>FF). However, my
assembler is smart enough to assume that if you say SWGR, you actually
meant DSWGR and it 'appends' the missing D.

Status byte: cnd bit cleared.

Opcode: \>F8 to \>FB

    Example:
           DST   >604A,@>8300          Address where to branch
           DSWRG >9804,@>8300          Branch at >604A using GROM port >9804

------------------------------------------------------------------------

The FMT sub-interpreter

When then opcode FMT is encountered, all the regular GPL opcodes become
invalid and are replaced by special opcodes, intended for easy screen
handling. One of these is the FEND opcodes, that quits the ForMaT mode
and returns to regular GPL.

Two screen pointers are used by the display instructions, a current
column pointer (\>837F) and a current row pointer (\>837E). These two
pointers are updated after each display instruction, in order to point
at the next available screen position, but neither can be greater than
31. The content of the current screen position, can be retrieved at
\>837D (current character).

Notes:

- The FMT sub-interpreter assumes that the screen image table is located
  between 0 and \>0300 in VDP mem. Although it is able to write up to
  \>03FF, addresses comprised between \>0300 and \>031F are interpreted
  as \>0000 to \>001F, which is a big problem in text mode, but
  unfortunately unavoidable (use a MOVE statement to write on text-line
  19).
- The multicolor mode is supported.
- Interrupts are disabled, while in FMT mode.
- Status byte is never affected.
- Some assembly instructions are valid in both modes, some are GPL
  specific and some FMT specific.

####  Horizontal TEXt

Syntax:

\[label\] HTEX quoted_string \[comment\]

or

\[label\] HTEX defined_expr,source \[comment\]

\[label\] HSTR defined_expr,source \[comment\]

Use:

Displays the content of quoted_string (1 to 32 characters) to the
current screen position. Or displays the number of characters specified
in defined_expression (from 1 to 28) and read from source.

Opcode: \>00 to \>1F (quoted string) and \>E0 to \>FB (source).

    Example:
           HTEX  'Hello, world'
           HTEX   32,@ NAME

####  Vertical TEXt

Syntax:

\[label\] VTEX quoted_string \[comment\]

Use:

Displays vertically the content of quoted_string (1 to 32 characters) on
screen, strarting from current screen position.

Opcode: \>20 to \>3F

    Example:
           VTEX   'Ordinate'

####  Horizontal CHAracters

Syntax:

\[label\] HCHA defined_expression,expression \[comment\]

Use:

Repeats horizontally the character specified in expression, the number
of times specified in defined_expression (from 1 to 32).

Opcode: \>40 to \>5F

    Example:
           HCHA   12,'*'
           HCHA   32,>40

####  Vertical CHAracters

Syntax:

\[label\] VCHA defined_expression,expression \[comment\]

Use:

Repeats vertically the cahracter specified in expression, the number of
times specified in defined_expression (from 1 to 32).

Opcode: \>60 to \>7F

    Example:
           VCHA   24,'!'

####  COLumn increment

Syntax:

\[label\] COL+ expression \[comment\]

\[label\] ICOL expression \[comment\]

Use:

Increments the column pointer by the content of expression (1 to 32).
This may result in incrementing the row pointer by one, if end of line
is reached.

Opcode: \>80 to \>9F

    Example:
           COL+  4

####  ROW increment

Syntax:

\[label\] ROW+ expression \[comment\]

\[label\] IROW expression \[comment\]

Use:

Increments the row pointer by the content of expression (1 to 32).

Opcode: \>A0 to \>BF

    Example:
           ROW+  1

####  RePeaT Block

Syntax:

\[label\] RPTB defined_expr \[comment\]

\[label\] FOR defined_expr \[comment\]

Use:

Repeats the display instructions comprised between the RPTB/FOR and the
next LOOP/NEXT intructions, for the number of times specified in
defined_expr. The number of repeats that remain to be executed is stored
on the subroutine stack (pointed at by \>8373).

Opcode: \>C0 to \>DF

    Example:
           RPTB  8       to erase a 8x8 window
           HCHA  8,' '
           COL+  1
           NEXT          loop back above

####  LOOP

Syntax:

\[label\] LOOP \[G@ expression\] \[;comment\]

\[label\] NEXT \[G@ expression\] \[;comment\]

Use:

Ends a RPTB loop. Normally the operand field should contain the address
were to branch, but most of the time this is the address immediately
following the RPTB, thus my assembler keeps track of 16 nested RPTB and
provides the right address if none was specified. However, if you would
like to branch to a different address, or to nest more than 16 RPTB (not
very likely to work anyway, since subroutine stack will overflow) you
should provide the required G address.

Opcode: \>FB (after RPTB)

    Example:
           RPTB  5
           HTEX  'TITLE'       this is not repeated
    L1     COL+  27            but all this is
           HCHA  '.....'
           LOOP  L1

####  Fmt END

Syntax:

\[label\] FEND \[comment\]

Use:

Exits the FMT sub-interpreter, and returns to regular GPL opcodes.

Opcode: \>FB

    Example:
           FMT
           HTEX  'HELP!'
           FEND

####  SCReen Offset

Syntax:

\[label\] SCRO defined_expr \[comment\]

or

\[label\] SCRO source

Use:

A byte value (0 to 255) taken either from defined_expr, or from a source
argument, will be added to any byte displayed by the FMT statements.
This screen offset is automatically set to 0 when entering the FMT
sub-interpreter.

It is mostly usefull in Basic, where there is a screen bias of \>60.

Opcode: \>FC (defined_expr) or \>FD (source).

    Example:
           FMT
           SCRO >60         basic bias
           HTEX 'TI-BASIC READY'
           FEND

####  ROW set

Syntax:

\[label\] ROW expression \[comment\]

Use:

Sets the current row pointer to the byte value contained in expression
(0 to 31). This uses two bytes of G memory, whereas ROW+ only requires
one...

Opcode: \>FE

    Example:
           ROW   5

####  COLumn set

Syntax:

\[label\] COL expression \[comment\]

Use:

Sets the current column pointer to the value contained in expression (0
to 32). The row pointer in not affected. This requires two bytes of G
memory, whereas COL+ only requires one.

Opcode: \>FF

    Example:
           FMT
           ROW 1
           COL 3
           HTEX   'MAIN MENU'
           COL=   3
           HCHA   9,'='            underline
           FEND

####  COLumn equal

Syntax:

\[label\] COL= expression \[comment\]

Use:

Actually this is not an opcode but an assembly-time instruction used
with my GPL assembler to issue a COL+ statement, bringing the cursor in
the column specified in expression. If the specified column is inferior
or equal to the current one, the cursor gets down one line.

The advantage on a COL statement is that it only takes one byte, the
advantage on a COL+ statement is that it saves you the burden of
calculating the current cursor position.

However be aware that the assembler calculates cursor position in order
of appearance of the FMT statements, thus you should not use COL=
inside, or after RPTB-LOOPs, nor should you assume its value will be
retained from a FMT-FEND block to another (although it may be, if they
are executed in the same order they are assembled).

    Example:
           ROW  2
           COL  5
           HTEX 'Last name:'
           COL= 5                 brings cursor back on column 5, on next line
           HTEX 'First name:'     In this example COL= 5 is assembled as COL+ 22

###  Text mode

The FMT sub-interpreter is essentially meant for use in basic mode (32
columns). In text mode (40 columns) some instructions don't work
properly, these are COL, COL=, ROW and ROW+ that all assume there is 32
characters per line.

To provide you with a support for text mode, some pseudo-opcodes have
been created. These actually are assembly-time instructions, that are
assembled into a serie of regular opcodes. They can only be used with my
GPL assembler.

Notes:

To toggle text mode on, you should modify VDP register 1 with a MOVE
instruction. For instance set it as \>F0. Text and background colors are
determined by VDP register 7.

Remember that line 19 is not accessible by the Fmt sub-interpreter (it's
echoed to line 0), so you'll have to use a MOVE statement to write to
that line.

####  Text POSition

Syntax:

\[label\] TPOS defined_expr1,defined_expr2 \[comment\]

Use:

This is the equivalent of a ROW and a COL statements: defined_expr1
should contain screen row and defined_expr2 screen column to set the
pointer to.

This statement is assembled as a ROW and a COL statements, taking into
account the fact that there are now 40 characters per line.

    Example:
           TPOS   5,1          text row 5, column 1
                               Assembled as ROW 6, column 9 (6*32 + 9 = 5*40 + 1)

####  Text ROW

Syntax:

\[label\] TROW defined_expr \[comment\]

Use:

This is the text mode equivalent of the ROW+ opcode. It is assembled as
a ROW+ and a COL+ statement.

N.B. COL+ itself works normally in text mode, but is limited to 32
characters.

    Example:
           TROW  3        gets down 3 lines (of 40 chars)
                          Assembled as ROW+ 3, COL+ 24 (3*32 + 24 = 3*40)

####  Text COLumn

Syntax:

\[label\] TCOL expression \[comment\]

Use:

This is the text mode equivalent of the COL= opcode. It calculates the
current cursor position and issues two COL+ statements to bring the
cursor to the column specified in expression.

If the specified column is inferior or equal to the current position of
the cursor PLUS ONE, the cursor gets down one line.

See COL= for restrictions of use.

    Example:
           TPOS  5,1
           HTEX  'Input file name:'      (16 characters)
           TCOL  1                       One line down, column 1
           HCHA  10,'_'
    * In this example, TCOL 1 is assembled as COL+ 10 and COL+ 14
    * (16 characters plus 24 columns = 40, i.e. next line).
------------------------------------------------------------------------

Assembly-time instructions

Any GPL assembler will provide you with some more instructions that may
or may not issue code, but control the function of the assembler. As
they are assembler-specific I can't describe them here (have a look at
the doc included in my GPL package for instructions controlling my GPL
assembler). There are some constants however that you will almost always
encounter:

#### BYTE

Syntax:

\[label\] BYTE expression \[comment\]

Use:

Lets you insert one or more bytes of data into your GPL program

    Example:
          CALL  @MYSUB         call a subroutine
          BYTE  >12,'T',>00    3 bytes of data that MYSUB can read with FETC

####  TEXT

Syntax:

\[label\] TEXT quoted_string \[comment\]

Use:

Lets you enter an ascii string as data inside the GROM memory. Single or
dobble quotes may be used, depending on the assembler (mine use double
quote to cause a bias to be added).

    Example:
           TEXT  'This is a test'

####  STRIng

Syntax:

\[label\] STRI quoted_string \[comment\]

Use:

Lets you enter an ascii string, preceded with a length byte, as data
inside the GROM memory. Single or dobble quotes may be used, depending
on the assembler (mine use double quote to cause a bias to be added).

    Example:
           STRI  'This is a test'       is equivalent to:
           BYTE  14                     string length
           TEXT  'This is a test'       string

####   EQUate

Syntax:

label EQU defined_expression \[comment\]

Use:

Defines an assembly-time constant, i.e. a label that you will be able to
use within your program, but that the assembler will replace with the
value specified in defined_expression. This is one of the rare cases
when a label is absolutely required.

    Example:
    KEY    EQU   >8375        defines "KEY" as number >8375
           SCAN               scan keyboard
           ST    KEY,@>8301   stores >8375 (key value) in @>8301

####  Absolute ORiGin

Syntax:

\[label\] AORG address \[comment\]

\[label\] GROM address \[comment\]

Use:

Tells the assembler (and the loader) where the program will be located
in GRAM memory. Should generally appear at the top of the program, but
may also be used to skip a large blank space.

    Example:
           AORG  >6000             begins a GRAM address >6000

####   COPY file

Syntax:

\[label\] COPY quoted_string \[comment\]

Use:

If your source file gets very long, it can be a problem to load it into
a text editor. This instruction causes the assembler to switch to the
source file specified in the quoted string. Single or double quotes may
be used according to the assembler. Generally, assembly will resume with
the current file once the "copied" file has been completely processed.
Some assemblers may allow nested COPY whithin copied files, but that's
not always the case.

    Example:
           COPY  'DSK1.TEST-C'

####   END of program

Syntax:

\[label\] END \[comment\]

Use:

Marks the end of the program and instructs the assembler to ignore
whatever follows.

------------------------------------------------------------------------

[Next page](gpl2.md)

Revision 1. 6/5/99 Preliminary. Adapted from the doc in my GPL
package.
Revision 2. 6/12/99 Polished
[Back to the TI-99/4A Tech Pages](titechpages.md)
