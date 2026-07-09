# The SuperAMS card

SuperAMS (Super Asgard Memory System, a.k.a. SAMS card) was one of the
first memory expansion card designed for the TI-99/4A. It was designed
around 1993 by Asgard Inc, under the managment of Jim Krych. Its elegant
simplicity is due to the fact that it's build around the 74LS612 memory
mapper, once commonly used in PCs and in Nintendo game cartridges. For
some reason, the card never got the popularity that it deserved: I was
told that only about 100 were produced. Originally, this may have been
due to the high cost of SRAM chips: the card was designed to accomodate
upto 1 megabyte, which was quite expensive in the early nineties.
Nowadays, the problem is rather with the 74LS612 that became extremely
hard to find: most PCs now emulate it in a VLSI chip.

**Hardware
The 74LS612 memory mapper  **
[Pinouts
](#74LS612)[Theory of operation
](#operating%20the%20612)[Timing diagrams
](#Timing%20diagrams)[Electrical charateristics
](#Electrical)**Card circuitery  **
[Power supply
](#Power)[Bus buffering
](#Bus%20buffers)[CRU logic
](#CRU%20logic)[Address decoding
](#Addressing)[Memory maping](#SRAM%20logic)

[AEMS: the ancestor
](#AEMS)[16 megs modification](#Multimega)

**Software  **
[Low-level routines
](#low-level)[High-level languages](#High-level)

#  Hardware

## The 74LS612 memory mapper

This very handy TTL chip serves to extend an address bus by 8 extra
bits. It does this by storing 12-bit values into 16 internal registers.
When mapping is enabled, four lines from the address bus serve to select
a register and the content of the register is transfered to 12 output
address lines. In effect, the 74LS612 expands 4 lines into 12, which
multiplies the address space by 256. For the TI-99/4A, this means that
the maximum memory becomes 16 megabytes.

To access all the extra memory, the CPU must load selected values into
the proper register. Typically, the address space will be divided in 16
banks, and 256 different "pages" can be selected to appear in these
banks. The size of a bank is that of the address space divided by 16 (8K
on the TI-99/4A).

Of course, the TI-99/4A does not support RAM in the whole address space,
only at \>2000-3FFF and \>A000-FFFF. This means that only 8 registers
will effectively be used.

The memory mapper comes in four flavors: the 74LS612 described here, the
74LS610 which has the ability to latch the address bus, the 74LS611 and
74LS613 which are open-collector versions of the above. This means that,
if you cannot find a 74LS612, you can go for any of the others. You just
need to make sure pin 28 will be pulled up to +5V with the '610 and
'611. If you're using an open-collector chip, you will need extra
pull-up resistors on the outputs MO0-MO11 (4.7 KOhms to +5V should be
ok).

### Pinout

            +----+--+----+
        RS2 |1 o       40| Vcc
        MA3 |2         39| MA2
        RS3 |3    7    38| RS1
        CS* |4    4    37| MA1
    STROBE* |5    L    36| RS0
       R/W* |6    S    35| MA0
         D0 |7    6    34| D11
         D1 |8    1    33| D10
         D2 |9    2    32| D9
         D3 |10        31| D8
         D4 |11        30| D7
         D5 |12        29| D6
        MM* |13        28| nc
        MO0 |14        27| MO11
        MO1 |15        26| MO10
        MO2 |16        25| MO9
        MO3 |17        24| MO8
        MO4 |18        23| MO7
        MO5 |19        22| MO6
        GND |20        21| ME*
            +------------+

Power supply
**Vcc**: +5 Volts
**Vss**: Ground

Mapper access
**CS\***: Chip select. When active (low) the CPU can access the
mappers registers via D0-D11.
**R/S\***: Direction: high to read from registers, low to write to
registers.
**STROBE\***: Write impulsion: stores D0-D11 into a mapper register
(CS\* and R/W\* must be low).
**RS0-RS3**: Register select. Address of the register to access.

Memory mapping
**MM\***: Map mode. When active (low) MO0-MO11 reflect the content
of the register selected by MA0-MA3. When high, MO8-MO11 reflect
MA0-MA3, and MO0-MO7 are low ("transparent" mode).
**MA0-MA3**: Map address. Four lines of the memory address bus that
selects a register in the mapper (MM\* low).
**MO0-MO11**: Map output. Twelve new address lines, corresponding to the
content of the register selected by MA0-MA3 when MM\* is low.
**ME\***: Map enable. A low level activates MO0-MO11. When high,
MO0-MO11 are in high impedance state (isolated).

**C**: Not connected on the 74LS612/74LS613. On the 74LS610/74LS611:
latch control. When high the content of the selected register is
transparently passed to MO0-MO11. A low level latches the outputs and
freezes MO0-MO11, even if the register (or its content) changes.

###  Operating the 74LS612

The 74LS612 supports four modes of operation: read, write, map and pass.


MO0-MO7=low

To load a value into a register, the CPU places the address of this
register on RS0-RS3, and sets CS\* and R/W\* low. A low pulse on
STROBE\* performs the write operation and causes the data present on
D0-D11 to be stored into the selected register.

The content of a register can be read back by setting CS\* low and
keeping R/W\* high. The data contained in the register selected by
RS0-RS3 will be placed on D0-D11.

To perform memory mapping, the CPU sets CS\* high and MM\* low. In this
configuration, the 74LS612 ignores the inputs RS0-RS3 and uses MA0-MA3
instead to select a register. The content of this register is presented
on MO0-MO11.

To disable memory mapping, the CPU can set both CS\* and MM\* high. In
this case, the 74LS612 is "transparent", i.e. forwards RA0-RA3 to the
MO8-MO11 pins. The other MO pins are set low.

Two additional control pins are provided: ME\* serves to place the
output pins MO0-MO11 in high impedance state, when the mapper should not
be in control of the address bus. On the 74LS610/611, the additional C
pin allows to freeze the outputs in a given state. This may be usefull
if the CPU needs to alter the register contents or to place "crazy"
values on MA0-MA3.

###  Timing diagrams

#### Write and read modes

    ______                                       _________
         |\_____________________________________/         \_____________________ CS*
         |   >20 ns    |         |      >20 ns  |         |
         |        |\   |         |     /|       |         |                      R/W*
         |        | >20|         | >20  |       |         |
         |        |    |\  >75  /|      |       |         |                      STROBE*
         |        ||>20|         |>20|  |       |         |
    XXXXXXXXXXXXXXXX  Address valid  X      Address    valid       X New address RS0-RS3
         |26-50|  |30-50|  | >75 |>20 | |20-35| |38-65 |  |26-50|  |39-75|
    -----------X output X--X   input  X-------X output X--------X output X------ D0-D11

####  Map and pass modes

        ________________________________________________________________
    ___/|                                                                CS*
    ____|__48-85 ns |
           |\       |                /|                                  MM*
           |22-40 ns|                 |
      \             |                 |                   /14-25|        ME*
      |             |                 |                         |
    XXXXXX   address valid  X   address valid    X  valid XXXXXXXXXXXXXX MA0-MA3
      |  | 39-70 ns |                 |          |              |
      |17-30|       |                 |22-40|    |13-30|        |
    --------X valid X   valid    X  valid   X  valid   X  valid X------- MO8-11
            |       |             |         |                   |
    --------X valid X   valid    X  valid   \                   ,------- MO0-7

           |          MAP             |        PASS

###  Electrical characteristics

#### Absolute maximum ratings

    Supply voltage: Vcc               7V
    Input voltage:  D0-D11          5.5V
                    All others        7V
    Free air temperature:     0 to 70 `C
    Storage temperature:   -65 to 150 `C

####   Recommended operating conditions


D0-D11
-2.6
D0-D11
8

####  Electrical characteristics under recommended conditions



D0-D11
Vcc = 4.75V I = max
2
2.4

D0-D11
Vcc = 4.75V I = 24 mA
Vcc = 4.75V I = 4 mA
Vcc = 4.75V I = 8 mA
0.35
0.25
0.35
0.5
0.4
0.5
DO0-DO11
-400
all other inputs
Vcc = 5.25V Vin = 7V
0.1
D0-D11
-30
-130
Outputs low
Outputs disabled
112
180
180
230

##   Card circuitery

Apart for the mapper and SRAM memory chips, the SuperAMS card contains
few additional circuitery.

#### Power supply

                   LM7805                          All chips
                +----------+      +5V              +-----+
+8V-----+-------|Vin   Vout|---+-----... ---+------|Vcc  |    
        |       |   Ref    |   |            |      |     |
        = 22 uF +----------+   = 0.1 uF     =0.1uF |     |
        |            |         |            |      |     |
       Gnd           Gnd       Gnd     Gnd--+------|Vss  |
                                                   +-----+

Nothing special here: a LM7805CT voltage regulator and two caps
(suggested values are mine: there are none on the shematics I got). In
addition, it's always a good idea to add a by-pass cap in front of each
chip on the card, to filter out transients.

####  Bus buffering

As recommended by Texas Instruments, all lines to/from the peripheral
bus are buffered on-card. This is done with three 74LS244 for the
address bus (16 bits) and some of the control lines: MEMEN\*, CRUCLK,
RESET\*, DBIN, and WE\*.

The data bus, of course needs a bidirectional 74LS245 buffer. The
DIRection is controlled by the DBIN signal from the peripheral bus, the
ENable pin by the CardSel\* signal generated by the card (see below).

The CardSel\* signal is also connected via a 74LS125 to the RDBENA\*
line of the peripheral bus to enable the drivers in the connection card
and the console-end of the cable. A 74LS125 is necessary, so as not to
hold the line high when we are not using it, as another card may need
it. The CardSel\* signal enables the '125 which input is connected to
the ground, the rest of the time the 74LS125 is in high-impedance (i.e.
isolated) state.

The following schematic combines the above buffering circuits and the
CRU logic described in the next chapter.

           +-----+                      +------+      
     A0&gt;---|     |---&gt;A0         A0&gt;---&gt;|A0  B0|--,      
      .    |     |    .          A1&gt;---&gt;|A1  B1|--+--Gnd 
      .    |     |    .          A2&gt;---&gt;|A2  B2|--&#39;
      .    |     |    .          A3&gt;---&gt;|A3  B3|--,
      .    |     |    .          A4&gt;---&gt;|A4  B4|--+
      .    |     |    .          A5&gt;---&gt;|A5  B5|--+--+5V
     A7&gt;---|     |---&gt;A7         A6&gt;---&gt;|A6  B6|--&#39;    
      Gnd--|EN*  |               A7&gt;---&gt;|A7  B7|--Gnd
           +-----+         ,------------|EN*   |
            &#39;244           |            |  A=B*|---,
           +-----+         |            +------+   |
     A8&gt;---|     |---&gt;A8   |                       |
      .    |     |    .    |                       |
      .    |     |    .    |        ,--------------&#39;
      .    |     |    .    |        |
      .    |     |    .    |        |   +------+  
      .    |     |    .    |        &#39;---|EN*   |  74LS04
     A15&gt;--|     |---&gt;A15  |    A12&gt;---&gt;|S2  Q0|---|&gt;o---Bit0*
      Gnd--|EN*  |         |    A13&gt;---&gt;|S1  Q1|---|&gt;o---Bit1*
           +-----+         |    A14&gt;---&gt;|S0  Q2|
            &#39;244           |            |    Q3|
           +-----+         |    A15&gt;---&gt;|D   Q4|
CRUCLK*&gt;---|     |---------&#39;            |    Q5|
 RESET*&gt;---|     |---------------------&gt;|RST*Q6|
    WE*&gt;---|     |---&gt;WE*               |    Q7| 
  MEMEN&gt;---|     |---&gt;MEMEN*            +------+  
CLKOUT*&gt;---|     |            
           |     |            
   DBIN&gt;---|     |---|&gt;o---+----DBIN*                        
      Gnd--|EN*  |         | 
           +-----+         |     
            &#39;245           |     
           +-----+         |     
           |  DIR|&lt;--------&#39;   
    D0&lt;---&gt;|A0 B0|&lt;--&gt; D0 
     .     |     |      .
     .     |     |      .   
     .     |     |      .
     .     |     |      .        
     .     |     |      .                           
     .     |     |      .                     
    D7&lt;---&gt;|A7 B7|&lt;--&gt; D7                      
           |  OE*|&lt;--,                           
           +-----+   |   ,----------&lt; CardSel*  
                     |   |                      
RDBENA*&lt;-------------+---&lt;|--Gnd
                       &#39;125  

####  CRU logic

The CRU logic is very basic. A 74LS688 comparator is used to compare the
CRU address to a hardwired value of \>1E00 (possible improvement on the
design: use a DIP switch on B4-B7). The output of the comparator enables
latching of the A15/CRUOUT bit by the 74LS259 addressable latch. Lines
A12-A14 are used to select a bit among eight. Note that no provision is
made to read back the status of the bits. Anyhow only two bits are used,
after due inversion by 74LS04 inverters.

####  Address decoding

       +--------+              
  A0---|S2   Y0*|
  A1---|S1   Y1*|------------,
  A2---|S0   Y2*|--------,   |
       |     Y3*| Bit0*--=)&gt;-------+------MapSel*
       |     Y4*|     74LS32 |     |
 +5V---|G1   Y5*|--,         |     ,=|)---CardSel*
   ,---|G2A* Y6*|--=|)-,     |     |
   +---|G2B* Y7*|------=|)---=|)---+------MemExp*
   |   |        |     74LS08
  Gnd  +--------+   
 

