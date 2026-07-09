# Understanding the CRU

## Introduction

The CRU (Communication Register Unit) can be seen as a bunch of 4096
wires connecting the TMS9900 microprocessor to various peripherals. Of
course, there are not that many physical lines, in fact there are only
three (CRUCLK, CRUOUT and CRUIN). How is this possible?

Well, the TMS9900 makes use of the address bus (lines A3 through A14) to
identify the CRU "wire" that needs to be used. A special line, CRUIN, is
used for input operations and another one, CRUOUT for output operations.
Note that the regular data bus is not used by CRU operations: CRUIN and
CRUOUT form a tiny data bus reserved for CRU operations. Finally, CRUCLK
is activated (i.e. goes high) to signal a CRU write operation.

To communicate with the TMS9900 via the CRU a peripheral device must
watch the address bus. When an address appears that matches its own CRU
address, the peripheral places data on the CRUIN line. If CRUCLK is
active, the peripheral knows it can also import one bit of data from the
CRUOUT line. The CRUCLK line is required for the peripheral to
distinguish CRU output operations from regular memory access, via the
data bus. It does not matter that the peripheral sends data on CRUIN
during memory operations, since the TMS9900 just ignores them.

It should be noted that CRUOUT is only available as such inside the
TI-99/4A console. In the PE-box, it is multiplexed with the extra
address line A15 (that does not exist in the console, since the data bus
is 16 bits there).

