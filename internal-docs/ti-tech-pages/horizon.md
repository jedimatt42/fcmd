# The Horizon 3000 Ramdisk

The Horizon Ramdisk is probably the most usefull piece of hardware I
ever bought for my TI-99/4A. The basic concept behind it is very simple:
it consists only in baterry-backed SRAM memories. The DSRs are written
in such a way that you can access the SRAM as you would access a floppy
drive. Or rather, several floppy drives as the maximum SRAM size tops at
3 Megs! And of course, access in immensely faster than with a real
floppy disk, or even a hard disk. Not to mention a lot of nifty features
that were added in the DSR...

An extra improvement to the Ramdisk is RAMBO mode. It requires an extra
piece of hardware but allows the user to access the SRAM direcly, as 8K
pages that appear in the area \>6000-7FFF. The DSRs make sure a RAMBO
page will never overlap with virtual disk data.

The original Horizon Ramdisk was designed by Ron Gries. Two other
versions, the Horizon 2000 and the Horizon 3000, were subsequently
released by Bud Mills at:

Western Horizon Technologies

11445 Clayton Road San Jose, CA 95127 (408)-259-4411



**Hardware  **
[CRU logic
](#CRU%20logic)[Memory selection
](#Memory%20selection)[Page selection
](#Page%20selection)[The 1-91 revision
](#Rev191)[Power supply
](#Power%20supply)[Phoenix mod
](#Phoenix)[Memory expansion
](#Mem%20exp)[RAMBO board](#RAMBO%20board)

**Software  **
Ramdisk Operating System
[DSRs
](#ROS,%20DSRs)[Subprograms
](#subprograms)[Power-up
](#Power-up)[ISR
](#ISR)[Personnal additions
](#personnal%20additions)Other programs
[Test programs
](#Other%20softwares)[Configuration
](#CFG)[Demo](#Demo)

##  Hardware

The Horizon 3000 board design is very elegant in its simplicity. An 8K
SRAM chip maps in the area \>4000-57FF and contains the DSRs. Other SRAM
chips, that can add up to 3 Megabytes, map by pages of 2K in the area
\>5800-5FFF. The page selection is performed by storing the appropriate
bit pattern in 14 CRU bits.

The board could be bought either preassembled and tested, or as a kit.
It came with a very detailed manual on how to build and test it. Click
[here](hor_card.md) for a commented picture and [here](hor_back.jpg)
for the back of the card (not very interesting, but someone requested
it, so here it is)..

#### CRU logic

The CRU address for the board is selectable via a 8-bit DIP-switch. It
allows to choose a CRU address in the range \>1000-1700. Address \>1000
is especially interesting, since it is scanned before the floppy disk
controller (whose address is \>1100). This way, you can have the DSK1 to
DSK3 DSRs in the Ramdisk and bypass the corresponding floppy drive.

The CRU address is processed by a 74LS138 decoder that decodes memory
lines A5-A7. Line A4 is connected to the enabling pin G2A\* (and must
therefore be low), whereas A3 is connected to G1 (and must be high). CRU
operations are identified by a low signal on the CRUCLK\* line,
connected to the G2B\* selection pin. Note that CRUCLK\* is only active
during CRU output operations, but since the Ramdisk does not allow CRU
input this design is ok.

The 8 switches of the DIP switch are connected together on one side and
to the decoder outputs Y0\* through Y7\* on the other. To select a CRU
address, one of the switches must be closed, all the other left open.

The output of the DIP switch is further combined with A11, via another
74LS138, wich allows for the use of 16 CRU bits, instead of the usual 8.
The outputsY0\* and Y1\* of this second decoder each enable a 74LS259
addressable latch. From here, we have a typical CRU circuit: the
74LS259s decode address lines A12-A14, take their data input from
A15/CRUOUT and are reset by the RESET\* line.

CRU bit 0 is used by the memory selection circuit, and also powers a
2N2222 transistor that lights up a red LED visible from the front panel.
CRU bit 15 is used by the RAMBO board, all other bits serve as page
selection lines for the SRAM.

           +--------+                  +--------+                +--------+    |/  LED
     A7&gt;---|S0   Y0*|--o-o--,   A11&gt;---|S0   Y0*|------,  A12&gt;---|S2    Q0|----| 
     A6&gt;---|S1   Y1*|--o o--+   Gnd----|S1   Y1*|---,  |  A13&gt;---|S1    Q1|-   |\
     A5&gt;---|S2   Y2*|--o o--+   Gnd----|S2      |   |  |  A14&gt;---|S0    Q2|-     &#39;-www-- Gnd
           |     Y3*|--o o--+          |        |   |  |         |      Q3|-      100 Ohms
CRUCLK*&gt;---|G2B* Y4*|--o o--+----------|G2B*    |   |  |  A15&gt;---|D     Q4|- 
     A4&gt;---|G2A* Y5*|--o o--+   Gnd----|G2A*    |   |  |         |      Q5|-
     A3&gt;---|G1   Y6*|--o o--+   +5V----|G1      |   |  &#39;---------|EN    Q6|-
           |     Y7*|--o o--&#39;          |        |   |   RESET*---|RST   Q7|- cru bit 7
           +--------+                  +--------+   |            +--------+
                                                    |            74LS259 #2
                                                    |            +--------+ 
                                                    |     A12&gt;---|S2    Q0|- cru bit 8
                                                    |     A13&gt;---|S1    Q1|- 
                                                    |     A14&gt;---|S0    Q2|- 
                                                    |            |      Q3|- 
                                                    |     A15&gt;---|D     Q4|-  
                                                    |            |      Q5|-               
                                                    &#39;------------|EN    Q6|-
                                                        RESET*---|RST   Q7|- cru bit 15  
                                                                 +--------+ 

####  Memory selection

The DSRs are contained in a 6264LP-12 8Kbytes SRAM chip. In fact, only
the first 6K are used, since addresses \>5800-5FFF are reserved for
Ramdisk access.

A 74LS138 decodes lines A0 through A2 of the address bus in chunks of
\>2000 bytes. The G2A\* input of the decoder is enabled by the MEMEN\*
line, so as to exclude CRU operations. The G1 input is pulled up at +5V
by a resistor, but can be grounded by with an external switch: this
prevents the card from answering to any memory reques (very handy when
something messed up the DSRs).

The Y2\* output of the decoder is activated by memory accesses in the
range \>4000-5FFF. It is used to enable another decoder, a 74LS156 that
decodes address lines A3 and A4, as well as CRU bit 0. Three of the
decoder outputs, Y0\*, Y1\*, and Y2\* are connected together and pulled
up to +5V by a 2.7K resistor. This "wired or" design results in a
selection line that reacts to memory operations in the range
\>4000-57FF, without requiring extra OR gates. This line is connected to
the CS1\* chip selection pin of the 6264 memory chip.

Output Y3\* is similarly used to select the SRAMs for any access in the
range \>5800-5FFF. Note the weird way these two selection signals are
combined with diodes to generate the selection signal RDBENA\* that
enables the data bus drivers in the connection card and cable. A similar
circuit, with an additional 2.7K pull-up to +5 volts, enables the
74LS245 that buffers the data bus on the Ramdisk. By the way, the
address lines A3-A14 and the WE\* line are buffered by two 74LS144.

Pin CS2 of the 6264 is connected to +5V via a LED, which is not visible
on the front panel of the PE-box, but can be checked by opening the box.
It's also grounded by a 2.7K resistor. When the power is off, a low
level on this pin will put the 6264 in a high impedance state, which
saves power (the 6264 is powered by a battery-backed
supply).

           +--------+               +--------+      2.7K              +--------+
     A2&gt;---|S0   Y0*|    CRU bit 0--|S2   Y0*|---+--www--+5V   A15&gt;---|A0    D0|---&lt;D5
     A1&gt;---|S1   Y1*|         A5&gt;---|S1   Y1*|---+             A14&gt;---|A1    D1|---&lt;D6
     A0&gt;---|S2   Y2*|---,     A5&gt;---|S0   Y2*|---+----------,  A13&gt;---|A2    D2|---&lt;D7
           |     Y3*|-, |           |     Y3*|-------,      |  A12&gt;---|A3    D3|---&lt;D0
 MEMEN*&gt;---|G2B* Y4*| | &#39;-----------|G1      |       |      |  A11&gt;---|A4    D4|---&lt;D1 
     Gnd---|G2A* Y5*| |             |        |       |      |  A10&gt;---|A5    D5|---&lt;D2
+5V--www-+-|G1   Y6*| &#39;--Rambo sel  |        |       |      |   A9&gt;---|A6    D6|---&lt;D3
    2.7K | |     Y7*|               |        |       |      |   A8&gt;---|A7    D7|---&lt;D4
Gnd--o o-&#39; +--------+               +--------+       | 1N34A|   A7&gt;---|A8      |
                                              ,------|--|&gt;|-+   A6&gt;---|A9    W*|---&lt;WE*
                       RDBEN*&gt;----------------+--|&gt;|-+      |   A5&gt;---|A10     |
                                        2.7K         |      |   A4&gt;---|A11   G*|---Gnd
                                  +5V---www---+------|--|&gt;|-+   A3&gt;---|A12     |
                   74LS245 G*-----------------+--|&gt;|-+      |         |        |
                                               1N34A |      &#39;---------|CS1*    |
                                                     | +5V--|&gt;|---+---|CS2     | 
                                     SRAMsel---------&#39;      LED   |   +--------+ 
                                                       Gnd--www---+               2.7K
                                                            2.7K  :     PN2222  ,-www----+5V
                                                                  :(1)     | ,--+---Voff
                                                                  &#39;---www--|/
 (1) Rev 1/91 only                                                   2.2K  |\___Gnd                                                                      

####  Page selection logic

Address lines A5 through A15 are connected to the SRAM address lines.
They form an address from \>0000 to \>07FF, which accesses a byte
whithin a RAM "page". The page selection is achieve via 14 CRU bits,
which gives us a theoretical maximum of 16384 pages, or 32 megs
(however, design considerations limits the memory size to 3 megs.
Anyway, who can afford 32 Megs of SRAM?). The CRU bits are first use to
drive the remaining address pins on the SRAM chips, the exact number of
pins depends on the size of the SRAM. Then, upto 6 CRU bits are
demultiplexed via a 74LS138 decoder and upto four 74LS154 16-bits
decoder, which allow us to select one in 32 SRAM chip.

          +--------+                  +--------+                   +--------+
    X0----|S0   Y0*|------,     X2----|S0      |-+          A15&gt;---|A0      |-+
    X1----|S1   Y1*|----, |     X3----|S1   Y0*| |          A14&gt;---|A1    D0| |---&lt;D5
    Gnd---|S2   Y2*|pig | |     X4----|S2   Y1*| |          A13&gt;---|A2    D1| |---&lt;D6
          |     Y3*|pig | |     X5----|S3   Y2*| |          A12&gt;---|A3    D2| |---&lt;D7
    Gnd---|G2B* Y4*|    | | SRAMsel---|G1*  Y3*| |          A11&gt;---|A4    D3| |---&lt;D0 
(2) Gnd---|G2A* Y5*|    | |   (1)     |     Y4*| |          A10&gt;---|A5    D4| |---&lt;D1
    +5V---|G1   Y6*|    | |           |     Y5*| |           A9&gt;---|A6    D5| |---&lt;D2
          |     Y7*|    | |           |     Y6*| |           A8&gt;---|A7    D6| |---&lt;D3
          +--------+    | |           |     Y7*| |           A7&gt;---|A8    D7| |---&lt;D4
                        | |           |     Y8*| |           A6&gt;---|A9    W*| |---&lt;WE*
                        | |           |     Y9*| |           A5&gt;---|A10     | |
                        | |           |    Y10*| |     CRU bit 2---|A11  OE*| |---Gnd
                        | |           |    Y11*| |     CRU bit 1---|A12     | |
                        | |           |    Y12*| |     CRU bit 4---|A13     | |
                        | |           |    Y13*| |etc  CRU bit 3---|A14     | |
                        | |           |    Y14*|------,   X6-------|A15     | |
                        | &#39;-----------|G2* Y15*|----, |   X7-------|A16     | |
                        |             +--------+ |  | |   X8-------|A17     | |
                        &#39;--------------|G2*      |  | |   X9-------|A18/CS2 | |
                                       +---------+  | |  +5Vbat----|CS2/A18 | |
                                                    | &#39;------------|CS1*    | |
 (1) Voff on 1/91 revision                          |    etc       +--------+ |
 (2) SRAMsel on 1/91 revision                                       +---------+

Several types of SRAM chips can be used, depending how much memory you
need... and how much money you've got. The common point is that they
should be capable of low-power stand-by so that they can be maintained
by batteries while the PE-box power is off.

The following chips are recommended by WHT:

Hitachi HM62256-LP12. These are 32 Kbytes in size, 120 ns access time
(not critical: the TI-99/4A is 400 ns!).

NEC uPD43256-12L. Replacement for the above: 32 Kbytes, 120 ns.

Sony MSM5256-LP12. Another possible substitution: 32 Kbytes, 120 ns.

Hitachi HM66204-LP12. These are 128 Kbytes in size, 120 ns.

Hitach HM628128-LP12. Replacement for the above, 120 ns.

Hitachi HM66205. The new 512 Kbytes chips!. They have a A18 pin instead
of CS2.

Hitach HM628512. Replacement for the above. 512K bytes.

Due to space constraints, only 12 SRAM chips can be fitted on the board.
It is however possible to "piggy-back" two additional layers of 12
chips, i.e. to solder them directly on top of the others, with only
their CS1\* pin connected to a different trace on the PCB.
Alternatively, WHT sold a piggy-back board that supports the extra chips
without having to stack them up Presumably, this board contains two
additional 74HC154 decoders (see connections labelled "pig" in the
schematic) and sockets for more memory chips, but I have never seen it.

All this adds up to 1.125 megabytes with 32 Kbytes chips (3 x 12 x 32K),
4.5 Megs with 128 Kbytes chips (3 x 12 x 128K) and 18 Megs with 512
Kbytes chips.

The CRU bits issued by the two 72LS259 must be connected differently,
depending on how many chips you have, and what their size is. Note that
it is not possible to mix chips of different sizes. The Horizon 3000
ramdisk board comes with jumpers that can be installed so as to wire the
CRU bits correctly. The connections are summarized in the table below (
X0-X9 refer to the corresponding lines in the above schematic).

|         |                  |                  |                  |
|---------|------------------|------------------|------------------|
| CRU bit | 32K chips        | 128K chips       | 512K chips       |
| 1       | SRAM: A12        | SRAM: A12        | SRAM: A12        |
| 2       | SRAM: A11        | SRAM: A11        | SRAM: A11        |
| 3       | SRAM: A14        | SRAM: A13        | SRAM: A13        |
| 4       | SRAM: A13        | SRAM: A14        | SRAM: A14        |
| 5       | 74HC154: S0 (X2) | SRAM: A16 (X7)   | SRAM: A16 (X7)   |
| 6       | 74HC154: S1 (X3) | SRAM: A15 (X6)   | SRAM: A15 (X6)   |
| 7       | 74HC154: S2 (X4) | 74HC154: S0 (X2) | SRAM: A17 (X8)   |
| 8       | 74HC154: S3 (X5) | 74HC154: S1 (X3) | 74HC154: S0 (X2) |
| 9       | 74LS138: S1 (X1) | 74HC154: S2 (X4) | 74HC154: S1 (X3) |
| 10      | 74LS138: S0 (X0) | 74HC154: S3 (X5) | 74HC154: S2 (X4) |
| 11      | not used         | 74LS138: S0 (X0) | 74HC154: S3 (X5) |
| 12      | not used         | not used         | 74LS138: S0 (X0) |
| 13      | not used         | not used         | SRAM: A18 (X9)   |

CRU bit 0 is used to select the Ramdisk card and make it appear in the
\>4000-5FFF address range.
CRU bit 14 is not used.
CRU bit 15 is used by the RAMBO board, that makes the SRAMs appear in
the \>6000-7FFF address range.
Connection X1 is grounded when not used.

#### The 1-91 revision

I found out that my Horizon ramdisk had been modified by cutting a trace
and installing an extra transistor and resistor. This modification is
shown in a later version of the Horizon schematics, hand-labelled "Rev
1-91". I'm not sure why it was introduced.

In the [above schematics](#Page%20selection), it is shown that the 1-91
revision now controls the SRAMs at the level of the master 74LS138,
rather than through the 74HC154. The result is the same, but it frees
the G1\* enabling input of the 74HC154, so that it can be use to disable
the SRAMs when power is off.

This Voff signal is described in the [previous schematics](#191schem).
It is generated by a PN2222 transistor, placed between a 2.7K pull-up
resistor to the battery-backed power and the ground. The base of this
transistor is connected to the regular +5 volts, via a 2.2K resistor and
the diagnostic LED. When power is off, the transistor is passing and the
74HC154s are enabled (G1\* is connected to ground). When power is off,
the transistor is blocking and the battery-backed power disables the
74HS154s through the pull-up resistor.

####  Power supply

The Ramdisk is powered via a 5 volts, 1 Amp, 7805 voltage regulator fed
+8V from the PE-Box.

A critical feature of the Ramdisk is that it contains batteries that
preserves its content while the power is off. These can either be three
AAA nickel-cadmium rechargable batteries, or three AAA dry batteries, or
a 4.5 volts lithium battery. The nice thing with NiCad batteries is that
they are automatically recharged when the power is on. So, unless you
leave your PE-box off for several weeks, you never have to worry about
changing batteries. However, NiCad are meant to be fully discharged,
then fully recharged, which is not what's going to happen within the
Ramdisk. This may eventually lead to battery failure... Dry batteries
are easier to find but will need to be replaced after about a year. A
lithium cell should last for at least five years.

To save power, only part of the Ramdisk is connected to the
battery-backed power line: the power lines for the SRAMs, the 6264 RAM
and the two 74HC154 decoders (that's why these are HC, not LS: we need
CMOS chips, not TTL). Note the diodes, that prevent battery current from
going to the 7805 to the batteries and conversely. The ideal diodes for
this purpose should have very low leakage current: FDH300 (Fairchild) or
1N3595 (equivalent). This being said, I have installed a 1N4148 with dry
batteries and it seems to work fine.

Of course, when using NiCad batteries, diode (1) must be removed so that
the batteries can charge when the power is on. Instead, you can install
a second current-limiting resistor of 33 Ohms.

N.B. The diode on the output of a 7805 regulator will cause a small drop
in voltage (about 0.6V). To compensate for this, an identical diode is
present at the REF input of the regulator, which bumps up the voltage
accordingly.

                    +--------+       1N4004
+8V---+---+----+----|In   Out|---+----|&gt;|----+-------------&gt; +5V
      |   |    |    |  Ref   |   |           &#39;---||---Gnd
      |   |    |    +---+----+   |   1N914      10 uF
100nF =   = 10 |        |        &#39;----|&gt;|----+-------------&gt; +5V bat
      |   | uF &#39;--www---+---|&gt;|---Gnd        |
     Gnd  Gnd    2.7K    1N914               +---||---Gnd
                                             |  10 uF
                     Batteries      FDH300   |
     Gnd-----www-------|&#39;|&#39;|&#39;---------|&gt;|----&#39;   Diode with Li or dry bats
            33 Ohms             or ---www----    Resistor with NiCad bats
                                    33 Ohms  

####  Phoenix modification

This was a modification designed for use with the Geneve 9640 system,
that allows the Ramdisk to appear both as a boot drive and as a
ramdrive. I don't know much about the Geneve (even though I'm born in
Geneve...) but my understanding is that a boot drive can be upto 256K in
size and is controlled with 8 CRU bits. Whereas a ramdrive can be 800K
in size (32768 sectors of 256 bytes) and is controlled with 16 CRU bits.
A regular Horizon Ramdisk can emulate either a bootdrive or a ramdrive,
but not both together. That's where Phoenix comes into play...

The Phoenix modification splits the Ramdisk memory in two chunks, and
dedicates one to the bootdrive and one to the ramdrive. It's like having
two Ramdisks on the same board, a big one and a small one. The total can
only add up to 1056K though, not 3 Megs as is the case for the normal
Horizon Ramdisk. Each "drive" can have its own CRU address, but the
Phoenix design also allows to have them both on the same CRU address.
This is done by counting the number of CRU bits passed to the Ramdisk: 8
bits (or less) will access the bootdrive, 16 bits will access the
ramdrive.

With a Ramdisk that uses 128 Kbytes SRAM chips, the modification
consists in taking two of the 128K SRAM chips out of the control of the
74HC154 decoder \#1 and transfering them to the second 74HC154 decoder
(which is not used for Ramdisk less than 1 Meg in size). These two chips
will make up the 256K bootdrive. The remaining chips, controlled by
decoder \#1, make up the ramdrive. A slight modifications the CRU bits 5
to 7 is required, so as to maintain a continuous address space. This is
shown on the schematic below.

When using 32 Kbytes SRAM chips, the two existing 74HC154 decoders are
left untouched but a third 74HC154 must be piggybacked on \#1, and wired
as \#2 in the schematic below. This is because \#2 is already in use
with 32K chips. The memory for the bootdrive will come from 3 to 8 extra
32 Kbytes chips controlled by decoder \#3. These must be piggybacked on
the existing SRAM chips, which are already piggybacked: that means
stacking them 3 high. This modification is outlined in the schematic
below by comments in \[square brackets\].

The Horizon 3000 board already includes a socket for an extra 74LS00
chip. This chip contains 4 NAND gates, two of which are wired as a AND
gate, so as to combine two different CRU addresses. One for the ramdrive
which is selected with the DIP switch, and one for the boot drive which
is hardwired as either \>1400 or \>1600.

The other two NAND gates on the 74LS00 are wired as a flip-flop that
reacts to CRU operations on bits 0-7 and 8-15 respectively. One output
of the flip-flop enables the 74LS138 decoder that selects the 74HC154
decoder \#1, the second output controls the 74HC154 decoder \#2 directly
(or the extra decoder, \#3, in case of a 32K-chips Ramdisk).

Finally, the 6264 SRAM that contains the ROS has to be replaced with a
32 Kbyte chip (only 16K are used, but I don't think Hitachi makes 16K
chips anyhow). The additional address line (i.e. A14) is controlled by
the flip-flop, so that the DSR space can hold a different copy of the
ROS for each partition.
           74LS138 #1 |          |            74LS138 #2     |             
           +--------+ |          |            +--------+     |
     A7&gt;---|S0   Y0*|-|-o-o--,   |     A11&gt;---|S0   Y0*|-----|---+-----&gt;74LS259 #1 (bits 0-7)
     A6&gt;---|S1   Y1*|-|-o o--+   |     Gnd----|S1   Y1*|-,   |_? |
     A5&gt;---|S2   Y2*|-|-o o--+   |     Gnd----|S2      | |    ,--=|)o--+------------------,
           |     Y3*|-|-o o--+   |            |        | | ,--|--------&#39;                  |
CRUCLK*&gt;---|G2B* Y4*|-+-o o--+   |  74LS00    |        | | |  &#39;----,      ,----&gt;A14/32K   |
     A4&gt;---|G2A* Y5*|---o o--+---=|)o--=|)o---|G2B*    | | ,=|)o---+------+         SRAM  |
     A3&gt;---|G1   Y6*|---o o--+         Gnd----|G2A*    | +-&#39;              &#39;----,    (new) |
           |     Y7*|---o o--+         +5V----|G1      | |  ?                  |          |
           +--------+        |                +--------+ &#39;--|---&gt;74LS259 #2    |          |
                             &#39;------------------------------&#39;    (bit 8-15)    |          |
                                                                               |          |
 ,-----------------------------------------------------------------------------&#39;          |
 |        74LS138 #3                  74HC154 #1 [and #2]          74HC154 #2 [or #3]     |
 |        +--------+                  +--------+                   +--------+             |
 |  X0----|S0   Y0*|------,     X2----|S0      | 128K SRAMs  X2----|S0      |             |
 |  X1----|S1   Y1*|--,   |     X3----|S1   Y0*|---&gt;CS1*     X3----|S1   Y0*|---&gt;CS1*     |
 |  Gnd---|S2   Y2*|  |   |     X4----|S2   Y1*|---&gt;CS1*     X4----|S2   Y1*|---&gt;CS1*     |
 |        |     Y3*|  |   |     X5----|S3   Y2*|---&gt;CS1*     X5----|S3   Y2*|nc  SRAMs    |
 &#39;--------|G2B* Y4*|  |   | SRAMsel---|G1*  Y3*|---&gt;CS1* SRAMsel---|G1*  Y3*|nc  2x 128K  | 
    Gnd---|G2A* Y5*|  |   |           |     Y4*|---&gt;CS1*           |     Y4*|nc[or 8x 32K]|
    +5V---|G1   Y6*|  |   |           |     Y5*|---&gt;CS1*           |     Y5*|nc           |
          |     Y7*|  |   |           |     Y6*|---&gt;CS1*           |     Y6*|nc           |
          +--------+  |   |           |     Y7*|---&gt;CS1*           |     Y7*|nc           |
                      |   |           |     Y8*|---&gt;CS1*           |     Y8*|nc           |
                      |   |           |     Y9*|---&gt;CS1*           |     Y9*|nc           |
                      nc  |           |    Y10*|nc (now Y0*/#2)    |    Y10*|nc           |
          [or 74HC154 #2] |           |    Y11*|nc (now Y1*/#2)    |    Y11*|nc           |
                          |           |    Y12*|nc                 |    Y12*|nc           |
                          |           |    Y13*|nc [or 32x 32K]    |    Y13*|nc           |
                          |           |    Y14*|nc                 |    Y14*|nc           |
                          &#39;-----------|G2* Y15*|nc             ,---|G2* Y15*|nc           |
                                      +--------+               |   +--------+             |
                                                               &#39;--------------------------&#39;
This way of counting CRU bit may seem a bit overcomplicated. After all,
why not have a different CRU address for each "drive"? This can actually
be done, thanks to two SPDT switches that are place on the inputs of the
flip-flop (marked as ? in the above schematic). The switches allow to
use a Phoenix-modified Ramdisk in a TI-99/4A system, that always uses 16
bits to access a Ramdisk, by having a different CRU address for each
partition. In this case there is no need to limit the memory to 800K +
256K: each drive could comprise up to 16 128K chips, but the extra
memory won't be accessible from a Geneve.

When in "Geneve" position, the switches connect the flip-flop to Y0\*
and Y1\* of the 74LS138 \#2, as shown above. When in the "TI-99/4A"
position, the switches connect the flip-flop to the CRU address
selection lines: the output of the DIP-switch and pin Y4\* (or Y6\*) of
the 74LS138 \#1. Note that in this case the address on the DIP switch
must be different from the hardwired one.
|         |                  |                  |
|---------|------------------|------------------|
| CRU bit | 32K chips        | 128K chips       |
| 1       | SRAM: A12        | SRAM: A12        |
| 2       | SRAM: A11        | SRAM: A11        |
| 3       | SRAM: A14        | SRAM: A14        |
| 4       | SRAM: A13        | SRAM: A13        |
| 5       | 74HC154: S0 (X2) | 74HC154: S0 (X2) |
| 6       | 74HC154: S1 (X3) | SRAM: A16        |
| 7       | 74HC154: S2 (X4) | SRAM: A15        |
| 8       | 74HC154: S3 (X5) | 74HC154: S1 (X3) |
| 9       | 74LS138: S1 (X1) | 74HC154: S2 (X4) |
| 10      | 74LS138: S1 (X1) | 74HC154: S3 (X5) |
| 11      | not used         | 74LS138: S0 (X0) |
| 12      | not used         | 74LS138: S1 (X1) |
| 13      | not used         | not used         |

As far as software is concerned, the Phoenix modification requires a
small program by J. Schoeder, RAMDOS, that patches the Geneve's MDOS so
that it can see the Horizon Ramdisk. Each time RAMDOS is run, it toggles
between "boot disk" and "ramdisk", but does not allow for simultaneous
use of the two partitions.

####  Memory expansion option

As an option, it is possible to install an extra 32 Kbytes of memory on
the Ramdisk board, to be used as a memory expansion. This was a small
kit designed by John Guion and Bud Mills. It consists in a 74LS08 4xAND
gates, a 32K SRAM chip and two diodes (1N34A).

The 74LS08 must be piggy-backed on top of the CRU address decoder
74LS138, but that's just for power-supply: all pins but ground and Vcc
are bent out. By contrast, the SRAM comes on top of the 6264 SRAM chip
that contains the DSRs, with all pins (but Vcc, CS1\*, A13 and A14)
connected to those of the 6264.

The two diodes are added to the RDBENA\* selection circuit, and to the
74LS145 data bus buffer selection circuit respectively. All other
connections are made with thin wires to the bent out pins of the 74LS08.

           +--------+                                                  +--------+
     A2&gt;---|S0   Y0*|                                           A15&gt;---|A0    D0|---&lt;D5
     A1&gt;---|S1   Y1*|---------------------,                     A14&gt;---|A1    D1|---&lt;D6
     A0&gt;---|S2   Y2*|---Ramdisk sel       |                     A13&gt;---|A2    D2|---&lt;D7
           |     Y3*|---Rambo sel         |                     A12&gt;---|A3    D3|---&lt;D0
 MEMEN*&gt;---|G2B* Y4*|                     |                     A11&gt;---|A4    D4|---&lt;D1 
     Gnd---|G2A* Y5*|------,              |                     A10&gt;---|A5    D5|---&lt;D2
+5V--www-+-|G1   Y7*|---+--==|)--+---,    | 74LS08 (new)         A9&gt;---|A6    D6|---&lt;D3
         | |     Y6*|---&#39;=|)---+-|---=|)--=|)---------------,    A8&gt;---|A7    D7|---&lt;D4
Gnd--o o-&#39; +--------+          | &#39;--------------------------|----------|A13     |
                               &#39;----------------------------|----------|A14     |
                                                            |    A7&gt;---|A8      |
                         2 existing diodes----+------|&gt;|----+    A6&gt;---|A9    W*|---&lt;WE*
               RDBENA*&gt;-----------------------&#39;  1N34A (new)|    A5&gt;---|A10     |
                                                            |    A4&gt;---|A11   G*|---Gnd
                        2 existing diodes-----+------|&gt;|----+    A3&gt;---|A12     |
            74LS245 G*------------------------+  1N34A (new)|          |     Vcc|---+5V
                                  +5V---www---&#39;             &#39;----------|CS1*    |
                                                                       +--------+ 

The existing 74LS138 decoder divides the memory space in chunks of
\>2000 bytes. Its output Y1\* is active (low) during memory operations
in the range \>2000-3FFF (low memory expansion). Similarly, Y5\*, Y6\*
and Y7\* reacts to \>A000-BFFF, \>C000-DFFF and \>E000-FFFF
respectively.

The new 74LS08 chip contains four AND gates. They are used to combine
Y1\*, Y5\*, Y6\* and Y7\* so that a memory operation in the range
reserved for the memory expansion card will activate the CS1\* selection
pin of the newly installed 62256 SRAM. The same selection signal is used
to activate the RDBENA\* line (that enables the data bus buffers in the
connection cable) and the existing 74LS245 (the data bus buffer on the
Ramdisk card). Two diodes are used to combine this signal with the
existing ones, in a "wired and" fashion.

All data and address lines of the 62256 are connected to the
corresponding pins on the 6264 on which it is piggy-backed. There are
however two extra address lines to take care of. One is connected to the
Y6\* AND Y7\* signal line, the other to the Y5\* AND Y7\* signal line,
effectively decoding the 32K address space of the memory expansion:

    >2000: A13=H A14=H
    >A000: A13=L A14=H
    >C000: A13=H A14=L
    >E000: A13=L A14=L

Finally, note that the newly installed SRAM takes its power from the
regular +5V, not the battery-backed one used by the 6264. This is
because the memory expansion is not supposed to retain its content when
the power is off.

###  The RAMBO board

This is a tiny board to be piggy-backed on top of the Ramdisk board. It
allows the SRAM memory to be paged in the area \>6000-7FFF, under
control of the CRU. By the way, RAMBO stands for RAM Block Operator. It
was designed by Ron Walters, John Guion and Gary Bowser. Their address
used to be:

Oasis Pensive Abacutors

432 Jarvis Street, suite 502

Toronto, Ontario M4Y-2H3

Unfortunately, this address is no longer valid...

To install the RAMBO board, a 74LS156 has to be removed from the Ramdisk
PCB. A replacement 74LS156 is present on the RAMBO board, together with
a PAL (programmable array logic) that performs all the selection
operations that would otherwise require several TTL chips. The advantage
is, of course, that it makes the RAMBO board smaller, but it also means
that you cannot build your own board: you need a PAL programmer and you
also need to know the fuse map to burn into the PAL.

Installing the RAMBO board, also implies redirecting CRU bits 1, 2 to
the board. This is done by bending out these two pins and connecting
them (together with the socket they were in) to the RAMBO board with
thin wires. Those going to the socket holes must be connected on the
back side of the board, which requires drilling a small hole in the PCB
to fit them through. Nothing too difficult though. This will allow to
change the value of CRU bits 1 and 2 while RAMBO mode is on, which helps
preventing RAMBO pages to overlap with Ramdisk pages.

In addition, two other wires connect the RAMBO board to CRU bit 15
(RAMBO selection bit, not used by the Ramdisk) and to output Y3\* of the
74LS138 address decoder (addresses \>6000-7FFF, connection labelled
"Rambo sel" in the schematics).

I'm not providing a schematic here. It would be pointless since all the
work is done by the PAL and I have no way to know what has been
programmed in it.

##   Software

### Ramdisk Operating System

Since the software is installed in SRAM, it can be endlessly improved by
its creators John Johnston and Michael Ballman. It is called ROS, for
Ramdisk Operating System. I have version 8.14 and I can tell you it is a
little jewel and a delight for an assembly programer to look at. A lot
of nifty features have been provided on top of the basic functionallity
that you would expect from a Ramdisk. A commented disassembly of ROS,
version 8.14B can be found on my [download](download.md) page.

The ROS is installed in the 6264 SRAM chip that maps at \>4000-57FF, and
in the first SRAM page that maps at \>5800-5FFF. Any user-defined DSR
and ISR and the RAMBO software share the second SRAM page.

####  DSRs

First things first: the Ramdisks appears to the user as 9 drives that
can have any number from DSK1 to DSK9 and from DSKA to DSKZ. These
virtual drives can be formatted as floppy disks would be, i.e. either
contain 360 "sectors" of 256 bytes (SS/SD disk), or 720 sectors (SS/DD
or DS/SD), or 1440 sectors (DS/DD).

Provision is made for the user to create an extra DSR, with any name
upto 8 characters in length. This DSR will be installed in SRAM page
1.

All regular file operations are available on these vitual drives. In
addition, four extra operations are possible:

Opcode **\>08** , scratch record, is now valid. It can be used to remove
a record in the middle of a file and shift all the following ones
accordingly. It only works on FIXED files, that have previously been
opened.

Opcode **\>0A**, assembly-program, loads a "program" file containing an
assembly program in EA5 format and launched it. If necessary, additional
files can be loaded automatically, according to the EA5 format.

The address where the program is to start can be altered by specifying
an alternate address in the PAB, byte 2+3:
\>0000 = use the address specified in the first file
\>FFFF = load but don't run,
other = start at this address.

PAB bytes 6+7 can be used to specify a maximal file length. If it's
\>0000, the file length won't be checked.

Opcode **\>0B**, Basic-program, enters Extended Basic, then loads and
runs a TI-Basic or Extended Basic program. The file type can either be
"program" or "internal, variable 254". It requires the Extended Basic
cartridge since what it does is to hook the interrupt service routine,
launch Extended Basic, wait till it's about to load the DSK1.LOAD file
and change the name of the file for the one you specified.

Opcode **\>0C**, cartridge, loads a "program" file containing a GRAM
memory dump in your GRAM-card. It is designed to work with the Horizon
P-code card, but can easily be addapted to other GRAM card.

Other differences with the TI standards include:

It is not necessary to specify all the file attributes (display, fixed,
etc) if the file already exists in the Ramdisk.
Each opcode returns the file status in PAB byte 9 (not just opcode \>09,
Status).
If you add \>40 to the opcode, VDP memory will not be used as a data
buffer. Instead, the address specified in PAB bytes 2-3 is considered as
a cpu RAM address. This allows for much faster data transfer. Of course,
your program has to be written so as to take advantage of this feature,
which is therefore only available to assembly programmers.

####  Subprograms

The standard disk controller subprograms \>10 through \>15 are
implemented as in the TI floppy disk controller, with a few differences:

Subprograms \>16 and FILES are not implemented since the ROS does not
need a VDP buffer to store disk data: it always allows you to use 16
files. It will however update the drive number and the name of the last
file accesses in the area reserved by the floppy disk controller in VDP
RAM. A little bug here: the ROS does this even if you don't have a
floppy controller installed, which causes weird things to happend on the
screen...
The drive number specified in \>834C accepts values from \>11 to \>2A to
accomodate drives DSKA through DSKZ (i.e. the drive letter minus '0',
ascii \>30).
You can add \>80 to the drive number (in \>834C) to use a CPU memory
buffer instead of VDP memory.
The subprogram list is appended at the end of the DSR list. Which means
every subprogram can be called as a DSR, with file-oriented functions
(e.g. DELETE AF). This is not advisable with subprograms \>10 to \>16
though, as they need parameters in the scratch-pad, but Ramdisk-specific
subprograms all support this type of calling mechanism.
Ramdisk specific subprograms were added. They are meant to be called
from Basic but parameters are not passed in the Basic way (i.e. in
brackets). Rather, they are appended after the program name with a dot
inbetween (e.g. `CALL WO.8` will write-protect DSK8)

**WO.x** Write protection on, x is the drive number: 1-9 and A-Z are
valid values. The drive will behave as if it contained a write-protected
floppy.

**WF.x** Write protection off, x is the drive number, as above.

**AO** Auto-start on. This cause the ramdisk power-up routine to launch
a program that can be selected with the CFG utility.

**AF** Auto-start off. Disables the auto-start feature (but still
remember the selected program).

**RD.o.n** Rename drive. o is the old drive letter, n is the new one.

**LD.x.name** Loads a program directly (That's right, assembly in
TI-Basic!). x is the drive number, "name" is the filename. This must be
a "program" file that can be loaded by either opcode \>0A (assembly),
\>0B (basic) or \>0C (cartridge).

**\>B0** Selects a RAMBO bank. This subprogram must be called from
assembly. It actually consists in 3 subfunctions, to be distinguished
with a parameter passed in byte \>834C-834D. All return an error flag in
\>834E-834F: \>0000 means no error, \>FFFF means something went wrong
(e.g. the page number is too high, or the page is not available).


Ret
Total number of pages
&gt;0000
Error flag
pages for all Ramdisks in the system.
Ret
CRU address of the Ramdisk
CRU value to select this page
Error code
Also turns Gramcard and cartridges off.
Ret
(unchanged)
&gt;FFFF
&gt;0000

In addition, there is a possibility for the user to install upto nine
**custom-made subprograms** (that can also be called like DSRs). Each
will cause a program file with that name to be loaded and run from the
first drive defined. These file can be assembly language, Basic,
Extended Basic or GPL cartridge file.

####  Power-up

There are two awfully complicated interrupt routines in ROS 8.14B. I
think they are meant for compatibilty with other hardware marketed by
WHT (the P-Gram card for instance). They also take into account the fact
that some user may have more than one Ramdisk installed.

The first power-up routine installs a peripheral card scanning routine
in the high memory expansion, at addresses \>A340-A3EC. This is
necessary to scan for existing Gramcards, but effectively overwrites
anything that was in this area of the memory expansion. Currently, this
routine only recognizes P-Gram cards, but it can easily be adapted to
search for other cards.

Then provision is made to run a user-defined power-up routine. This
routine is not part of the linked list of power-up routines, instead it
is called by the first power-up routine of the respective Ramdisk.

The routine in the memory expansion then snatches control away from the
console GROMs and scans all other cards for power-up routines. It calls
them all in order, except for Ramdisk power-up routines that are saved
on a stack and executed afterwards (to run user-defined routines).
The second power-up routine launches a program from the Ramdisk, unless
the \ key is pressed. This has to be the first user-defined
subprogram in the list. The AO and AF subprograms respectively install
and remove the second power-up routine from the linked list.

This is done by loading a second scanning routine at addresses
\>A050-A1EC, that scans all Ramdisks whose addresses were placed on the
stack, and calls the first user-defined subprogram in each (if
auto-start is on for this Ramdisk). This causes a program to be run,
just like subprogram LD would.

If the program returns, scanning continues with other Ramdisks. If all
such programs return, the routine displays a copyright screen and
returns control to the console.

####  ISR

The ROS does not contain any interrupt service routine, but provision is
made for the user to install one. Note however that the Ramdisk cannot
generate interrupts at the hardware level, so such an ISR can only
intercept interrupts generated by another card (provided the Ramdisk CRU
base it less than that of the card).

####  Personnal additions

It's hard to improve on such a good software. But I still wrote two
patches that I found usefull.

[gk.txt](gk.txt) This one makes RAMBO mode compatible with the german
128K Gram-Karte, instead of the P-Gram+ card.
[vnb.txt](vnb.txt) This one causes the ROS to insert and automatically
update version numbers for each file. A "second best" solution short of
a time-stamp.

###  Other software

Additional software is provided on a companion disk.

#### MEMTEST and MEGTEST

These are testing utilities, to be used after you built your own board
and make sure all memories are ok, or to locate possible hardware
problems. MEMTEST is for the old model of Ramdisk, MEGTEST is for the
new ones.

#### CFG

This is a most usefull program that allows you to configure the Ramdisk
Operating System.

It lets you define upto nine drives, with DSR names in the range
DSK1-DSK9 and DSKA-DSKZ. You can set a name for the virtual floppy in
this drive, define its size and whether is is protected or not. You can
delete or change the size of the last drive in the list only, but all
can be renamed.

You can define upto nine subprograms (i.e. CALLs) that correspond to
assembly language programs stored in the first drive you defined.
Calling this subprogram/DSR results in loading and running the
respective program, which can be assembly, Basic or GPL. The first in
the list is the one launched by the auto-start feature at power-up time.

You can load an uncompressed assembly DF80 tagged-object file containing
a custom-made DSR, a custom-madde power-up routine and/or a custom-made
interrupt service routine. These will become part of the ROS.

You can save the ROS on disk, or load it back from disk (keeping the
current drive definitions or not, as you wish).

You can specify a character pattern file, and default colors for screen
and character color (in text mode). These will be used when loading an
assembly file with opcode \>0A or the LD subprogram (or during
auto-start).

#### Demo programs

Finally, there are a number of demo programs, such as a a MENU program
to be launched at startup time, and game that makes use of RAMBO banks.

There are also modified versions of widely used programs, that will
accept drive numbers in the range DSKA-DSKZ (DiskU, Archiever III, etc).

Revision 1. 4/12/00 Preliminary

Revision 2. 4/15/00 OK to release
Revision 3. 1/15/02 Elaborated on power options
Revision 4. 7/10/02 Added rev 1-91

Revision 5. 11/15/02 Added diode names in schematics
[Back to the TI-99/4A Tech Pages](titechpages.md)