Addresses are decoded by a 74LS138 3-to-8 decoder that splits the
addressing space into eight \>2000-byte chunks. The active low outputs
Y1\*, Y5\*, Y6\* and Y7\* are combined with AND gates to produce an
active low signal, MemExp\*, upon accesses in the range \>2000-3FFF
(Y1\*) or \>A000-FFFF, i.e. the usual memory expansion space.

In addition, Y2 is combined with Bit0\* to access the memory mapper
registers in the address block \>4000-5FFF, when CRU bit 0 is set as 1.
Both this signal and MemExp\* are combined in a fourth AND gate, to
yield the CardSel\* signal that was discussed above.

####   SRAM mapping

                    +-----------+    ,--------------------------+----|&gt;o-----,
                ,---|D0      MO0|-  /         +----------+      |            |
          Gnd---+---|D1      M01|- o o  +5V   |        CE|---&lt;(=---,         |
                +---|D2      MO2|- | |    | +----------+ |    ,----|---------&#39;
                &#39;---|D3      MO3|- | +--o o |        CE|---&lt;(=-----+---MemExp*
               D0---|D4      MO4|--&#39; |   /  |          | |  74LS32
               D1---|D5      MO5|----|---|--|A18     D7| |---D0
               D2---|D6      MO6|----&#39;   &#39;--|A17     D6| |---D1
               D3---|D7      MO7|-----------|A16     D5| |---D2
               D4---|D8      MO8|-----------|A15     D4| |---D3
               D5---|D9      MO6|-----------|A14     D3| |---D4
               D6---|D10    MO10|-----------|A13     D2| |---D5
               D7---|D11    MO11|-----------|A12     D1| |---D6
                    |           |      A4---|A11     D0| |---D7
              A11---|RS0        |      A5---|A10       | |
              A12---|RS1        |      A6---|A9        | |
              A13---|RS2        |      A7---|A8        | |
              A14---|RS3        |      A8---|A7        | |
                    |           |      A9---|A6        | |
               A0---|MA0        |     A10---|A5        | |
               A1---|MA1        |     A11---|A4        | |
               A2---|MA2        |     A12---|A3        | |
               A3---|MA3        |     A13---|A2        | |
                    |           |     A14---|A1        | |
                   -|C          |     A15---|A0        | |
              WE*---|STB        |           |          | |
             DBIN---|R/W*       |     WE*---|R/W*      |-+
 MapSel*---+--------|CS*        |   DBIN*---|OE*       |
           &#39;---|&gt;o--|ME         |           +----------+
 Bit1*----,     ,---|MM         |          (2x) 512K SRAM
 MemExp*--=)&gt;---&#39;   +-----------+
        74LS32  

