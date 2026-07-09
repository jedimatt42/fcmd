## Using the XML instruction

The GPL instruction XML is used to eXecute Machine Language procedures.
You can call up to 256 different procedures, divided in 16 tables of
upto 16 procedures each. The XML instruction requires a one byte
immediate operand in the form \>XY, where \>X is the table number, and
\>Y the procedure number in that table.

The address of the procedure itself is fetched from the proper table and
branched at by the interpreter.

Example: XML \>1B call the 12th procedure in the second table (numbers
start from zero).

The locations of the 16 tables are pre-defined:

|       |         |                                    |
|-------|---------|------------------------------------|
| Table | Address | Memory                             |
| \>0   | \>0D1A  | Console ROM                        |
| \>1   | \>12A0  | "                                  |
| \>2   | \>2000  | Low memory expansion               |
| \>3   | \>3FC0  | "                                  |
| \>4   | \>3FE0  | "                                  |
| \>5   | \>4010  | Peripheral card ROM (or RAM)       |
| \>6   | \>4030  | "                                  |
| \>7   | \>6010  | Cartridge ROM/ Rambo bank (RAM)    |
| \>8   | \>6030  | "                                  |
| \>9   | \>7000  | Cartridge bank / Rambo bank        |
| \>A   | \>8000  | Decoded as \>8300 on most machines |
| \>B   | \>A000  | High memory expansion              |
| \>C   | \>B000  | "                                  |
| \>D   | \>C000  | "                                  |
| \>E   | \>D000  | "                                  |
| \>F   | \>8300  | Scratch-pad memory                 |

The first two tables being in ROM memory, their content is predefined.
The others might be filled by the user, allowing for combination of GPL
and machine language.

####  Table \>0 Floating point calculation routines.