Forbidden opcodes
Note that a few other assembly language instructions also activate
the CRUCLK line. These are CKON, CKOF, LREX and RSET. Each places a
different address on the address bus lines A0-A3 (real CRU operations
place zeros on these lines). BUT...most peripherals do not bother with
checking that part of the address bus! Which means that the above
instructions will be missinterpreted as CRU operations, with
unpredictable results. That's why those instructions should never be
used with the TI-99/4A.
**Hardware  **
[Address decoders](#decoder)
[Bit latching and emiting](#latches)
[ROM selection](#ROM%20select)

**[Software](#software)**

**CRU maps  **
[Console
](#console%20map)[TI disk controller](#disk%20controller)
[RS232 card](#rs232)
[P-code card
](#Pcode)[Gram Karte](#Gramkarte)
[IDE interface card](#IDE%20card)

##  Hardware considerations

A peripheral card only requires a few TTL components to handle CRU
operations. Its main tasks are: 1) decoding the address bus so that the
card only answers to the proper CRU address, 2) latch CRU bits during
output operations, issue bits during CRU input operations, and 3)
optionally turn on DSR memory at addresses \>4000-5FFF when CRU bit 0 is
set.

###  Address decoder

Addresses \>0000-0400 are reserved for the TMS9901 in the console (it
only uses 64 bits, but the associated decoding circuitery answers to
every address upto \>03FE). Traditionally peripheral cards have
addresses in the range \>1000-1F00, because the scanning routines in the
console ROM search card ROMs at CRU address \>1000 to \>1F00, by
increments of \>0100. If you were to set a CRU address in the range
\>0400-0FFE, no DSR, subprogram, power-up or interrupt routine will be
found in your card ROM. Now if your card does not have a ROM (or has no
standard header in its ROM), the lower addresses are perfectly ok.

Traditionally, each card gets a pool of 128 successive addresses, i.e.
the CRU addresses of different cards are \>0100 apart. This means that
only A0-A7 need to be decoded; however we saw above that A0-A2 can
safely be assume to be zero during CRU operations. We are thus left with
only five lines to decode: A3-A7. We could also exclude memory
operations by checking the MEMEN\* line and make sure it's high, but
this is not strictly required: the TMS9900 ignores the CRUIN line during
memory operations, and CRU output operations are signaled via the CRUCLK
line.

     10k  |
          |   +---------+
A3&gt;--o o--+---| A=B     |         10k
A4&gt;-----------| A0   B0 |-----+--wwww---+-- +5V
A5&gt;-----------| A1   B1 |----+|--wwww---+
A6&gt;-----------| A2   B2 |---+||--wwww---+
A7&gt;-----------| A3   B3 |--+|||--wwww---+
              |         |  ||||
        Gnd---| A&lt;B     |  oooo  Select device CRU address
              |         |  oooo  with these switches
        Gnd---| A&gt;B     |  ||||
              |         |  Gnd     &#39;04
              |     A=B |---------|&gt;o----+------&gt; CRU_enableI* (active low)
              +---------+                | &#39;32
                74LS85     CRUCLK*&gt;-------=)&gt;---&gt; CRU_enableO* (active low)

The above circuit makes use of a 74LS85 4-bit comparator. You can
connect one set of inputs (pins 10, 12, 13 and 15) to address lines
A4-A7, the other set (pins 9,11,14 and 1 respectively) to 10K pull-up
resistors and a DIP switch connected to ground. This way we can manually
select the CRU address of our card in the range \>1000-1F00 by just
composing the binary equivalent of 0 through \>F on the switches.

A3 must also be handled, but the 74LS85 only compares 4 bits. A nifty
solution is to make use of the cascade inputs of the comparator. There
are three of them: A\B (pin 4).
Unfortunately, when A=B is high the other two inputs are ignored, so
only A=B is of any use: we'll connect A3 to it, so as to enable the
74LS85 when A3 is high.

Optionally, we can place a switch in the A3 line with a 10K pull down
resistor. This way, the user can inactivate the card by opening the
switch, which may come handy if the card takes over during power-up. The
P-code card and the Horizon Ramdisk have such switches (although they do
not use a 74LS85 for decoding).

The "A=B" output of the comparator provides an active high seletion
signal. You may want to invert it , as most chips have an activelow
activation input.

For output operations, we must also check the CRUCLK\* line and make
sure it's low. Note that CRUCLK is inverted in the peripheral box and
thus becomes CRUCLK\*, which is active low (as indicated by the \*). For
CRU input operations, we should not use CRUCLK\* as it will never be
active. N.B. If you prefer active high selection signals, move the
inverter to the CRUCLK\* line and replace the OR gate with an AND gate
(e.g. 74LS08).
#### Alternative circuit

               +---------+          ,------------&gt;CRU_enableO
A3&gt;------------| G1   Y0*|-----o o--+     &gt;1000
A4&gt;------------| S0   Y1*|-----o-o--+     &gt;1800
A5&gt;------------| S1   Y2*|-----o o--+     &gt;1400
A6&gt;------------| G2A* Y3*|-----o o--&#39;     &gt;1C00
A7&gt;------------| G2B*    |
               |      Y4*|-----o o--,     &gt;1000
CRUCLK*&gt;-------| S2   Y5*|-----o-o--+     &gt;1800
               |      Y6*|-----o o--+     &gt;1400
               |      Y7*|-----o o--+     &gt;1C00
               |         |          &#39;------------&gt;CRU_enableI
               +---------+

Here is an alternative circuit for a read-and-write peripheral device,
using a 74LS138 decoder. It provides two active low signals, one for
output operations, one for input. The DIP switches are used to select
one out of the four CRU addresses the circuit can answer to. In the
exemple above, the address \>1800 was selected. Note that only one
switch should be closed at a time (unless you want your card to appear
at two distinct CRU addresses). You can obtain other addresses than the
above four by choosing which of the A4-A7 address lines goes to a G2x\*
active low input and which goes to the S0 and S1 selection inputs. If
needed, inverters could be placed on the lines going to G2A\* and G2B\*.

With a read-only device there is no need to check CRUCLK\*, so S2
becomes available for an extra address line and the user has a choice of
eight possible CRU addresses.

###  Bit latching/emitting

We can decode and store 8 CRU bits using a 74LS259 addressable latch:
let's connect A15 to the D input (pin 13) , A12, A13 and A14 to the S0,
S1 and S2 inputs respectively (pins 1-3), our enabling signal (a
combination of CRUCLK\* and the address decoder) to EN\* (pin 14), pull
the RST\* reset input high (pin 15) or connect it to the RESET\* line,
and that's it. The output pins Q0 through Q7 (pins 4-7 and 9-12) become
eight wires virtually connected to the TI-99/4A console. To decode more
than 8 bits, we can use additionnal 74LS259 and a 74LS138 decoder to
enable one or the other. We could also use a 74LS154, which is a 16-bit
equivalent of the 74LS259.

If our peripheral is to allow CRU input operations a simple way is to
use a 74LS251 multiplexer. Let's connect upto eight output wires to the
inputs A0 through A7 (pins 4, 3, 2, 1, 15, 14, 13, and 12), address
lines A12, A13, and A14 to inputs S0, S1 and S2 respectively (pins 1-3)
and our enabling signal (from the address decoder, ignoring CLUCLK\*) to
the EN\* input (pin 7). The non-inverted output Y (pin 5) will be
applied to line CRUIN. Here again, you can use additional 74LS251
(controlled by the same 74LS138 decoder used for write operations) to
implement more than eight lines.

It's a good practice to mirror each output bit on the 74LS259 to an
input bit on the 74LS251, so that the user can determine the current
settings of the peripheral. But this is by no means required.

              +----------+                +----------+
A12&gt;----------| S2    Q0 |------Bit0------| A0       |
A13&gt;----------| S1    Q1 |------Bit1------| A1     Y |----------&gt;CRUIN
A14&gt;----------| S0    Q2 |------Bit2------| A2       |
              |       Q3 |------Bit3------| A3       |
A15&gt;----------| D     Q4 |------Bit4------| A4       |
              |       Q5 |------Bit5------| A5       |
CRU_enableO*&gt;-| EN*   Q6 |------Bit6------| A6       |
              |       Q7 |------Bit7------| A7       |
RESET*&gt;-------| RST*     |                |          |
              |          |  CRU_enableI&gt;*-| EN*      |
              +----------+                +----------+

####  DSR ROM selection

Bit 0 in a card address space has a special meaning: it should turn on
card ROM (or RAM) memory so that it appears at adresses \>4000-5FFF. The
TI-99/4A manipulates the CRU bit 0 of every card it finds during power
up, to look for power-up routines and programs in the card ROMs. The
same thing occurs after a peripheral interrupt, or if the user calls a
DSR (e.g. with a file operation) or a subroutine (e.g. with a basic
CALL): every card is searched for the corresponding routine until one of
them answers it. Concretely, this means that we cannot use bit 0 for any
other purpose than turning ROM on. Generally, it will also turn on the
light for that card in the PE-Box, but this is not an absolute
requirement: the RS232 card uses bit 7 for that purpose.

              +----------+    |   100 Ohms
A0&gt;-----------| S2    Y0*|    V&quot; LED
A1&gt;-----------| S1    Y1*|    |
A2&gt;-----------| S0    Y2*|----+---&gt;CS* select memory in range &gt;4000-5FFF
              |       Y3*|
MEMEN*&gt;-------| G2A*  Y4*|
        Gnd---| G2B*  Y5*|
CRU bit0&gt;-----| G1    Y6*|
              |       Y7*|
              |          |
              +----------+

The above circuit uses a 74LS138 decoder to answer to memory requests on
the range \>4000-5FFF. It decodes A0-A2 which results in selecting
\>2000 blocks with the eight Yx\* active low outputs. The one for
\>4000-5FFF is Y2\*, it should be applied to the chip select CS\* pin of
our ROM memory. In this exemple, it also shines a light emitting diode
(LED).

CRU bit 0 is connected to the G1 enabling entry, which means the outputs
will only become active when this bit is set to 1. MEMEN\* signals a
memory operation and is applied to the active low enabling entry, to
prevent the device from mistaking CRU operations for memory operations.
Finally, G2B\* has been connected to ground to enable it. Note that we
could have used a switch here, with a 10K pull-up to +5V: this way we
could disable the card by opening the switch.

##   Software considerations

The TMS9900 microprocessor looks for the address of the CRU line in the
R12 register and copies bits 3 through 14 on the address bus, lines A3
through A14. It sets A0-A2 low to differentiate CRU operations from the
four "external" opcodes and places the data to send out (if any) on
CRUOUT. Note that this means that R12 must contain the CRU address
*times two,* since it must begin with address line A14 (whose wheight is
\>0002). That's a very frequent cause for bugs when writing assembly
programs that deal with the CRU.

Once the address has been placed in R12, you can use five different
instructions to access the CRU "vitual wires". These are:

**SBO** Set Bit to One. Sets the coresponding wire as "high" in the
peripheral.
**SBZ** Set Bit to Zero. Sets the corresponding wire as "low" in the
peripheral.
**TB** Test Bit. Checks the corresponding wire to see whether it's high
or low.
**LDCR** LoaD CRu. Sets the electrical state of 1 to 16 wires in the
peripheral.
**STCR** STore CRu. Checks the electrical state of 1 to 16 wires.

####  SBO *bit*

This instruction sets a wire "high" in the peripheral, i.e you are
sending a "1" bit to the peripheral. The syntax is very easy: *bit* is a
number from -127 to +128 representing an offset from the wire addressed
in R12. This is very convenient since you don't need to change R12 each
time you want to address another wire. Just place the lowest (for
instance) address in R12, and then use SBO 0 to access the first wire,
SBO 1 for the second, SBO 2 for the next one, etc.

####  SBZ *bit*

The syntax is the same as for SBO: *bit* is the number of the wire,
relative to the one selected in R12. This wire will be set as "low" by
the instruction, i.e. you are sending a "0" bit to the peripheral.

####  TB *bit*

Just as the two above: *bit* is the number of the wire you want to test,
relative to the one addressed by R12. Another way of putting it is that
you're testing the status of a bit in the peripheral. The result of the
test is placed in the EQ bit of the status register. Thus you can make
decisions by placing a JEQ (jump if bit is 1, i.e. if wire is high) or a
JNE (jump if bit is 0, i.e. if wire is low) after the TB instruction.
Note that this is anti-intuitive: JEQ jumps if the bit is 1.

####  LDCR *Rx, nbits*

This instruction is slightly more complicated: *nbits* is the number of
bits to write, and *Rx* is the register where to find them. Bits are
taken from the register starting with the rightmost one: this one will
be assigned to the "wire" addressed in R12. The next bit on the left is
used for the next "wire", etc. If there are less more than 8 bits to
write, they are taken from the whole register, if however *nbits* is 1
through 8, then bits are taken from the most significant byte of the
register. If *nbits* is 0, it is understood as 16 and the whole register
is transfered.

The main hassle is that you must change the value in R12 if you want to
begin a write operation with another bit. Don't forget to double the
address in R12. Note however that most peripheral cards handle only 8
CRU bits: in this case you can set R12 once and for all. (Unless you
want to modify only some of the 8 bits while leaving the others intact,
and these are write-only bits).

####  STCR *Rx,nbits*

The syntax is identical to LDCR: *nbits* is the number of bits to read,
*Rx* is the register where to place them. If *nbits* is 1-8 bits are
placed in the left byte of the register, starting with the rightmost
one. If *nbits* is 9-0, bits are dumped into both bytes of the register,
starting with the least significant (rightmost) bit.

STCR suffers from the same problem as LDCR: you must change R12 each
time you want to start reading from a different place.

##  CRU map

This page lists the CRU bits used by the console and all peripheral
cards that I know of. If you have info on other cards, please
[share](feedback.md) them with me: I'd love to add them to this list.

In the following text I will refer to CRU addresses as R12-addresses.
This is meant to avoid the ambiguity due to the fact that the address of
a line must be dobbled when placed in R12. Thus bit 1 is at address
\>0002. An R12-address is the content of the R12 register, thus twice
the bit number.

###  Console

The first 32 bits of the CRU address space are mapped to the TMS9901
interrupt processor and keyboard decoder. Very unfortunately, the TI
engineers got lazy (or the management got cheap) and they decided not to
decode address lines A4-A10. As a result, the first half of the CRU
space, R12-address \>0000 to \>0FFF is mirroring R12-addresses
\>0000-0040. This means that the first CRU bit, can be accessed at
R12-address \>0000, or \>0040, \>0080, etc. What a waste of address
space! A possible improvement to the TI-99/4A console would be to
reclaim R12 addresses \>0040-00FF by adding a simple OR gate in the
console.



###  TI disk controller

The standard R12-address for that card is hardwired as \>1100. This was
probably meant to speed up disk access by making sure this card is one
of the first to be scanned for DSR or CALLs . Note however, that you can
install a card at address \>1000: if it contains a DSR called DSK1, it
can by-pass the TI-controller card!

|     |                               |                                 |
|-----|-------------------------------|---------------------------------|
| Bit | Meaning when read             | Meaning when written to         |
| 0   | Load head requested (HLD pin) | 1: Select ROM, turn light on    |
| 1   | Drive 3 selected              | 1: Strobe motor (for 4.23 sec)  |
| 2   | Drive 2 selected              | 0: Ignore IRQ + DRQ             |
| 3   | Drive 1 selected              | 1: Signal head loaded (HLT pin) |
| 4   | 0: Motor strobe on            | 1: Select drive 1               |
| 5   | Always 0                      | 1: Select drive 2               |
| 6   | Always 1                      | 1: Select drive 3               |
| 7   | Selected side                 | Select side                     |

###   RS232 card

The original RS232/PIO card by TI is meant to be installed either as
R12-address \>1300 or at \>1500. The card ROMs are designed to allow two
such cards per PE box: the first (at address \>1300) answers to calls to
RS232/1, RS232/2 and PIO/1, the second card (at address \>1500) deals
with RS232/3, RS232/4 and PIO/2.

In addition to the first eight bits traditionally used at the beginning
of the address space, each TMS9902 asynchronous communication controller
uses 64 bits of CRU space. The first chip (RS232/1 or RS232/3) is at
R12-address \>1x40, the second chip (RS232/2 or RS232/4) at \>1x80. In
the table below, I renumbered bits from the base address of each chip,
since changing R12 is the usual way to address a given chip.


Value to load in selected register (11 bits)
Reset by bit 18
Reset by loading Emission register
Reset by bit 20
Reset by bit 21
Reset by bit 18
Reset by loading Emission register
Reset by bit 20
Reset by bit 20
Reset by bit 21

###  P-Code card

The base R12-address for this card is \>1F00. Only two bits are active,
although a jumper on the card allows bit 7 to control the LED, instead
of bit 0. Note that, due to the circutery on the card, the bit addresses
appear as two blocks.


1: ROM page 2.

###  German Gram-Karte

This card has a DIP switch that allows to install it at any CRU address
in the range \>1000-1F00.


1: Default bank appears (DIP selected 1-4)
1: Writing to &gt;6000-7FFF writes data

###  Horizon Ramdisk

This card has a DIP switch that allows to select its CRU address in the
space \>1000-\>1700. By selecting \>1000 one can ensure that the Ramdisk
will be accessed before the floppies.

|     |             |     |                                 |
|-----|-------------|-----|---------------------------------|
| Bit | R12 address | I/O | Usage                           |
| 0   | \>1x00      | O   | 1: Turn card ROM (and light) on |
| 1   | \>1x02      | O   | Page selector                   |
| 2   | \>1x04      | O   | Ditto                           |
| 3   | \>1x06      | O   | Ditto                           |
| 4   | \>1x08      | O   | Ditto                           |
| 5   | \>1x0A      | O   | Ditto                           |
| 6   | \>1x0C      | O   | Ditto                           |
| 7   | \>1x0E      | O   | Ditto                           |
| 8   | \>1x210     | O   | Ditto                           |
| 9   | \>1x12      | O   | Ditto                           |
| 10  | \>1x14      | O   | Ditto                           |
| 11  | \>1x16      | O   | Ditto                           |
| 12  | \>1x18      | O   | Ditto                           |
| 13  | \>1x1A      | O   | Ditto                           |
| 14  | \>1x1C      | O   | Ditto                           |
| 15  | \>1x1E      | O   | 0: Ramdisk mode 1: RAMBO mode   |

###  IDE interface card

The base address for this card can be set with a DIP-switch to any
address between \>1000 and \>1F00. Since this is a user-built card, not
all CRU bits may be implemented, or they may be shuffled around. The
software compensates for this. Below are the default settings:


&lt;&gt; switch: SRAM maps at &gt;4000-40FF
Revision 1. 3/25/99 OK to release
Revision 2. 9/1/99 VDP+periph interrupts were inverted!

Revision 3. 3/19/01 Changed the decoder schematics (include MEMEN\*,
correct 74LS85 behaviour).
Revision 4. 9/9/01 Added p-code card CRU.
Revision 5. 9/22/01 Added my IDE card.
[Back to the TI-99/4A Tech Pages](titechpages.md)