##### SRAM connections

The SRAM data pins D0-D7 are connected to the data bus lines D7-D0, in
reverse order because TI numbers the lines the opposite way to everyone
else. The address pins A0-A11 are connected to the address lines A4-A15,
providing an \>1000-byte (4K) address bank. The extra address pins
A12-A18 serve to select the RAM page that should appear in this bank.
They are controlled by the output pins of the 74LS612 memory mapper,
M5-M11.

In addition, two jumpers provide the possibility to use either one or
two 512 Kbyte or 128Kbyte SRAM chips. The MemExp\* signal that should
normally go directly to the CS\* pin of the SRAM is filtered through two
OR gates. One receives its input directly from the first jumper, the
other receives the inverse of this signal. The jumper serves to select
either MO4 or MO6 as the line that controls which SRAM chip is accessed.
The other jumper is connected to A17 on both chips and offers the choice
between MO6 or +5 volts: when using 128Kbytes chips (that have less
pins) the jumper should be set on +5 volts.

This gives us two possible configurations:

With 512K chips: MO6 serves as a chip selection line, MO4 is connected
to A17.

With 128K chips: MO4 serves as a chip selection line, A17 is held high.

Note that it is possible to install only one chip, so the card can have
four possible memory sizes: 128K, 256K, 512K or 1024K. Just be carefull
not to select a page that maps into the missing chip! There is no such
problem when using 128K instead of 512K chips because higher page
numbers just mirror lower pages, e.g. pages 64, 128 and 192 are the same
as page 0.

