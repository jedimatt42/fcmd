# Scratch-pad RAM

The TI-99/4A console contains 256 bytes of RAM that appear at address
\>8000-80FF. As the address is not fully decoded, the same memory
appears at \>8100-81FF, at \>8200-82FF, and at \>8300-83FF. The
convention is to use the \>8300 addresses, however well-written programs
(such as the ROM DSR in the TI cards) figure out where the RAM actually
lies and never specify any "hard-coded" address.

This tiny memory is called the scratch-pad RAM and is the only CPU RAM
in the console. Surprisingly, it is made of static RAM, which was a very
expensive solution for the time. Probably because the TI-99/4A was
originally designed around a microprocessor containing 256 bytes of RAM.
When the TMS9900 was substituted for it, external RAM had to be added
and SRAM was chosen because it did not require a refresh circuit like
DRAM (or so I was told).

The scratch-pad is implemented as two 128 bytes chips, one holding the
odd bytes, the other one holding the even bytes. Together, they make up
a 16-bit data bus which results in quicker memory access. Apart for the
console ROMs, this is the only 16-bit memory in the TI-99/4A system.

[Pinout
](#Pinout)[Content](#content)

## Pinout

             +----+--+----+
         Vss |1 o  M    24| Vcc              MCM 6810P
      D0/D11 |2    C    23| A6
      D1/D10 |3    M    22| A5
      D6/D9  |4         21| A4
      D5/D8  |5    6    20| A3
      D3/D15 |6    8    19| A2
      D4/D14 |7    1    18| A1
      D2/D13 |8    0    17| A0
      D7/D12 |9    P    16| R/W*
        CS0  |10        15| CS5
        CS1* |11        14| CS4*
        CS2  |12        13| CS3
             +------------+

Power supply
**Vcc** +5V
**Vss** Ground
CPU interface
**A0-A6** Address bus. Connected to the console address bus, lines
A8 to A14.
**D0-D7** Data bus. In the TI-99/4A, one chip is connected to lines D0
to D7, the second chip to lines D8-D15.

**CS1\*** Chip select. In the TI-99/4A, active (low) when memory in the
range \>8000-83FF is accessed. The selection is performed by a 74LS138
decoder, with A0, A1 and A2 as inputs, and MEMEN\* as enabling input
G2A\*. Output Y4\* is combined with address lines A3, A4 and A5 via
three cascading OR gates. The output of the last gate controls CS1\*.
**CS2, CS4*\*,*** **CS5**. Chip select. Hardwired to the ground in the
TI-99/4A.
**CS0, CS3**. Chip select (active high). Hardwired to +5V in the
Ti-99/4A.

**R/W\*** Read/write selection: low for write operations, high for
reads. In the console, this line is controlled directly by the WE\* pin
of the TMS9900 CPU.

##  Contents

Being RAM, the scratch-pad can contain any kind of data. However, the TI
operating system in the console ROMs expects some values at precise
locations. So do the GPL interpreter, the TI-Basic interpreter and the
Extended-Basic interpreter. Of course other programs may use the
scratch-pad at their leisure... The tables below attempt at summarizing
important values.

The first part of the scratch-pad is used mainly by TI-Basic and
Extended Basic. If a program does not require any of these languages,
these bytes are free for use.


3=1 Trace, 4=1 Edit mode, 5=1 On warning stop,
6=1 On warning next, 7=1 Auto-num, 0&amp;2 reserved.
to convert a line of text into Basic tokens).
The second part of the scratch-pad memory is more general and heavily
used by the GPL interpreter and the console ROM routines. The names in
the left column are used by my GPL assembler, most (but not all) of them
are generally accepted.


Also: sign of a real number
Also: exponent of a real number
Basic)
Quit key
mem
&gt;02 multicolor mode, &gt;01 sound table in VDP mem
Revision 1. 3/2/00 Preliminary
Revision II. 3/4/00 OK to release
Revision III. 7/18/01 Corrected XB flagbits in \>8345
Revision IV. 1/30/06  Corrected description of CS1\* decoding
[Back to the TI-99/4A Tech Pages](titechpages.md)