|                |         |                                                   |
|----------------|---------|---------------------------------------------------|
| XML            | Address | Use                                               |
| \>00           | \>0000  | Not used, dummy value                             |
| [\>01](#xml01) | \>0F54  | Rounds number in \>834A-\>8351 to 8 bytes.        |
| [\>02](#xml02) | \>0FB2  | Rounds this number. Number of decimals in \>835C. |
| [\>03](#xml03) | \>0FA4  | Check number in \>834A-51 and saves status.       |
| [\>04](#xml04) | \>0FC2  | Overflow / underflow.                             |
| [\>05](#xml05) | \>0FCC  | Overflow (part of the above procedure).           |
| [\>06](#xml06) | \>0D80  | Adds floating point numbers (i.e. reals).         |
| [\>07](#xml07) | \>0D7C  | Substracts reals.                                 |
| [\>08](#xml08) | \>0E88  | Multiplies reals.                                 |
| [\>09](#xml09) | \>0FF4  | Divides reals.                                    |
| [\>0A](#xml0a) | \>0D3A  | Compares reals.                                   |
| [\>0B](#xml0b) | \>0D84  | Addition with stack.                              |
| [\>0C](#xml0c) | \>0D74  | Substraction with stack.                          |
| [\>0D](#xml0d) | \>0E8C  | Multiplication with stack.                        |
| [\>0E](#xml0e) | \>0FF8  | Division with stack.                              |
| [\>0F](#xml0f) | \>0D46  | Comparison with stack.                            |

####  Table \>1 Various utilities.

|  |  |  |
|----|----|----|
| XML | Address | Use |
| [\>10](#xml10) | \>11AE | Converts string to number, from Basic. |
| [\>11](#xml11) | \>11A2 | Same as above, entry point from GPL. |
| [\>12](#xml12) | \>12B8 | Converts real to integer. |
| [\>13](#xml13) | \>1648 | Fetches a Basic symbol address, from statement. |
| [\>14](#xml14) | \>164E | Fetches the value of a Basic symbol. |
| [\>15](#xml15) | \>1642 | Assigns a value to a Basic variable. |
| [\>16](#xml16) | \>15D6 | Searches the Basic symbol table, for any symbol. |
| [\>17](#xml17) | \>163C | Pushes a value on the VDP memory value stack. |
| [\>18](#xml18) | \>1F2E | Fetches the value saved as above. |
| [\>19](#xml19) | \>0AC0 | Branches to a dsr/sbr if it's in ROM. |
| [\>1A](#XML%20%3E1A) | \>0B24 | Branches to a dsr/sbr if it's in GROM. |
| [\>1B](#xml1b) | \>1868 | Fetches next Basic token from current statement. |
| \>1C | \>C120 | Not used, dummy value (but could be usefull...) |
| \>1D | \>834A | " |
| \>1E | \>1342 | Not used, dummy value |
| \>1F | \>04C0 | " |

##  Using the I/O instructions

The GPL opcode I/O n,dest can call 7 procedures:


Bytes 1+2: CRU address/2
Byte 3: number of bits
Byte 4: data address (&gt;83xx)
Bytes 1+2: number of bytes
Bytes 3+4: address in VDP memory

###  Sound lists

- I/O  0,dest      in GROM/GRAM
      I/O  1,dest      in VDP memory

*dest* contain the address of a sound list in GRAM/GROM or VDP memory.

The [format](ints.md) of this sound list is described in the page
dealing with the interrupts, since the list will be played by the
interrupt service routine.

    Examples:

    SLIST  BYTE  6         number of sound bytes
           BYTE  >AE,>0F   440H Hz on generator 2
           BYTE  >B0       full volume on generator 2
           BYTE  >D5       minus 10 dB on generator 3
           BYTE  >C6,>0D   middle C on generator 3
           BYTE  60        for 1 second (USA value. Use 50 in europe)

           BYTE  2         size, next sound
           BYTE  >E5       white noise, at 3496 Hz
           BYTE  >FE       low volume (-28 dB)
           BYTE  120       for 2 seconds

           BYTE  0         size=0 means "branch at"
           DATA  SLIST     address in gram (forever loop)

    HUSH   BYTE 4          4 bytes in that sound
           BYTE >9F,>BF,>DF,>FF    turn all generators off
           BYTE 0          duration=0 means "end of list"

Notes:

The address of the table is saved at \>83CC with memory type indicated
in \>83FD, bit 7: \>00 = grom, \>01 = vdp.

The byte \>83CE contains 0 when all sounds have been played. Testing it
is a good way to wait for the end of a sound list.

###  Dealing with the CRU

- I/O 2,dest           cru in
      I/O 3,dest           cru out

*dest* contain the first of 4 bytes, in cpu memory (! at even address !)

2 bytes: CRU address divided by 2 (i.e. absolute bit number)

1 byte: Number of bits to read/write (0-15)

1 byte: Source/destination in scratch-pad memory (\>83xx). The least
significant (rigthmost) bits are passed first.

    Example:
           DST   >0012,>8300      keyboard column address (>0024)
           ST    >03,>8302        pass 3 bits
           ST    >4A,>8303        from >834A
           ST    >01,>834A        column 1
           I/O   3,>8300          write keyboard column

           DST   >0003,>8300      keyboard row address (>0006)
           ST    >08,>8302        pass 8 bits
           I/O   2,>8300          read row of 8 keys
           INV   >83A4            key pressed=1

###  Cassette operations

It's probably easier and safer to access cassettes with a PAB, having
CS1 or CS2 as a filename. However if you insist on doing it yourself,
here are some clues:

- I/O 4,dest         read
      I/O 5,dest         write
      I/O 6,dest         verify

The motor should first be turned on (1), then off (0) with cru bits
\>0016 for CS1 and \>0017 for CS2.

*dest* contains the first of 4 consecutive bytes in cpu memory (! even
address !)

1 word number of bytes to read/write/verify

1 word address of data buffer, in vdp memory

In case you're curious to know how the data are encoded on the magnetic
tape, look-up my [page](cassette.md) on cassette tape operations.

##  Using the COINcidence instruction

The COIN instruction checks for coincidence between two objects on
screen. These objects can be anything, including sprites, characters, or
even exist only in the programmer's mind! The instruction returns with
cnd bit set if coincidence was detected, and reset if it wasn't.

The source argument must contain the screen coordinates of object 1, as
2 bytes (pixel row, pixel column). Similarly, the destination argument
should contain the coordinates of object 2.

The COIN instruction must be followed by some data:

1 byte indicating the grain of the checking (mapping value)

2 bytes that contain the address of a coincidence table.

The header of this table contains indications on the size of the
considered objects. The interpreter uses them, together with objects
coordinates, to quickly decide whether objects are in contact. If that's
the case, the remainder of the table is used to determine if there is
coincidence or not.

This way, the definition of 'coincidence' is up to the user.

The draw back is that you must supply coincidence table for each pair of
objects that you intend to check.

If you find difficult to generate such tables, you may use the small
extended Basic program included in my GPL package: it's awfully slow,
and limited to 8x8 pixels objects but that's all I could come with in
one week-end. Please feel free to improve it, for instance by writing a
GPL or 9900 version of it...

###  Building a coincidence table

In a coincidence table, you consider all possible positions in which the
two objects could touch or overlap each other, and for each position you
tell the interpreter if you regard it as coincident or not.

For instance, let's consider the following two objects:

     ++    is a 2x3 characters object,     xx    is a 3x3 chars object
    +++                                    xxx
                                           xxx

Note: the size to consider is the size of a putative rectangle
circumscribing the object. For instance, this object is 2 by 4:

     o
    oooo

There are 42 positions in which the 2 objects touch/overlap, let's view
them in order, from top-left touching to bottom-right touching:

     ++       ++      ++      ++      ++      ++      ++
    +++      +++     +++     +++     +++     +++     +++
       xx      xx     xx     xx     xx     xx     xx
       xxx     xxx    xxx    xxx    xxx    xxx    xxx
       xxx     xxx    xxx    xxx    xxx    xxx    xxx

     ++       ++      ++      ++      ++      ++      ++
    +++xx    ++*x    +**     **+    x*++   xx+++  xx +++
       xxx     xxx    xxx    xxx    xxx    xxx    xxx
       xxx     xxx    xxx    xxx    xxx    xxx    xxx

     ++xx     +*x     **      x*+   xx++   xx ++  xx  ++
    +++xxx   ++*xx   +**x    ***    x**+   xx*++  xxx+++
       xxx     xxx    xxx    xxx    xxx    xxx    xxx

       xx      xx     xx     xx     xx     xx     xx
     ++xxx    +*xx    **x    x**    xx*+   xxx++  xxx ++
    +++xxx   ++*xx   +**x    ***    x**+   xx*++  xxx+++

       xx      xx     xx     xx     xx     xx     xx
       xxx     xxx    xxx    xxx    xxx    xxx    xxx
     ++xxx    +*xx    **x    x**    xx*+   xxx++  xxx ++
    +++      +++     +++     +++     +++     +++     +++

       xx      xx     xx     xx     xx     xx     xx
       xxx     xxx    xxx    xxx    xxx    xxx    xxx
       xxx     xxx    xxx    xxx    xxx    xxx    xxx
     ++       ++      ++      ++      ++      ++      ++
    +++      +++     +++     +++     +++     +++     +++

For each of these you should provide the interpreter with one bit
telling if there is coincidence (bit=1) or not (bit=0).

Let's arbitrarly decide there is coincidence only when 2 characters are
overlapping (represented by a \*). We get the following matrix:

    0 0 0 0 0 0 0
    0 1 1 1 1 0 0
    0 1 1 1 1 1 0
    0 1 1 1 1 1 0
    0 1 1 1 1 0 0
    0 0 0 0 0 0 0

Now lets write all these bits as bytes:

    00000000 11110001 11110011 11100111 10000000 00xxxxxx (6 padding bits)
      >00      >F1      >F3      >E7      >80      >3F    (let's pad with 1s)

We've just compressed the above table in 6 bytes!

To get a complete coincidence table, we need four more bytes, that
serves as a header. These are:

Height of object 1 plus height of object 2 (here: 3+2 = 5)

Width of object 2 plus width of object 2 (here: 3+3 = 6)

Height of object 1 (here 2)

Width of object 1 (here 3)

The final table thus looks so:

    TABLE1 BYTE  5,6,2,3
           BYTE  >00,>F1,>F3,>E7,>80,>3F

Great, but recall this is only valid for coincidence between object 1
and object 2, in this very order. Should you decide to check for
coincidence between object 2 and object 1 (although I really don't see
why), then you would need another table.

If there are several objects of each kind, you also need different
tables to check for coincidence between objects 1 and 1, and ojects 2
and 2.

###  Changing the mapping value

The mapping value selects the "grain" of coincidence checking:

A value of 0 operates at pixel level,

A value of 1 considers "boxes" of 2 by 2 pixels,

A value of 2 uses 4x4 pixel boxes

Etc.

The advantage of a coarser grain is that it reduces the size of the
coincidence table, at the expense of a lower resolution.

To construct a table in mapping value 1, first construct the
"pixel-wise" table with mapping value 0. Then divide it in 2x2 bits
boxes, starting with the position where the upper left corner of the two
objects are superimposed.

Using the above example again, we have:

              V
    0   0 0   0 0   0 0
    0   1 1   1 1   0 0

    0   1 1   1 1   1 0   <-- upper-left corners superimposed
    0   1 1   1 1   1 0

    0   1 1   1 1   0 0
    0   0 0   0 0   0 0

Now reduce each box to a single bit, by taking the predominant value in
each box. If there is a tie, one can choose to consider it as 0 or 1
(for the purpose of this example, let's decide for 1).

The reduced table looks like this:

    0    1    1    0
    0    1    1    1
    0    1    1    0

Which gives us \>67 \>6F, when padding with four 1s.

For a mapping value of 2, one would proceed similarly, by creating 4x4
boxes.

Now, did you get that? I hope so, given the time it took me to put it
down!

##  Using sprites

### Sprites size and magnification

The size and magnification of sprites are determined by 2 flag bits in
the VDP register 1:

|     |      |      |      |       |      |      |      |
|-----|------|------|------|-------|------|------|------|
| Mem | Scrn | Ints | Text | Multi | n.u. | Size | Mag. |

**Bit 0** Memory size \>00 = 4K, \>80 = 16K (default).
**Bit 1** Screen \>00 = off, \>40 = on
**Bit 2** Interrupts \>00 = disabled, \>20 = enabled
**Bit 3** Text mode \>00 = off, \>10 = on
**Bit 4** Multicolor \>00 = off, \>80 = on
**Bit 5** not used \>00
**Bit 6** Sprites size \>00 = 1 pixel, \>02 = 2x2 pixels
**Bit 7** Magnification \>00 = 1 char, \>01 = 4 characters per sprite

Notes:

- Sprites are not allowed in text mode (40 columns, 6 pixels-wide
  characters). Neither are colors: colors of text and background is
  stored in VDP register 7.
- Interrupts should be left on for proper sprite motion and sound
  processing
- Memory size should be left as 16K (otherwise any address above \>1000
  will wrap around (i.e. \>2345 is understood as \>0345)
- When setting vdp register 1 (with the MOVE instruction) a copy is
  automatically made at \>83D4 (VDPR1), it's often convenient to recover
  that byte to modify vdp register 1, since all VDP registers are
  write-only. The keyboard scanning routine (GPL opcode SCAN)
  automatically reloads register 1 from that byte when a key is
  detected.

###  Sprite attributes

For each sprite, in numerical order, a 4 bytes definition is to be
loaded in the sprite attribute table, whose location is defined by VDP
register 5 (address = register content \* \>0080). The usual address is
\>0300, it MUST be used if auto-motion is desired.

Each sprite entry, in numerical order, consists in:

|           |              |         |             |
|-----------|--------------|---------|-------------|
| Pixel row | Pixel column | Pattern | Ck \| Color |

Pixel row values: top of screen, \>FF,\>00,\>01...\>BF, bottom (after
that: invisible. \>D0: last sprite).
Pixel column values left: \>00....\>FF right
Pattern values, refer to entry in pattern table: \>00 to \>FF
Clock bit. \>0: normal, \>8: shift by 16 pixels
Color values: \>0 to \>F

Notes:

- The sprite pattern table doesn't need to be the same as the character
  pattern table. By entering a different value in VDP register 6 (addr =
  value \* \>0800) one can have 256 definitions, located say at \>1000,
  and that have nothing in common with character pats (located at \>0800
  if VDP reg 4 contains 1).
- You probably noted that a sprite can "vanish" at the right of the
  screen, being only partly visible, whereas this is impossible at the
  left: at position \>00 all the sprite is onscreen, at \>FF it jumps to
  the right.
  The way around this is to set the clock bit, i.e. add \>80 to sprite
  color: this results in shifting the sprite reference point from the
  top-left corner to 16 pixels to the right (i.e. top-right corner of a
  dobble size, dobble magnification sprite). Of course sprite position
  must be adjusted (by adding 16 to column value), but now the sprite
  can vanish on the left.



- Sprites can vanish at the top (row values \>F0-FF) and at the bottom
  (row value \>B0-BF) without any problem because there are only 192
  pixel rows.

###  Automatic motion

The interrupt routine in charge of moving sprites expects a sprite
motion table located at \>0780 in VDP memory, and the sprite attributes
at \>0300 (VDP reg 5 = \>06)

Srite entries are arranged in numerical order, statting with sprite \#0.
Each entry consists in 4 bytes:

|            |             |            |            |
|------------|-------------|------------|------------|
| Vert speed | Horiz speed | ISR buffer | ISR buffer |

Vertical motion values: -128 (up) to +127 (down)
Horizontal motion values: -128 (left) to +127 (right)
ISR buffer: don't bother. Used by the ISR to store the current counter
value.

The speed is defined as pixels per 16/60 seconds (16/50 in Europe),
positive values move sprites to the right/bottom, negatives to the
left/top.

To enable auto-motion, after properly setting the table, you should:

Store the number of the last sprite to move into byte \>837A (SPRNO)

Make sure the "ignore all" and "ignore sprites" bits of the flag byte
\>83C2 (SYSFL) are not set. These are bit 0 (\>80) and 1 (\>40), they
can be cleared with: `AND >3F,@>83C2.` (For your information, the next 2
bits (\>20 and \>10) disable auto-sound processing and \ key
detection, respectively).

###  VDP registers summary

**VDP register 0:** Flags for bit mode (\>02) and external video as
background (\>01).
**VDP register 1:** Various flags (see above).
**VDP register 2:** Screen image table. Address = value \* \>0400. Size
\>0300 (\>0400 in text mode).
**VDP register 3:** Color table. Address = value \* \>0040. Size \>0040:
1 byte per 8 chars.
**VDP register 4:** Char patterns table. Address = value \* \>0800. Size
\>0800: 8 bytes per char.
**VDP register 5:** Sprite attributes. Address = value \* \>0080. Size
\>0080: 4 bytes per sprite.
**VDP register 6:** Sprite patterns. Address = value \* \>0800. Size
\>0800: 8 bytes per sprite.
**VDP register** 7: Text color \| screen color+ text background (in text
mode).

####  VDP status register

The VDP status read-only register is copied at \>837B by the interrupt
routine. This reading operation resets its interrupt flag bit. It
contains the following flag bits:

|       |       |      |                               |
|-------|-------|------|-------------------------------|
| Inter | 5 spr | Coin | Number of 5th sprite on a row |

**Bit 0** Interrupt: \>00 = none, \>80 = issued (vertical retrace
ongoing).
**Bit 1** 5 sprites: \>00 = ok, \>40 = more than 4 sprites on a pixel
row.
**Bit 2** Coincidence: \>00 = none, \>20 = at least two sprites have
overlapping "on" pixels.
**Bit 3-7** 5th sprite: the number of the 5th sprite on the first row
where that problem was encountered..

##  Usefull addresses

These addresses belong to 2 categories::

System constants

Scratch-pad addresses

GROM subroutines

For details on how to use most of the GROM routines, refer to the GPL
manual written by the 9T9 users group of Toronto. Their address is
(was?):

109-2356 Gerrard St. East,
Toronto M4E-2E2,
Canada.

I have this manual and could put it online, but I'm not sure they'd be
happy about it...

####  System constants

|          |         |                               |
|----------|---------|-------------------------------|
| Mnemonic | Address | Use                           |
| SOUND    | \>8400  | Sound chip port               |
| .        | \>8800  | VDP read data port            |
| .        | \>8802  | VDP read status port          |
| .        | \>8C00  | VDP write data port           |
| .        | \>8C02  | VDP write address port        |
| SPCHRD   | \>9000  | Speech synthesizer read port  |
| SPCHWT   | \>9400  | Speech synthesizer write port |
| .        | \>9800  | GROM read data port (#1)      |
| .        | \>9802  | GROM read address port        |
| .        | \>9C00  | GRAM write data port          |
| .        | \>9C02  | GROM write address port       |

####  Scratch-pad addresses

The memory area located at \>8300 to \>83FF is known as the scratch-pad.
This is the only (16 bits) cpu RAM available when no memory expansion is
connected.

The first part is used by TI-Basic and (in some cases) by Extended
Basic. If your program does not require any of these languages, these
bytes are free for you to use.


3=1 Trace, 4=1 Edit mode, 5=1 On warning stop,
6=1 On warning next, 2 + 7 unused.
to convert a line of text into Basic tokens).
The second part of the scratch-pad memory
is more general and deals with GPL rather than Basic. My GPL assembler
defines mnemonics for almost all bytes but not all of them are generally
accepted.


Also: sign of a real number
Also: exponent of a real number
Basic)
Quit key
mem
&gt;02 multicolor mode, &gt;01 sound table in VDP mem

####  GROM subroutines

Only GROMs 0 and 1 have a standard header, GROM 2 is the continuation of
GROM 1 and has no header.

At the beginning of GROM 0 is a table of subprograms, that contains only
branching instructions (mostly BR, and a few B). The location of these
subprograms may vary among GROM versions, but the table is always the
same.

For each of these subroutines, there is a pre-defined symbol in my
assembler. No guaranty that other assemblers will use the same name...

N.B. FAC (floating point accumulator) corresponds to \>834A-8351, ARG
(argument) to \>835C-8363

|  |  |  |
|----|----|----|
| Mnemonic | Address | Use |
| LINK | [\>0010](#g@0010) | Subroutine/DSR call: FETCh \>0A/\>08, name ptr in \>8356 |
| RETURN | [\>0012](#g@0012) | Return from a subroutine/DSR |
| CNS | [\>0014](#g@0014) | Convert number to string (from FAC to FAC, infos in \>8355-57) |
| STCASE | [\>0016](#g@0016) | Load title screen character patterns at VDP address in \>834A |
| UPCASE | [\>0018](#g@0018) | Load upper case character patterns at VDP address in \>834A |
| BWARN | [\>001A](#g@001a) | Issue warning message |
| BERR | [\>001C](#g@001c) | Issue error message |
| BEXEC | [\>001E](#g@001e) | Begins Basic excution: FETCh 4 bytes: address of first and last lines |
| PWRUP | [\>0020](#g@0020) | System reset |
| INT | [\>0022](#g@0022) | Convert real to integer (from FAC to FAC) |
| PWR | [\>0024](#g@0024) | Power-of-ten routine FAC=FAC \* 10^ARG |
| SQR | [\>0026](#g@0026) | Square root routine FAC=SQR(FAC) |
| EXP | [\>0028](#g@0028) | Exponentiation routine FAC=e^FAC |
| LOG | [\>002A](#g@002a) | Logarithm calculation FAC=ln(FAC) |
| COS | [\>002C](#g@002C) | Cosine calculation FAC=cos(FAC) |
| SIN | [\>002E](#g@002e) | Sine calculation FAC=sin(FAC) |
| TAN | [\>0030](#g@0030) | Tangent calculation FAC=tgn(FAC) |
| ATN | [\>0032](#g@0032) | Arctangent calculation FAC=atn(FAC) |
| BEEP | [\>0034](#g@0034) | Issue acceptation sound |
| HONK | [\>0036](#g@0036) | Issue error sound |
| GETSPA | [\>0038](#g@0038) | Get VDP memory space for a string |
| BITREV | [\>003B](#g@003b) | Bit reversal routine \>834A: VDP address, \>834C: number of bytes |
| NAMLNK | [\>003D](#g@003d) | Part of LINK: searches in GROM only |
| PABSPA | [\>003F](#g@003f) | Check memory space for PAB |
| TOKEN | [\>0042](#g@0042) | Get next token, set Basic pointers |
| LOCASE | [\>004A](#g@004A) | Load lower case character patterns |
The address of the following routines may vary, since they are not
included in a vector (branch) table. It's therefore risk to call them
directly. But it's up to you...


NUMBER, OLD, RES,SAVE and EXIT.
0-9..).
Finally, here are some usefull data in GROM.
The exact addresses may also vary according to GROM versions:

|         |                                                            |
|---------|------------------------------------------------------------|
| Address | Contents                                                   |
| \>0451  | Default values of the 8 VDP registers.                     |
| \>0459  | Content of the color table, for title screen.              |
| \>04B4  | Characters 32 to 95 patterns, for title screen.            |
| \>06B4  | Regular upper case character patterns.                     |
| \>0874  | Lower case character patterns.                             |
| \>16E0  | Joysticks codes returned by SCAN.                          |
| \>1700  | Key codes returned by SCAN.                                |
| \>1730  | Ditto with SHIFT.                                          |
| \>1760  | Ditto with FCTN.                                           |
| \>1790  | Ditto with CTRL.                                           |
| \>17C0  | Key codes in keyboard modes 1 et 2 (half-keyboards).       |
| \>2022  | Error messages (with Basic bias of \>60, and lenght byte). |
| \>285C  | Reserved words in Basic, and corresponding tokens.         |

### Structure of a Header

A standard grom header may be found at locations \>0000, \>2000, \>4000,
\>6000, \>8000, \>A000, \>C000 and \>E000. Its structure is the
following:

Content

&gt;AA indicates a standard header

Version number

Number of programs (optional)

Not used

Pointer to power-up list (&gt;0000 if
none)
Pointer to program list (&gt;0000 if
none)
Pointer the DSR list (&gt;0000 if none)
Pointer to subprogram list (&gt;0000 if
none)

Structure of a list:

    Link to next item --+
    Address             |
    Name length         |
    Name                |
    +-------------------+
    |
    V
    Link to next item: >0000
    Address
    Name length
    Name

    N.B. Name lengh and name are not necessary for power-up routines

If you would like more details on standard headers and how to write
subprograms and/or DSRs, I have a whole [page](headers.md) on the
subject.

------------------------------------------------------------------------

## XML routines in ROM

#### XML \>01 ROUND1

Use : Rounds real number to 14 digits

Input :
\>834A real number to round (9 bytes!!!)
\>8375 sign
\>8376 exponent

Output:
\>834A is rounded to 8 bytes

Status: reflects the value in \>834A

Errors: \>8354 \>01 if overflow

####  XML \>02 ROUND

Use : Rounds real number with specified number of bytes.

Input :
\>834A real number to round
\>8375 sign
\>8376 exponent
\>8354 integer: number of bytes in this real number

Output:
\>834A rounded to one byte less

Status: reflects the value in \>834A

Errors: \>8354 \>01 if overflow

####  XML \>03 STST

Use : Store status after comparing number to zero.

Input : \>834A real number / interger

Output: -

Status: reflects the value in \>834A

####  XML \>04 OVEXP

Use : Reacts to overflow/underflow (according to sign of \>8376).

Input :
\>8376 exponent
\>8375 sign

Output: \>834A 0 if underflow, min/max number (sign in \>8375) if
overflow.

Status: reflects the value in \>834A

Errors: \>8354 \>01

####  XML \>05 OV

Use : Reacts to overflow

Input : \>8375 sign

Output: \>834A min number / max number (according to sign in \>8375).

Status: reflects the value in \>834A

Errors: \>8354 \>01.

####  XML \>06 FADD

Use : Floating point addition

Input :
\>834A real number
\>835C real number

Output:
\>834A result of \>835C + \>834A
\>8376 exponent
\>8375 sign (\>00 = positive, \>FF = negative)

Status: reflects the result in \>834A

Errors: \>8354 \>01 if overflow

####  XML \>07 FSUB

Use : Floating point substraction

Input :
\>834A real number to substract
\>835C real number

Output:
\>834A result of \>835C - \>834A
\>8376 exponent
\>8375 sign

Status: reflects the result in \>834A

Errors: \>8354 \>01 if overflow

####  XML \>08 FMUL

Use : Floating point multiplication

Input :
\>834A real number
\>835C real number

Output:
\>834A result of \>835C \* \>834A
\>8376 exponent
\>8375 sign

Status: reflects the result in \>834A

Errors: \>8354 \>01 if overflow

####  XML \>09 FDIV

Use : Floating point division

Input :
\>834A real number to divide
\>835C real number divisor

Output:
\>834A result of \>834A / \>835C
\>8376 exponent
\>8375 sign

Status: reflects the result in \>834A

Errors: \>8354 \>01 if overflow

####  XML \>0A FCOMP

Use : Floating point comparison

Input :
\>834A real number
\>835C real number

Output: -

Status: results of the comparison of \>835C to \>834A

####  XML \>0B SADD

Use : Addition in stack

Input :
\>834A real number
\>836E points at real number on vdp stack

Output:
\>834A result of \>834A + V\*\>836E
\>836E decremented by 8 (points at previous number)
\>8376 exponent
\>8375 sign

Status: reflects the result in \>834A

Errors: \>8354 \>01 if overflow

####  XML \>0C SSUB

Use : Substraction in stack

Input :
\>834A real number to substract
\>836E points at real number on vdp stack

Output:
\>834A result of V\*\>836E - \>834A
\>836E decremented by 8 (points at previous number)
\>8376 exponent
\>8375 sign

Status: reflects the result in \>834A

Errors: \>8354 \>01 if overflow

####  XML \>0D SMUL

Use : Multiplication in stack

Input :
\>834A real number
\>836E points at real number on vdp stack

Output:
\>834A result of \>834A \* V\*\>836E
\>836E decremented by 8 (points at previous number)
\>8376 exponent
\>8375 sign

Status: reflects the result in \>834A

Errors: \>8354 \>01 if overflow

####  XML \>0E SDIV

Use : Division in stack

Input :
\>834A real number to divide
\>836E points at real number divisor on vdp stack

Output:
\>834A result of \>834A / V\*\>836E
\>836E decremented by 8 (points at previous number)
\>8376 exponent
\>8375 sign

Status: reflects the result in \>834A

Errors: \>8354 \>01 if overflow

####  XML \>0F SCOMP

Use : Comparison in stack

Input :
\>834A real number
\>836E points at real number on vdp stack

Output:
\>836E decremented by 8 (points at previous number)

Status: result of the comparison of V\*\>836E to \>834A

####  XML \>10 CSN

Use : Converts string to real number (and rounds it if needed)

Input :
\>8356 points at string in vdp mem

Output:
\>834A real number
\>8356 points at first illegal character
\>8376 exponent
\>8375 sign (\>00 = positive, \>FF = negative)

Status: reflects the result in \>834A

Errors: \>8354 \>01 if overflow

####  XML \>11 CSNGR

Use : Converts string to real number (and rounds it if needed)

Input :
\>8356 points at string in vdp mem or gram/grom
\>8389 memory flag: 0 = vdp, else = g mem

Output:
\>834A real number
\>8356 points at first illegal character
\>8376 exponent
\>8375 sign (\>00 = positive, \>FF = negative)

Status: reflects the result in \>834A

Errors: \>8354 \>01 if overflow

####  XML \>12 CFI

Use : Converts floating point number to integer

Input :
\>834A real number to convert (-32768 to 32767)

Output:
\>834A integer (2 bytes long)

Status: not affected

Errors: \>8354 \>01 if overflow

####  XML \>13 SYM

Use : Searches a symbol by name, from a statement (Basic)

Input :
\>832C points at symbol name in basic statement
\>833E points at basic variables symbol table (in vdp mem)
\>8389 0 = basic statement in vdp mem, else = in grom/gram

Output:
\>834A points at symbol entry, in the symbol table
\>832C points at next character
\>8342 contains this character

Errors: \>8322 contains \>0003 if not found, returns to GPL at GROM
address saved in \>8326

####  XML \>14 SMB

Use : Get symbol value

Input :
\>834A points at symbol entry, in the symbol table
\>8343 option base (in case symbol is part of an array)

Output:
\>834A Variable description
\>8350 value / string content pointer

Errors: \>8322 may contain \>0003 or \>0503.

####  XML \>15 ASSIGNV

Use : Assigns value to a symbol (Basic)

Input :
\>836E points at variable description in vdp stack
\>834A new description/value

Output:
\>836E decremented by 8 (points at previous number)
The variable is modified, in symbol table

Errors: \>8322 may contain \>0603 (\$-# mismatch), or \>0007

####  XML \>16 SCHSYM

Use : Search a given symbol by name (Basic)

Input :
\>834A variable name
\>8359 name length
\>833E points at basic variables symbol table (in vdp mem)
\>8389 0 = basic statement in vdp mem, else = in grom/gram

Output:
\>834A pointer to variable entry in symbol table

Status: Cnd reset, if found

Errors: Cnd set, if not found

####  XML \>17 VPUSH

Use : Pushes data (symbol description) on vdp stack (Basic)

Input :
\>834A data to push on stack (8 bytes)
\>836E vdp stack pointer (increment before use)
\>831A first free address (i.e. end of stack)

Output:
\>836E decremented by 8 (points at previous number)

Errors: \>8322 Contains \>0103 if not enough room left

####  XML \>18 VPOP

Use : Pops data (symbol description) from vdp stack (Basic)

Input :
\>836E vdp stack pointer (data to pop)
\>8324 points at bottom of stack

Output:
\>834A contains the data popped
\>836E decremented by 8 (points at previous number)

Errors: \>8322 Contains \>0403 if stack is empty

####  XML \>19 SROM

Use : Calls a ROM subroutine or a dsr by name, then RTN.

Input :
\>834A subroutine/dsr name
\>8354 name length
\>836D Type: \>08 = dsr, \>0A = subroutine
\>83D0 Optional: CRU to use. \>0000 scans all cards
\>83D2 If CRU was specified: next link address

Output:
\>83D0 CRU of the card the sub was in
\>83D2 Next link address

Errors: Does not perform RTN if the name wasn't found.

####  XML \>1A SGROM

Use : Finds a GROM subroutine or dsr by name, puts its address on data
stack.

Input :
\>834A subroutine/dsr name
\>8354 name length
\>836D Type: \>06 = program, \>08 = dsr, \>0A = subroutine
\>83D0 Grom base (if 0: use \>9800, and search Grom 0)
\>83D2 Grom address of header list.

Output:
\>83D0 Grom base the name was found in
\>83D2 Grom address the name was found at
\>836C Number of occurences the name was found

\>8372 incremented by 2 (for each occurence)

Errors: \>83D0 Contains 0 if name wasn't found

####  XML \>1B PGMCH

Use : Gets next character in Basic program.

Input :
\>832C points at current character, in Basic statement
\>8389 mem flag: 0 = program in vdp, else = in grom

Output:
\>832C points at next character
\>8342 contains the required character

------------------------------------------------------------------------

## GPL routines in GROMs

####  G@\>0010 LINK

Use : Calls DSR and subroutines in GROMs or cards ROM

Input :
1 byteFETCched: \>08 = dsr, \>0A = subroutine
\>8356 points at name string in VDP ram (i.e. PAB+9 for DSRs)

Status: Cnd reset

Errors: Cnd set if not found, returns to caller.

Used: \>8352 to \>8358.
Base + return address saved on subroutines stack.

Calls XML \>19, then XML \>1A

####  G@\>0012 RETURN

Use : Returns from a G@\>0010 call

Status: Cnd reset

Used: Base + address recovered from suroutines stack

####  G@\>0014 CNS

Use : Converts real number into a string

Input :
\>834A real number
\>8355 string type: 0 = scientific notation, else: number of digits
\>8356 number of significant digits in case of overflow
\>8357 positive: fixed digits right to the point, negative: float

Output:
\>8355 scratch-pad address of string (i.e. \>83xx)
\>8356 string length

Status: reflects the value in \>834A

Errors: Cnd set

Used: \>834A is modified
\>8352
\>8358-9
\>8376-7

####  G@\>0016 STCASE

Use : Loads standard (i.e. title) upper case characters \>20 to \>5F

Input :
\>834A: vdp address where to load patterns

Status: Cnd reset

####  G@\>0018 UPCASE

Use : Loads upper case characters \>20 to \>5F

Input :
\>834A word: vdp address where to load patterns

Status: Cnd reset

Used : \>83D0 to \>83D2

####  G@\>001A BWARN

Use : Issues a Basic "Warning" message (Basic)

Input :
2 bytes FETChed: message string pointer

Output:
\>8320 cursor position = end of message

Used : \>835C-F
\>8374
\>8376-7

By run time: \>8363-8367

Notes : Calls HONK

####  G@\>001C BERR

Use : Issues a Basic "Error" message (Basic)

Input :
2 bytes FETChed: message string pointer
\>8304 for i/o errors: points 4 byte above opcode, in PAB (in VDP mem)

Used : \>835C-F
\>8374
\>8376-7

If I/O error: \>8361, \>8366-7

By run time: re-loads characters, close files, XML \>18, etc.

Notes : Calls BWARN

####  G@\>001E BEXEC

Use : Begin execution of a Basic program in gram/grom (Basic)

Input :
4 bytes FETChed: addresses of beginning and end of line \# table

Used : many !

####  G@\>0020 PWRUP

Use : Restarts the computer (equivalent to "quit" key)

Input : -

Used : everything !

####  G@\>0022 INT

Use : Converts a floating point number to the greatest integer

Input :
\>834A real number

Output:
\>834A greatest integer contained in the real value

Status: reflects the result in \>834A

Used :\>8352-7
\>8375-7

Notes : calls XML \>02

####  G@\>0024 PWR

Use : Raises a number to a power

Input :
\>834A floating point power
\>835C real number to raise to power \>834A

Output:
\>834A result: \>835C ^ \>834A

Status: reflects the value in \>834A

Errors: \>8354 floating point error code

Used : \>8354, \>8375-7, VDP stack

Notes : May call XML \>02, \>04, \>06, \>08, \>09, \>0A, \>0B, \>0C,
\>0D, \>0E, \>0F, \>12
May call EXP and INT

####  G@\>0026 SQR

Use : Calculates the square root of a number

Input :
\>834A real number

Output:
\>834A result: sqr(\>834A)

Status: reflects the value in \>834A

Errors: \>8354 \>04 if \>834A is negative

Used : \>8352, \>8375-6

Notes : May call XML \>03, \>06, \>08, \>09, \>0B, \>0E

####  G@\>0028 EXP

Use : Calculates "e" at the power of a number (inverse natural log)

Input :
\>834A floating point power

Output:
\>834A result: e^ \>834A

Status: reflects the result in \>834A

Errors: \>8354 \>01 if overflow

Used : \>8352-7, \>8375-7, VDP stack

Notes : May call XML \>04, \>06, \>08, \>09, \>0A, \>0C, \>0D, \>12
May call INT

####  G@\>002A LOG

Use : Calculates natural log of a number (i.e base "e")

Input :
\>834A real number

Output:
\>834A result: ln(\>834A)

Status: reflects the result in \>834A

Errors: \>8354 \>06 if \>834A = 0, or negative

Used : \>8376-7

Notes : May call XML \>03, \>06, \>07, \>08, \>0B, \>0D, \>0E

####  G@\>002C COS

Use : Calculates cosine of an angle

Input :
\>834A real number: angle in radians

Output:
\>834A result: cos(\>834A)

Status: reflects the result in \>834A

Errors: \>8354 \>07 if \>834A is invalid

Notes : May call XML \>06, \>07, \>08, \>0C, \>0D
Calls SIN, may call INT

####  G@\>002E SIN

Use : Calculates the sine of an angle

Input :
\>834A real number: angle in radians

Output:
\>834A result: sin(\>834A)

Status: reflects the result in \>834A

Errors: \>8354 \>07 if \>834A is invalid

Used : \>8375-7

Notes : May call XML \>06, \>07, \>08, \>0C, \>0D
May call INT

####  G@\>0030 TAN

Use : Calculates the tangent of an angle

Input :
\>834A real number: angle in radians

Output:
\>834A result: tan(\>834A)

Status: reflects the result in \>834A

Errors: \>8354 \>01 if overflow, \>07 if \>834A is invalid

Used : \>8375-7

Notes : May call XML \>03, \>05, \>06, \>07, \>08, \>09, \>0C, \>0D
Calls SIN and COS, may call INT

####  G@\>0032 ATN

Use : Calculates the arctangent of a number

Input :
\>834A real number

Output:
\>834A result: atn(\>834A), angle in radians

Status: reflects the result in \>834A

Errors: \>8354 floating point error code

Notes : May call XML \>06, \>09, \>0A, \>0D, \>0E

####  G@\>0034 BEEP

Use : Issues accept tone

Input : -

Output: -

Used : \>8358

####  G@\>0036 HONK

Use : Issues bad response tone

Input : -

Output: -

Used : \>8358

####  G@\>0038 GETSPA

Use : Get vdp space for string constants (Basic)

Input :
\>830C number of bytes required

Output:
\>831C ptr to the space allocated

Status: Cnd reset

Errors: may call BERR with "memory full" message

Used : \>8352-B

Notes : may call XML \>17, \>1B

####  G@\>003B BITREV

Use : Produces a mirror image of 1 or more bytes, in VDP mem

Input :
\>834A: VDP address of the byte(s) to revert
\>834C-D: number of bytes to process

Output:
The bits are reversed in VDP bytes

Used : \>8300-\>8340

Notes : Downloads a small machine language routine to \>8300 and
executes it.

####  G@\>003D NAMLNK

Use : Calls DSR/ subroutines in GROM/GRAM

Input :
\>834A device name
\>8354 DSR name size
\>8356 end of DSR name in VDP mem (in PAB)
\>836D \>08 (for DSR call)
\>83D0 optional: GROM base to use, 0 = use \>9800
\>83D2 if base specified: address of DSR header ("next link" word)

Output: -

Status: Cnd reset

Errors: Cnd set if not found

Notes : Part of G@\>0010, but calls XML \>1A only (no search in rom)

####  G@\>003F PABSPA

Use : Gets space for a new pab entry (Basic)

Input :
\>834A integer: number of bytes to reserve
\>8356 points at name string in VDP RAM (i.e. PAB+9)

Output: -

Status: Cnd reset

Errors: May call BERR with "memory full" message

Used : \>834A, \>8352-8

Notes : May call XML \>17, \>1B

####  G@\>0042 TOKEN

Use : Gets next Basic token/char from current statement (Basic)

Input :
\>8389 flag: 0 = from VDP mem, else = from GROM/GRAM
\>832C if in GROM/GRAM: pointer to next token
\>8356 if in VDP: pointer to next token/char

Output:
\>8342 token/char
\>832C updated pointer

Status: Cnd reset

Notes : calls XML \>1B

####  G@\>004A LOCASE

Use : Loads lower case characters \>60 to \>7F

Input :
\>834A word: VDP address where to load patterns

Output: -

Status: Cnd reset

Used : \>83D0-3

####  G@\>004C PATITL G@\>004E PATUP G@\>0050 PATLO

Use : Point at the patterns lists for standard chars (8 bytes/char),
upper or lower case characters (7 bytes/chars, the first being \>00).

Notes : absent in some GROM versions.
These are pointers to data, not to subroutines. Don't call them with
CALL !
[Previous](gpl.md)

Revision 1. 6/1/99 Preliminary
Revision 2. 6/13/99 OK to release
Revision 3. 3/5/00 Minor corrections
[Back to the TI-99/4A Tech Pages](titechpages.md)