The R/W\* pin is connected to the WE\* line and the OE\* pin to the
inverse of DBIN (but could probably be grounded instead).

##### 74LS612 connections

Address lines A11-A14 are connected to the RS0-RS3 pins: they select one
of the 16 internal registers of the 74LS612 when the software accesses
the registers directly (address range \>4000-4020). Each register is 12
bits wide but only 8 bits are used: the most significant pins D4-D11 are
connected to the data bus lines D0-D7. The remaining pins D0-D4 are
grounded. This allows the software to load a value from 0 to 255 into
any of the registers.

When the 74LS612 is used as a mapper it chops the address space in 16
chunks of \>1000 bytes and provides a 10-bit value, taken from the
appropriate register, on MO0-MO11. Only the most significant 8 bits are
used and serve to select a SRAM page. Address lines A0-A3, connected to
the MA0-MA3 pins, select the appropriate register while in mapping mode.

Pin R/W\* is controlled by DBIN, pin STB by WE\*, and pin CS\* by the
MapSel\* signal (address \>4000-5000, with CRU bit 0 set to 1), while
the ME pin receives the inverse of the latter signal. Pin MM enables
memory mapping when an access occurs in the range \>2000-3FFF or
\>A000-FFFF and CRU bit 1 is set as '1'. This CRU bit can also be reset
as '0' to put the mapper in "transparent" mode. In this mode, \>2xxx
maps to page \>0002, \>3xxx maps to page \>0003, \>Axxx maps to page
\>000A, etc. Note that CRU bits are reset upon power-up, so this mode
will be the default. The software can then quietly load other values
into the registers, then set bit 1 to '1' and the new mapping will come
into effect.

###  AEMS: the ancestor

Before the SuperAMS, came AEMS (Asgard Extended Memory System, if I'm
not mistaken). It suffered from several limitations, or so I was told:

It could only be 128K or 512K (probably because it had only one memory
chip).

The area \>2000-3FFF could not be paged (?).

The most significant bit was permanently set to one, so page 0 would in
fact be page 128 (??).

But then again, I didn't see the schematics for this one...

###  Extension to 8 or 16 megabytes

A possible improvement to the design would be to make use of the extra
mapping lines M0 through M3. This could be done easily with a 74LS273
latch, controlled by A15 and WE\* so as to memorize the odd-address byte
upon any write operation (the odd byte is always written first). Four of
the input lines would be connected to the D4-D7 data bus lines, and the
corresponding outputs to the D0-D3 pins of the 74LS612. This way, page
numbers from 0 to 4096 can be stored into the registers. The output
MO0-MO4 could drive two 74LS154 decoders and select a SRAM chip among
32, bringing the total available memory to 16 megabytes, arranged as
4096 pages of 4 Kbytes each.

Alternatively, only one 74LS154 decoder could be used which would allow
only 16 chips, i.e. 8 megabytes. In this case, MO0 could control the
second enabling pin of the decoder. This would let us turn mapping off
on a per-bank basis. The big advantage of such a design would be to
enable mapping in the area \>4000-5FFF (card DSRs, we'll need another
CRU bit to select SRAM or 74LS612 in this area) and \>6000-7FFF
(cartridge banks and RAMBO banks). This cannot be done with the current
design because mapping can only be disabled globally, not for given
memory windows.

Here is a tentative schematic:

     MapSel*----,                |            +-------+
                |        ,-------+------------|G1* Y0*|----------,
CardSel*&lt;----(|=------&lt;(=-----------+---------|G2*  . |          |
            74LS08  74LS32          |         |     . |          |
A15---------,                       |         |     . |          |
WE*---|&gt;o---=|)--,                  | ,-------|S0   . |          |
   74LS04        |                  | | ,-----|S1   . |          |
      +-------+  |                  | | | ,---|S2   . |          |
      |    CLK|--&#39;       74LS612    | | | | ,-|S3 Y15*|--------, |
      |       |       +-----------+ | | | | | +-------+        | |
 D4---|D0   Q0|--|&gt;o--|D0      MO0|-&#39; | | | |                  | |
 D5---|D1   Q1|-------|D1      M01|---&#39; | | |    +----------+  | |
 D6---|D2   Q2|-------|D2      MO2|-----&#39; | |    |       CS*|--&#39; |
 D7---|D3   Q3|-------|D3      MO3|-------&#39; |  +----------+ |    |
      |       |  D0---|D4      MO4|---------&#39;  |       CS*|------&#39;
+5V---|RST*   |  D1---|D5      MO5|------------|A18       | |
      +-------+  D2---|D6      MO6|------------|A17       | |
       74LS273   D3---|D7      MO7|------------|A16     D7| |---D0
                 D4---|D8      MO8|------------|A15     D6| |---D1
                 D5---|D9      MO6|------------|A14     D5| |---D2
                 D6---|D10    MO10|------------|A13     D4| |---D3
                 D7---|D11    MO11|------------|A12     D3| |---D4
                      |           |       A4---|A11     D2| |---D5
                A11---|RS0        |       A5---|A10     D1| |---D6
                A12---|RS1        |       A6---|A9      D0| |---D7
                A13---|RS2        |       A7---|A8        | |
                A14---|RS3        |       A8---|A7        | |
                      |           |       A9---|A6        | |
                 A0---|MA0        |      A10---|A5        | |
                 A1---|MA1        |      A11---|A4        | |
                 A2---|MA2        |      A12---|A3        | |
                 A3---|MA3        |      A13---|A2        | |
                      |           |      A14---|A1        | |
                     -|C          |      A15---|A0        | |
                WE*---|STB        |            |          | |
               DBIN---|R/W*       |      WE*---|R/W*      |-+
 MapSel*-----+--------|CS*        |    DBIN*---|OE*       |
             &#39;---|&gt;o--|ME         |            +----------+
 Bit1*-----,      ,---|MM         |          (16x) 512K SRAM
 MemExp*---=)&gt;----&#39;   +-----------+  

The 16 megs version is identical except that there will be two 74LS154,
both with their G1\* input connected to MemExp\*. One of them should
have its G2\* input connected directly to MO0 and the other should
receive MO0 inverted by a 74LS05 on its G2\* input. Obviously, each
decoder will drive a different set of 16 SRAM chips.

Finally, here is a possible modification of the address decoder that
only requires one extra TTL chip (74LS30):

        +--------+    S1   ,--WWW--+5V  +5V===|)
   A0---|S2   Y3*|---o o---+------------------| )
   A1---|S1   Y1*|----------------------------|  )
   A2---|S0   Y5*|----------------------------|   )o--|&gt;o--+------MemExp*
        |     Y6*|----------------------------|  )         |
        |     Y7*|----------------------------| )          |
  +5V---|G1   Y0*|  Gnd---WWW--+---|&gt;o--=)&gt;---|)           ,=|)---CardSel*
Memen*--|G2A* Y4*|        4.7K |       |                   |
  Gnd---|G2B*    |  Bit2--o o--+-------|---,               |
        |     Y2*|----,   S2           |   |               |
        +--------+    |                |   |               |
         Bit0---|&gt;o----=)&gt;-------------+----=)&gt;------------+------MapSel*
             74LS04                      74LS32

Y1\*, Y5\*, Y6\* and Y7\* are combined via an 8-inputs NAND gate and
trigger the active-low MemExp\* signal when an address in the range
\>2000-3FFF or \>A000-FFFF is accessed. Y3\* plays the same role for
addresses in the range \>6000-7FFF.

Y2\* is combined via an OR gate with the inverse of CRU bit 0, so that
addresses in the range \>4000-5FFF are only active when CRU bit 0 is set
to 1. This signal is further ORed with CRU bit 2 or its inverse so that
when bit 2 is 0 the 74LS612 maps at \>4000-5FFF, whereas when bit 2 is
set to 1 the SRAM maps in this area.

The switches and corresponding pull-up/pull-down resistors are optional:
they let you disable the extra mapping areas and have the card react
just like the original Superam (only with more memory). S1 disables
mapping at \>6000-7FFF when open. S2 disables CRU bit 2 when open, so
\>4000-5FFF always accesses the 74LS612 registers. Note that this can be
done by software on the 8 mega card, since MO0 is used to enable/disable
mapping.

##  Software

#### Low-level routines

You will have noted that the original SuperAMS does not map any memory
at \>4000-5FFF, thus cannot contain any DSR. This is because it is a
memory expansion card, and these do not need DSRs.

However, we still need some way to control memory paging. This must be
done by the application program.

The principle is very easy. You can set a page number by setting CRU bit
0 to 1, and writing a page number into 8 among the 16 available
registers.

|         |             |
|---------|-------------|
| Address | Paged area  |
| \>4004  | \>2000-2FFF |
| \>4006  | \>3000-4FFF |
| \>4014  | \>A000-AFFF |
| \>4016  | \>B000-BFFF |
| \>4018  | \>C000-CFFF |
| \>401A  | \>D000-DFFF |
| \>401C  | \>E000-EFFF |
| \>401E  | \>F000-FFFF |
| Others  | Inactive    |

It may seem harmless to store arbitrary values in the remaining
registers, as mapping never occurs in the area that they control.
However, these should not be used to store data irrelevant to mapping,
as a future version of the card may want to extend mapping to the
\>4000-7FFF area, for instance.

As mentionned above, CRU bit 1 can be used to toggle between mapping and
transparent mode. A `SBO 1` enables mapping in the whole area
\>2000-3FFF and \>A000-FFFF, a `SBZ 1` puts it in transparent mode (page
2 at \>2000-2FFF, page 3 at \>3000-3FFF, etc)..

* Page # in R0 msb, memory address in R1
MAPAGE LI   R12,&gt;1E00        Superams CRU address
       SRL  R1,12            Reduce address to 4K chunks
       SLA  R1,1             Registers are 2 bytes appart
       SB0  0                Access the mapper
       MOVB R0,@&gt;4000(R1)    Set the page number
       SBZ  0                Back to mapping mode
       B    *R11`
* Enables mapping
MAPON  LI   R12,&gt;1E00        Superams CRU address
       SBO  1                Mapping mode
       B    *R11`
MAPOFF LI   R12,&gt;1E00        Superams CRU address
       SBZ  1                Transparent, mapping off
       B    *R11`
* Set page number, for 8 Megs modification
* Page # in R0, memory address in R1
* Add &gt;0800 to enable mapping in a bank
MAPAGX LI   R12,&gt;1E00        Superams CRU address
       SRL  R1,12            Reduce address to 4K chunks
       SLA  R1,1             Registers are 2 bytes appart
       SWPB 0                Msb is in odd byte
       SBO  2                Access the mapper at &gt;4000-5FFF
       SBO  0                Turn it on
       MOV  R0,@&gt;4000(R1)    Use MOV instead of MOVB
       SBZ  0                Access off
       SBZ  2                SRAM maps at &gt;4000-5FFF
       SWPB 0                Restore page #
       B    *R11

Note that the old version of MAPAGE also works with an 8 megs card . The
MOVB instruction will latch \>FF in the 74LS273 because it first reads
the odd byte to replace it later, and the card does not answer so all
lines read high. The old software will thus access the last SRAM chips.
And thanks to the inverter between 74LS273/Q0 and 74LS612/D0, mapping
will always be on.

####  High-level languages

A complete development system (assembler, linker, etc) was released by
R.A. Green of RAG Software, that lets you write assembly programs in a
transparent manner, i.e. without having to worry about page switching.

A variant of C99 was designed by Joe Delekto to take advantage of the
SuperAMS card.

Brad Snyder created "XBpacker", an utility that allows to store several
Extended Basic programs in the SuperAMS card, or even one huge XB
program written in a modular fashion.
Revision 1. 7/7/00 Preliminary.
Revision 2. 7/10/00 Added credits, high-level languages, corrected
errors. OK to release.

Revision 3. 7/12/00 Added address decoding logic for 8/16 megs.

Revision 4. 7/22/00 Got the 74LS612 manual, added chapter on it.
Revision 5. 7/2/02 Corrected meaning of CRU bit 1 (transparent mode).
Added AEMS.
[Back to the TI-99/4A Tech Pages](titechpages.md)
