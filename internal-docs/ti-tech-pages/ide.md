# IDE interface card (prototype)

Click [here](ide2.md) for the "production" version of this card.

Building a hard drive controller card for my TI-99/4A has long been a
dream of mines. Unfortunately, it was far beyond my abilities. And yet,
here it is: I finally made it! Not that I suddenly became more
knowledgeable in electronics, but because the IDE standard made things
so much easier. You see, with IDE (Integrated Drive Electronics, or is
it Intelligent Drive Electronics?) the controller is part of the hard
disk. All I had to do was to design an adapter to make the PE-box bus
look like a PC bus. And that's fairly easy. It would even be trivial if
Texas Instruments had not crippled the PE-box by multiplexing the
TMS9900 16-bit bus to 2 times 8 bits. As it is, we'll need a
**demultiplexer**, but that's not too complicated: just a few TTL chips.

The nice thing with IDE is that these days everybody swears by
gigabytes-wide SCSI drives, so it is really easy to find very cheap
"small" IDE drives. I bought a 540 Megs drive for \$18, to give you an
idea. And 540 Megs is equivalent to about 6000 floppies! This interface
card supports **two IDE drives**. It may also work with CD-ROMs but
you'll have to write your own control program.

While I was at it, I also included a 32K to 512K **SRAM memory** chip on
the card. It serves to hold the DSRs and the buffers for opened files.
The memory is also available to the user in the area \>6000-7FFF,
according to the RAMBO protocol introduced with the Horizon Ramdisk.

Last but not least, the card comprises a **real-time clock**. This
allows the software to time-stamp the files, which is something I always
missed with the TI-99/4A. And the clock chip I selected also contains 4K
of battery-backed memory, so we can stuff the DSRs in there, to be
transfered in the SRAM at power-up time. This saves us the trouble of
having a battery-backed SRAM.

I have no intention to market that card, but if you want to build your
own you'll find all the necessary instructions in this page. That's
fairly easy to do too: after all I did it and I'm no EE, nor especially
good at handywork... You can also download the operating system (and its
instruction manual), tentatively named [IDEAL](ideal.md) for "IDE
Access Layer".

Here is an annotated [picture](idecard.md) of the prototype board I
built. Note that this point-to-point soldering on a pre-etched perf
board is a bit messy: this awful rat's nest creates quite a bit of
resonnance between wires. I have been plagued by glitches that generate
spurious read operations when the ultra-fast IDE controller mistakes
them for a data access. I was able to control them by placing a
capacitor on the MEMEN\* line, but still I strongly recommend that you
etch a PCB...

Here is a detailed [schematic](ide.zip). To view it you will need a
program called PC-Trace which can be freely downloaded from
[www.eesoft.com](http://www.eesoft.com) (it will also let you create a
PCB, but to print it out you'll have to fork out a registration fee).
The specific sections of the circuit are described in details further
down this page.

Please, let me know if you built such a card and if you're happy with
it. Suggestions for improvements, and software bug reports are always
appreciated.

height="112" alt="2001 Mauk Award" />I'm proud to announce that this
card was granted the 2001 Edgar Mauk award, in the category "hardware".

#### Legal disclaimer. Read this first.

The contents of this webpage are for educative purposes only. I'm not an
electrical engineer, I have little knowledge of electronics and
therefore cannot guaranty that the device described in this document as
an IDE interface card will function properly. In fact, it probably
won't. It may even cause damage to your computer. And quite possibly
catch on fire and burn your house to ashes, sending toxic fumes in the
whole neighbourhood. Actually, it may even kill you. So if you know
where your interest it, don't build it! Don't even think of it.
Furthermore, building the aforementionned device may constitute a
copyright violation, an infringement on FCC regulations, a federal crime
or whatever it is called in the country you live in. You have been
warned! By reading this page further and/or building the electronic
device described herein, you agree on taking whole legal and moral
responsability for any disapointment, loss of data, damage, accident,
catastrophe, or apocalypse that this device may directly or indirectly
cause or favor. And if you were thinking of suing me, forget it. I may
have an MD but I'm only a scientist: I don't have any money.

The design detailed below is copyrighted by me, Thierry Nouspikel, till
the end of time or 50 years after my death, whichever occurs first. I
grant you the right (but not the exclusive rights) to produce and even
market as many of these as you want, as long as you understand that I
take no responsability for it. If you market them, be sure to include my
copyright and a link to the present webpage.

**Hardware
  **
[Components
](#Components)**Card circuitery  **
[Power supply
](#Power)[Bus buffering
](#Bus%20buffers)[CRU logic
](#CRU%20logic)[Address mapping
](#Addressing)[SRAM access
](#SRAM%20logic)[Clock access
](#Clock%20logic)[IDE port access
](#IDE%20logic)**RTC-65271  **
[Pinouts
](#RTC%20pinout)[Batteries
](#Batteries)[Registers
](#RTC%20registers)[XRAM
](#XRAM)[Time & date
](#Time%20and%20date)[Alarms
](#alarms)[Square wave
](#square%20wave)[Timing diagrams
](#Timing%20diagrams)[Electrical characteristics
](#Electrical)**IDE interface  **
[Cable pinouts
](#IDE%20pinouts)[Registers
](#IDE%20registers)[Commands
](#IDE%20commands)[Operating the IDE controller
](#Operating%20the%20IDE%20controller)[LBA vs CSH
](#LBA%20vs%20CSH)[DMA
](#DMA)[Drives](#Drive%20considerations)
**Software**

[CRU map
](#CRU%20map)[Memory map
](#Memory%20map)[Low-level access
](#Low-level)[High-level access](#High%20level)

------------------------------------------------------------------------

Hardware

#### Components required

As I said, I'm not selling this board, not even as a kit. This table is
only meant to give you an idea of what it would cost to build the card.


(or equivalent)
$12.00

Total: components \$20-\$30 depending on the SRAM. Plus board + hard
drive. About \$60. Could be cheaper if you look around for a good deal
on the expensive parts (SRAM, board and drive).

####  Souces

Digi-key Corporation (www.digikey.com)

BG-Micro (www.bgmicro.com)

All Electronics Corporation (www.allelectronics.com)

Fry's Electronics

Haltek surplus (out of business, but may come back...)

Halted surplus, aka HSC (www.halted.com)

###  IDE card circuitery

#### Power supply

                   78L05                          All chips
                +----------+      +5V              +-----+
+8V-----+-------|Vin   Vout|---+-----... ---+------|Vcc  |    
        |       |   Ref    |   |            |      |     |
        = 22 uF +----------+   = 0.1 uF     =0.1uF |     |
        |            |         |            |      |     |
       Gnd           Gnd       Gnd     Gnd--+------|Vss  |
                                                   +-----+

Nothing special here: the 78L05 voltage regulator can supply 1 mAmp at
+5 volts. Make sure you use a large heat sink: it's gonna heat a lot.

To filter transients, each chip should have a 100 nF cap connected
accross its supply pins, as close to the chip as possible. Ideally, the
+5 volts current should meet the cap before it meets the Vcc pin of the
chip.

####  Bus buffering

As recommended by Texas Instruments, all lines to/from the peripheral
bus are buffered on-card. This is done with three 74LS244 for the
address bus (16 bits) and some of the control lines: MEMEN\*, CRUCLK,
RESET\*, DBIN, and WE\*.

The data bus, of course needs a bidirectional 74LS245 buffer. The
DIRection is controlled by the DBIN signal from the peripheral bus, the
ENable pin by the CardSel\* signal generated by the card (see below).

The selection signal is also connected via a 74LS125 to the RDBENA\*
line of the peripheral bus to enable the drivers in the connection card
and the console-end of the cable. A 74LS125 is necessary, so as not to
hold the line high when we are not using it, as another card may need
it. The CardSel\* signal enables the '125 which input is connected to
the ground, the rest of the time, the 74LS125 is in high-impedance (i.e.
isolated) state.

The only line that is not buffered in the CRUIN line that comes directly
from the 74LS251 (this chip has a 3-state output anyhow).

The following schematic combines the above buffering circuits and the
CRU logic described in the next chapter.

           +-----+   
     A0&gt;---|     |---&gt;A0   
      .    |     |    .    
      .    |     |    .
      .    |     |    . 
      .    |     |    .
      .    |     |    .
     A7&gt;---|     |---&gt;A7        
      Gnd--|EN*  |                             CRU logic
           +-----+     
            &#39;244      
           +-----+                        270 Ohm
     A8&gt;---|     |---&gt;A8              Gnd---WWW---,                  IDE port
      .    |     |    .                           |    ,------------&lt;IRQ*
      .    |     |    .                      LED  ^    | 
      .    |     |    .                   &#39;259    |    |      &#39;251
      .    |     |    .                 +------+  |    |    +------+      
      .    |     |    .    ,------------|RST   |  |    |    |      |
     A15&gt;--|     |---&gt;A15  |    A12&gt;----|S2  Q0|--&#39;    &#39;----|D0  S2|----&lt;A12 
      Gnd--|EN*  |         |    A13&gt;----|S1  Q1|---     ----|D1  S1|----&lt;A13 
           +-----+         |    A14&gt;----|S0  Q2|------------|D2  S0|----&lt;A14
            &#39;244           |            |    Q3|------------|D3    |  
           +-----+         |    A15&gt;----|D   Q4|------------|D4   Q|----&gt;CRUIN
 RESET*&gt;---|     |---------&#39;   &#39;32      |    Q5|------------|D5    | 
CRUCLK*&gt;---|     |--------------=)&gt;-----|EN  Q6|------------|D6    |  
    WE*&gt;---|     |---&gt;WE*      |        |    Q7|------------|D7  EN|---,   
  MEMEN&gt;---|     |---&gt;MEMEN*   |        +------+            +------+   |   
CLKOUT*&gt;---|     |             &#39;-----------------------------,         | 
           |     |                Main       +------+  &#39;04   |         | 
   DBIN&gt;---|     |---,           switch      |   A=B|--|&gt;o---+---------&#39;     
      Gnd--|EN*  |   |       A3---o_o-------&gt;|A=B   |          10K         
           +-----+   |      Gnd---o   A4&gt;---&gt;|A1  B1|------+---WWW---+-+5V 
            &#39;245     |                A5&gt;---&gt;|A2  B2|-----+|---WWW---+    
           +-----+   |                A6&gt;---&gt;|A3  B3|----+||---WWW---+  
           |  DIR|&lt;--&#39;                A7&gt;---&gt;|A4  B4|---+|||---WWW---&#39;  
    D0&lt;---&gt;|     |&lt;--&gt; D0              Gnd--&gt;|A&lt;B   |   ||||             
     .     |     |      .              +5V--&gt;|A&gt;B   |   oooo  DIP        
     .     |     |      .                    +------+   oooo  switch 
     .     |     |      .                      &#39;85      ||||
     .     |     |      .                               ++++-Gnd
     .     |     |      .                                                   
     .     |     |      .                                                   
    D7&lt;---&gt;|     |&lt;--&gt; D7                                                   
           |  OE*|&lt;------,                                                      
           +-----+       +----------&lt; CardSel*                            
                         |                                                  
RDBENA*&lt;-----------------&lt;|--Gnd
                       &#39;125  

####  CRU logic

The CRU logic is pretty standard. A 74LS85 comparator is used to compare
the CRU address to a value set with a 4-DIP-switch (actually, it's a
7-DIP-switch, the other switches are used elsewhere or reserved). Line
A3 is connected to the A=B input and must be high to enable the output,
which allows to discriminate the console CRU space (\>0000-0FFF: A3 =
low). An optional SPDT switch can be used to toggle between the A3 line
and ground. When the A=B input is connected to ground, it shuts off the
card and prevents it from answering any CRU request. This may come handy
when the DSRs get messed up and prevent the TI-99/4A from booting
correctly...

The A=B output of the 74LS85 is active high. It must be inverted so as
to control the 74LS251 that performs CRU input. The inverted signal must
be further combined with CRUCLK\* to enable the 74LS259 that accepts CRU
output from the peripheral bus. This is necessary so that CRU input
operations don't alter the content of the '259 (the opposite circuit is
not needed on the '251 because the TMS9900 CPU just ignores the CRUIN
line during CRU output operations).

Most CRU bits are reflected on themselves by connecting each output of
the '259 to the corresponding input of the '251. This is not true
however for bits 0 and 1. Bit 0 ouput turns on a yellow LED and provides
a selection signal for the addressing logic. In input mode, it reads the
status of the IRQ line on the IDE connector. Bit 1 will be discussed
below.

####  Address decoding

          +-------+                          +------+                 +------+
    Gnd---|G* A=B*|--------------------------|S0 Y0*|--+--------------|G*    |
    A7&gt;---|A0   B0|--,        Bit0---,       |   Y1*|--|-+--&gt;SRAMsel* |   Y0*|------XRAMsel*
    A6&gt;---|A1   B1|--+  A2--+--------==)&gt;o---|S1    |  | |            |   Y1*|------RTCsel*
    A5&gt;---|A2   B2|--+      &#39;---|&gt;o--, &#39;02   |      |  | |     A10&gt;---|S0    | 
    A4&gt;---|A3   B3|--+        Bit4---==)&gt;o---|S2    |  | |      A9&gt;---|S1 Y2*|---+--------&gt;CS1Fx 
    A3&gt;---|A4   B4|--+--Gnd +5V--||--------, |      |  | | &#39;08        |   Y3*|---|-+------&gt;CS3Fx 
    A2&gt;---|A5   B5|--+         1nF MEMEN*&gt;-+-|G2A*  |  &#39;-==|)--,      |      |   | |
    Gnd---|A6   B6|--+                 A0&gt;---|G2B*  |          |      |      |   &#39;-==|)-+-IDEsel*
Bit1out---|A7   B7|----+---o o---Gnd   A1&gt;---|G1    |      CardSel*   |      |      &#39;08 |
          +-------+    |   10K               +------+                 +------+    270   V LED
            Bit1in-----+---WWW--- +5V                                       +5V---WWW---&#39;

The heart of the address decoding logic is the 74LS138 3-to-8 decoder,
which I am using as a 1-to-2 decoder with multiple enabling lines. It is
enabled by MEMEN\*, A0 and A1, i.e. for memory operations in the range
\>4000-7FFF. Line A2 is combined with CRU bits 0 and 4, so as to select
outputs Y0\* and Y1\* in two cases:

When CRU bit 0 is high and A2 is low (address range \>4000-5FFF, i.e.
card ROM space)

When CRU bit 4 is high and A2 is high (address range \>6000-7FFF, for
RAMBO banks)

The 74LS688 is an 8-bit comparator whose A=B\* outpout is active low
when the address is in the range \>4000-40FF and CRU bit 1 matches the
DIP-switch. Line A2 is included in the comparison, so that the '688
never gets active in the RAMBO space. The DIP-switch allows you to
decide whether the SRAM or the clock RAM should be present at power-up
time. As CRU bits are reset to 0 upon power-up, the clock RAM is
selected when the switch is closed, the SRAM when the switch is open.
The status of this switch can be read with CRU bit 1, so the software
always know how to switch the SRAM on (when output does not match input)
or off (when bit 1 output mirrors its input).

The A=B\* output of the '688 selects either Y0\* or Y1\* of the '138.
Both are combined with an AND gate to produce the CardSel\* signal that
is used to enable the data bus. Y1\* activates the SRAM, Y0\* activates
a 74LS139 decoder that decides which of the 4 extra devices is accessed.
Lines A9 and A10 are used for this selection:

Addresses \>4000-401F correspond to the clock extended RAM (128 pages of
32 bytes)

Addresses \>4020-403F access the real-time-clock registers (see
[below](#Clock%20logic))

Addresses \>4040-405F access the first set of registers in the IDE
controller

Addresses \>4060-407F access the second set.

Addresses \>4080-40FF just repeat the above pattern (only the RTC uses
them differently).

Note that the two IDE access lines are combined through an AND gate, to
provide an IDEsel\* signal for the bus demultiplexer. Just for the fun
of it, this signals also shines a red LED, that lets you know when the
drive is accessed. Both this LED and the yellow LED triggered by CRU bit
0 should be mounted so as to shine through the tiny window, in front of
the PE-Box.

You may wonder why I have a 1 nF cap on the Memen\* line. This was an
attempt to filter out the glitches that caused spurious opertations from
the IDE controller. I suspect that the glitches are due to my
point-to-point soldering, and that the cap would not be necessary with a
properly etched PCB...

####   SRAM access

                                             ||||||||
                                   74LS373     10 K        SRAM
                                   +------+  ||||||||   +----------+
                            A14---&gt;|D0  Q0|--++++++++--&gt;|A13     D7|&lt;---&gt;D0
                             .     |      |             | .        |     .
                             .     |      |             | .        |     .
                             .     |      |             | .        |     .
                            A7----&gt;|D7  Q7|------------&gt;|An      D0|&lt;---&gt;D7
                                   |      |      A3----&gt;|A12       |
SRAMsel*----, &#39;32        &#39;02       |      |       .     | .        |
WE*---------==)&gt;-------==)&gt;o------&gt;|LE    |       .     | .        |  &#39;32  ,----Bit5
CRU bit 2---|&gt;o--------&#39;           |      |       .     | .     WE*|---&lt;(==----&lt;WE*
            &#39;04  &#39;32    &#39;02        |      |      A15---&gt;|A0        |
         A3---==)&gt;----==)&gt;o-------&gt;|OE*   |             |       CS*|----SRAMsel
         A2---&#39;       |            +------+             +----------+
  CRU bit 3-----------&#39;

The SRAM data lines are connected to the data bus. Note that Texas
Instruments numbers the bits the opposite way to everyone else: D0 (or
A0) is the most significant bit. That's why I connected D0 to D7, D1 to
D6, etc. This is not critical for the SRAM, but it is for all other
devices!

The SRAM address lines are driven (in part) by the address bus, lines A3
through A15. This provides us with a \>2000-byte access window. As the
SRAM is likely much larger than 8K, it will have extra address lines.
These serve as "page selection" lines and are set by the 74LS373 latch.
The exact number of lines depends on the size of the SRAM, the '373 can
latch 8 bits, which allows us to use a 2 Megabyte SRAM (if such a
beastie exists...).

The '373 latches the address bus, lines A7 through A14, according to a
convention set by the cartridges ROMs. The latching occurs when one
writes to the SRAM while CRU bit 3 is set to one (which it isn't at
power-up time). To this end, a selection circuit combines SRAMSel\* and
WE\* via an OR gate (active low when both are active) and combines the
resulting signal with the inverse of CRU bit 2, via a NOR gate.

An additional and critical feature is the little circuit that controls
the OE\* (output enable) pin of the '373. When this pin is high, the
outputs of the '373 are in high impedance state, so the corresponding
address lines will reflect a default state. The eight 10K resistors
grounding these pins ensure that page 0 will be selected in this
situation. If you wanted to, you could use a DIP-switch (or a CRU bit)
to connect the resistors either to ground or to +5V: this would give the
user the choice between two default pages.

These outputs are only disabled when an access occurs in the range
\>4000-4FFF (i.e. A2 and A3 are low), and CRU bit 3 is 0. This is
critical so that a default page, that supposedly contains the DSRs, is
always selected upon power-up and won't be paged off when the user
switches RAMBO pages. Also, it allows routines in the \>4000-4FFF memory
space to switch pages in the \>5000-5FFF and \>6000-7FFF area without
kicking themselves out of memory. The cartridge ROMs do this by having a
copy of the \>6000-6FFF area at the bottom of each page, but wasting all
this memory is kind of a shame! The above hardware trick allow us to
freeze the bottom half of page 0, while still paging the top part.

Finally, the WE\* input pin is controlled by the WE\* line, after due
masking by CRU bit 5. When this bit is 1, it prevents any writing to the
SRAM. This "ROM emulation" feature is usefull to switch banks without
altering the content of the SRAM.

Optionally, you could install a +3V coin battery to back-up the SRAM
when power is off. This requires a low-power SRAM, with a way of putting
the outputs in high-impedance when power is off (either an active-high
CS pin, or an active-low STANDBY\* pin).

####  Clock access

                       +----------+
                A8----&gt;|A5      D7|&lt;---&gt; D0
               A15----&gt;|A4        |      .
               A14----&gt;|A3        |      .
               A13----&gt;|A2        |      .
               A12----&gt;|A1        |      .
               A11----&gt;|A0        |      .
         &#39;125          |        D0|&lt;---&gt; D7
INTRQ*&lt;---&lt;|---Gnd     |          |
          &#39;------------|INTRQ*    |    &#39;04
                       |       RD*|&lt;---o&lt;|---&lt;DBIN
                       |       WR*|&lt;---------&lt;WE*
         RTCsel*------&gt;|RTC*      |
        XRAMsel*------&gt;|XRAM*     |
RESET*---o o------+---&gt;|RESET*    |
             10K  |    +----------+
       +5V--WWW---&#39;

The RTC-65271 chip serves two functions: it contains a real-time clock
(RTC) similar to the standard MC146818 clock found on the PC computers.
In addition, it contains 4 Kbytes of battery-backed RAM, accessible as
128 pages of 32 bytes. This is a nice feature because it allows us to
store the DSRs in the clock RAM, without the need for a battery-backed
SRAM. At power-up time, the DSRs will be copied into the SRAM, since it
is not very convenient to switch pages every 32 bytes!

The clock data lines must be connected to the data bus in reverse order,
i.e. D7 to D0, D6 to D1, etc. The address lines must be connected as
shown above. Under these conditions, the extended RAM maps at
\>4000-401F, and the page selection register (controlled by pin A5,
address line A8) at \>4080.

The RTC contains 64 registers that are accessible via two ports. First
you write the number of the desired register at \>4030, then you can
read or write to this register at \>4020. (NB: Lines A1-A5 are ignored
upon RTC access, so the remaining addresses map to the same ports).

The WR\* pin is connected to WE\*, the RD\* pin to the inverse of DBIN.
The two chip select lines, RTC\* and XRAM\*, are connected to the
outputs of the 74LS139 in the address selection logic. They select the
Real-Time-Clock or the eXtended-RAM respectively.

Finally, the RTC chip can generate interrupts. These will pull down the
EXTINT\* line of the peripheral bus, via a 74LS125 buffer whose input is
grounded. When the IRQ pin is high, the '125 is in high impedance and
won't affect the EXTINT\* line.

Because interrupts can occur at any time, even when the computer is off,
there is a risk that the system becomes unbootable: as soon as you turn
the console on it sees the interrupt and tries to answer it. But if
IDEAL is not loaded yet, it won't clear the interrupt condition and the
system keeps scanning cards, inbetween each GPL instruction. To prevent
this from happening, the RESET\* line is connected to the RESET\* pin of
the clock chip. This will disable all interrupts when the console is
powered on. This feature can be disabled by opening the DIP switch in
the RESET\* line. For instance, if you have a battery-backed SRAM, you
may be reasonnably sure that IDEAL will always be loaded and ready to
handle interrupts.

####  IDE port access

                                            +------+                       
                                ,-----------|Q0  D0|&lt;---,
                              ,-|-----------|Q1  D1|&lt;---|-,
                            ,-|-|-----------|Q2  D2|&lt;---|-|-,
                          ,-|-|-|-----------|Q3  D3|&lt;---|-|-|-,
                        ,-|-|-|-|-----------|Q4  D4|&lt;---|-|-|-|-,
                       ,|-|-|-|-|-----------|Q5  D5|&lt;---|-|-|-|-|-,
                    ,-|-|-|-|-|-|-----------|Q6  D6|&lt;---|-|-|-|-|-|-,
                  ,-|-|-|-|-|-|-|-----------|Q7  D7|&lt;---|-|-|-|-|-|-|-,
                  | | | | | | | |  ,--------|OE* LE|----|-|-|-|-|-|-|-|---------,
                  | | | | | | | |  |        +------+    | | | | | | | |         |
   Even byte      | | | | | | | |  |          74LS244   | | | | | | | |         |
                  | | | | | | | |  |         +------+   | | | | | | | |         |
                  | | | | | | | +--|--------&gt;|A0  Y0|---+-|-|-|-|-|-|-|---&gt;D15  |
                  | | | | | | +-|--|--------&gt;|A1  Y1|-----+-|-|-|-|-|-|---&gt;D14  |
                  | | | | | +-|-|--|--------&gt;|A2  Y2|-------+-|-|-|-|-|---&gt;D13  |
                  | | | | +-|-|-|--|--------&gt;|A3  Y3|---------+-|-|-|-|---&gt;D12  |
                  | | | +-|-|-|-|--|--------&gt;|A4  Y4|-----------+-|-|-|---&gt;D11  |
                  | | +-|-|-|-|-|--|--------&gt;|A5  Y5|-------------+-|-|---&gt;D10  |
                  | +-|-|-|-|-|-|--|--------&gt;|A6  Y6|---------------+-|---&gt;D9   |
                  +-|-|-|-|-|-|-|--|--------&gt;|A7  Y7|-----------------+---&gt;D8   |
                  | | | | | | | |  |         |G1*G2*|                           |
                  | | | | | | | |  |         +------+                           |
                  | | | | | | | |  |           |  |             IDE connector   |
 PE box           | | | | | | | |  |     ,-----+--&#39;             16-bit bus      |
 8-bit bus        | | | | | | | |  |     |    74LS373                           |
                  | | | | | | | |  |     |    +------+                          |
  D0&lt;-------------|-|-|-|-|-|-|-+--|-----|---&gt;|D0  Q0|---+----------------&gt;D7   |
  D1&lt;-------------|-|-|-|-|-|-+-|--|-----|---&gt;|D1  Q1|---|-+--------------&gt;D6   |
  D2&lt;-------------|-|-|-|-|-+-|-|--|-----|---&gt;|D2  Q2|---|-|-+------------&gt;D5   |
  D3&lt;-------------|-|-|-|-+-|-|-|--|-----|---&gt;|D3  Q3|---|-|-|-+----------&gt;D4   |
  D4&lt;-------------|-|-|-+-|-|-|-|--|-----|---&gt;|D4  Q4|---|-|-|-|-+--------&gt;D3   |
  D5&lt;-------------|-|-+-|-|-|-|-|--|-----|---&gt;|D5  Q5|---|-|-|-|-|-+------&gt;D2   |
  D6&lt;-------------|-+-|-|-|-|-|-|--|-----|---&gt;|D6  Q6|---|-|-|-|-|-|-+----&gt;D1   |
  D7&lt;-------------+-|-|-|-|-|-|-|--|-----|---&gt;|D7  Q7|---|-|-|-|-|-|-|-+--&gt;D0   |
                  | | | | | | | |  |     |    |OE* LE|   | | | | | | | |        |
                  | | | | | | | |  |     |    +------+   | | | | | | | |        |
                  | | | | | | | |  |     &#39;      |   |    | | | | | | | |        |
   Odd byte       | | | | | | | |  |     &#39;------+   |    | | | | | | | |        |
                  | | | | | | | |  | 74LS244    |   |    | | | | | | | |        |
                  | | | | | | | |  | +------+   |   |    | | | | | | | |        |
                  | | | | | | | &#39;--|-|Y0  A0|&lt;--|---|----&#39; | | | | | | |        |
                  | | | | | | &#39;----|-|Y1  A1|&lt;--|---|------&#39; | | | | | |        |
                  | | | | | &#39;------|-|Y2  A2|&lt;--|---|--------&#39; | | | | |        |
                  | | | | &#39;--------|-|Y3  A3|&lt;--|---|----------&#39; | | | |        |
                  | | | &#39;----------|-|Y4  A4|&lt;--|---|------------&#39; | | |        |
                  | | &#39;------------|-|Y5  A5|&lt;--|---|--------------&#39; | |        |
                  | &#39;--------------|-|Y6  A6|&lt;--|---|----------------&#39; |        |
                  &#39;----------------|-|Y7  A7|&lt;--|---|------------------&#39;        |
                                   | |G1*G2*|   |   |                           |
                                   | +------+   |   |        74LS04             |
                                   |   |  |     |   |     ,---o&lt;|---WE*         |
                         74LS139   |   &#39;--+     |   &#39;--(|=----------A15         |
                                   |      |     |                         &#39;04   |
                         +------+  |      |     |    74LS08             ,--|&gt;o--&#39;
               A15&gt;-----&gt;|S0 Y2*|--&#39;      |     | A11---,               |
               DBIN&gt;----&gt;|S1 Y3*|---------+-----|-------=)&gt;-------,     |
                         |   Y1*|-nc            |       &#39;32    ,---=)&gt;--+------&gt;RD*
     IDEsel*&gt;------+----&gt;|G* Y0*|---------------+--------,    |    &#39;32
             LED   |     +------+                 WE*----=)&gt;--|----,              
 +5V---WWW---|&gt;|---&#39;             74LS165                 &#39;32  +----=)&gt;---------&gt;WR*
     220 Ohm                    +------+      Pulses          |   74LS32
                            ,---|P7  Q7|----------------------&#39;
                            +---|P6    |
                +5V---WWW---+---|P5 CK1|&lt;---Phi3*            A12&gt;--------------&gt;A2
                      4.7K  +---|P3 CK2|---Gnd               A13&gt;--------------&gt;A1
                            +---|P1    |                     A14&gt;--------------&gt;A0
                            +---|P0 LD*|&lt;---o&lt;|---Memen*
                            &#39;---|D     |                   Bit0in&lt;-------------&lt;IRQ*
                         Gnd----|P4    |                   Bit7-------|&gt;o------&gt;RESET*
                         Gnd----|P2    |
                                +------+                                          

The IDE port consists in 10 registers, that map as two separate blocks.
The two blocks are enabled by CS1Fx and CS3Fx, controlled by two outputs
of the 74LS139 in the address selection logic (see
[above](#Addressing)). CS1Fx maps at \>4040-405F and CS3Fx maps at
\>4060-407F. The register number is passed to the IDE controller on its
A0-A2 lines, from the address lines A14-A13 (in reverse order because A2
is the msb for the IDE controller).

All registers are 8 bits wide, except the data register that is 16-bit.
For this reason, we must demultiplex the peripheral data bus, and bring
it back to what it was in the console. This is achieved by the circuit
above, that basically consists in two 74LS244 buffers and two 74LS373
latches.

The 74LS139 decoder (the other half of the chip used in the address
selection logic) discriminates between the four operations: Y0\* = write
even byte, Y1\* = write odd byte, Y2\* = read even byte, Y3\* = read odd
byte. The odd byte is always passed first, for both read and write
operations.

For write operations, the odd byte is latched in a 74LS373. Normaly,
Y1\* should drive the "latch enable" pin of this chip, after being
combined with WE\* via an OR gate. Unfortunately we are running out of
OR gates! To avoid having to install an extra 74LS32 chip, I'm using one
of the AND gates left in the 74LS08 instead. The AND gate combines A15
with WE\* , just like the 74LS139 would. Note that LE is an active-high
signal.

The second byte (the even one) is passed directly to the IDE controller,
via a 74LS244 buffer. You could use a 74LS245 if you find its pin layout
more convenient, just make sure the DIR pin is properly set. The buffer
and the 74LS373 latch are enabled together by Y0\* when the TI-99/4A
writes the second byte, so the IDE controller can read a full word.

For read operations, the odd byte is passed to the data bus via a
74LS244, enabled by Y3\*. At the same time, the even byte is latched
into a second 74LS373 until the TI-99/4A is ready to read it. When this
finally occurs, the latch is enabled by Y2\*.

The 74LS165 shift register is necessary because the TI-99/4A does not
provide a "read" pulse to be applied to the RD\* pin of the IDE
controller. I tried to make the whole 2-byte read cycle a single pulse,
but that was too messy: the IDE controller is so fast that it would pick
any glitch on the line and think it was a request for the next data
word. As a result, it was sending lots of words before the TI-99/4A was
ready to read them! To solve this problem I had to modify the
multiplexer (my initial design had only one latch) and to implement a
pulse generator. When Memen\* goes low, the 74LS165 latches the bit
pattern present on P0-P7. At this time, the output Q7 reflects P7. Then,
at each clock pulse of Clkout\* the next bit is passed to Q7. New bits
are filled in by the D pin, but this is not really needed because a
memory operation in the PE box only takes 6 clock cycles. By connecting
P4 and P2 to ground, we are generating two active-low pulses on Q7: one
between clock cycle 2 and 3 for read operation, and one between cycles 4
and 5 for write operations.

The WR\* input of the IDE port is controlled by the WE\* signal, at the
time the even byte is writen (Y0\* is low), and is further truncated by
the second pulse from the 74LS165. The latter may not be strictly
necessary since WE\* is a pulse in itself, but lets play it safe...

The RD\* input is controlled by Y3\* (which is active low while reading
the first byte) combined with the first pulse and A11. Using A11
restricts read operations to the range \>4040-404F and \>4060-406F. This
is required because the TMS9900 CPU performs a read before each write.
The IDE controller does not expect that and may not answer properly to
write operations if they were intermingled with reads (although I did
not try). Masking RD\* with A11 ensures ensures that no read operation
will occur in the range \>4050-405F nor in \>4070-407F, which is where
we'll perform the write operations. The true reads will be done at
\>4040-404F and \>4060-406F. The RD\* pulse is also used to latch the
second byte in a 74LS373 until the TI-99/4A is ready to read it.

Finally, as mentionned above, the IRQ\* line is connected to input A0 of
the 74LS251 chip, so that it can be read with CRU bit 0. CRU bit 7 is
used to perform a hardware reset. It is inverted so that the drive will
power-up when the PE-Box is turned on, even if the console is still off.

The LED is optional. It lights up to indicate disk access and should be
of a different color (and somewhat brighter) than the LED controlled by
bit 0. It should be installed so that both LEDs are visible from the
little window in the PE-box.

------------------------------------------------------------------------

### The RTC-65271

The RTC-65271 is a real cool chip. Basically, it is a real-time clock,
compatible with the MC146818A that you'll find in a PC. The nice thing
is that the chips has its own crystal, and a battery holder: no extra
components needed! In addition, it contains 4 Kbytes of battery-backed
SRAM. This will come handy to store a "boot sequence" that will load the
DSRs from disk at power-up time.

#### Pinout

           +----+--+----+
        A0 |1 o       28| A2
        A1 |2    R    27| A3
     TMODE |3    T    26| Vcc
    TCLOCK |4    C    25| SQW
     STBY* |5         24| A4
        D0 |6    6    23| A5
        D1 |7    5    22| n.c.
        D2 |8    2    21| IRQ*
        D3 |9    7    20| RESET*
        D4 |10   1    19| RD*
        D5 |11        18| n.c.
        D6 |12        17| WR*
        D7 |13        16| XRAM*
       GND |14        15| RTC*
           +------------+

Power supply
**Vcc**: +5V
**GND**: ground

Control signals
**RTC\***: Accessed the real-time clock registers when low (unless
XRAM\* is also low).
**XRAM\***: Acceses the extended RAM when low (has precedence on
RTC\*).
**RD\***: Read data from a register or the XRAM when low.
**WR\***: Latch data into a register/XRAM when low. RD\* and WR\* should
never be low together.
**RESET\***: Resets the clock. When low, clears register C and bits SQW,
PIE, AIE, and UIE in register B. Pin IRQ\* is high impedance and the
data bus is disabled. Caution: should not be used within 300 ms of
power-up. If not used, connect to Vcc.
**STDBY\***: Stand-by.All inputs are inhibited, all outputs are in
high-impedance. If batteries are installed, the clock still operates
normally and the XRAM is maintained intact. The pin has an internal
pull-up resistor, but should still be connected to Vcc if not used.

Address bus
When RTC\* is low: **A0** selects the address register when low, the
data/clock registers when high. **A1-A5** are ignored.
When XRAM\* is low: **A5** selects the page register when high (A0-A4
are ignored), or the XRAM when low. In the latter case, **A0-A4** are
the address of a byte in the selected page.

Data bus
**D0-D7**: Bidirectional data bus. D0 is the least significant bit,
D7 the most significant one (opposite of the TI-99/4A).

Extra pins
**IRQ\***: the clock bring this open-collector output low when
issuing an interrupt (unless it is in stand-by or in battery-backed
mode). The load voltage should not excess Vcc. Leave n.c. when not in
use.
**SQW**: the clock can output a square wave signal on this pin if the
proper bit is set in register B. Otherwise, or when the clock is in
stand-by or power-off mode, the pin remains in high impedance.
**TMODE**: Test mode. Do not use. Leave open (has an internal pull-down
resistor) or connect to ground.
**TCLOCK**: Test clock Always keep low. The clock may not work properly
if either TMODE or TCLOCK is high.

####  Batteries

Use two BR-1225 batteries (12.5 mm diameter, 2.5 mm thick, 3.0 volts).
Do not mix old and new batteries, remove exhausted batteries in case
they would leak. Insert the batteries from right to left (pin 28 towards
pin 1) with flat side up, do not apply force on the pin1-side of the
holder. To remove a battery, insert a pointed object on the pin28-side
of it and lift it up gently.

####  Registers

he RTC contains 64 registers. To access a register, you must first pass
its address to the chip: just write a number from 0 to 63 while A0 is
low. Then pull A0 high and you can read or write the selected register.
Pulse RD\* low for a read, pulse WR\* low for a write. In all cases
RTC\* should be low to select the RTC. Registers 0 to 13 access clock
functions, the remaining ones are SRAM registers free for you to use.
IDEAL 1.0 uses register 14 to store the first two digits of the year
(20), but this won't be updated by the RTC don't forget that on December
31 2999.

Most values in the clock registers can be expressed as hexadecimal (\>00
to \>FF) or binary coded decimal (\>00 to \>99). With the latter, a hex
digit codes for the equivalent decimal digit. For instance 32 would be
coded \>32 (as opposed to \>20 in hexadecimal) and 99 would be \>99
(instead of \>63). IDEAL 1.0 expects the RTC to operate in hexadecimal
mode, so from now on I will put the emphasis on this one.

**Register 0**: Seconds. Valid values: 0 to 59 (Hex \>00-3B, BCD \>59)

**Register 1**: Alarm seconds. As above. Add \>C0 to disregard seconds
in the alarm settings.

**Register 2**: Minutes. Valid values: 0 to 59.

**Register 3**: Alarm minutes. As above. Add \>C0 to disregard minutes
in alarm settings.

**Register 4**: Hours. Valid values : 0 to 23 in 24h mode. 1 to 12 and
\>81 to \>8C in am/pm mode (add \>80 for pm).

**Register 5**: Alarm hours. As above. Add \>C0 to disregard hours in
alarm settings.

**Register 6**: Day of the week. Valid values 1 (Sunday) through 7
(Saturday).

**Register 7**: Day of the month. Valid values 1 to 31 (the RTC knows
how many days there are in each month. Leap years are taken into
account, including year 2000).

**Register 8**: Month. Valid values 1 to 12.

**Register 9**: Year. Valid values 0 to 99.

**Register 10**: Control register A.

DV
RS

**UIP**: Read-only bit. 1=Register update in progress. 0=no update for
the next 244 usec.
**DV**: Oscillator + frequency divider control 0=both stopped. 2=both
on. 6=oscillator on, reset all dividers below 8192 Hz.
**RS**: Frequency of the square wave signal on pin SQW.

**Register 11**: Control register B

|      |      |      |      |      |      |      |      |
|------|------|------|------|------|------|------|------|
| \>80 | \>40 | \>20 | \>10 | \>08 | \>04 | \>02 | \>01 |
| SET  | PIE  | AIE  | UIE  | SQW  | DM   | 24h  | DSE  |

**SET**: 1=stop register update, clears UIE and UIP. 0=registers are
updated every second.
**PIE**: 1=enabls periodic interrupts at a frequency determined by RS in
register A.
**AIE**: 1=enable alarm interrupts.
**UIE**: 1=enable update interrupts: once per second, after registers
are updated.
**SQW**: 1=issue a square wave signal on pin SQW, at a frequency
determined by RS.
**DM**: Data mode. 1=hexadecimal. 0=binary-coded decimal.
**24h**: 1=24h mode 0=am/pm mode. Caution: hour + alarm hour must be
modified when changing this bit.
**DSE**: 1=Enable daylight savings.

**Register 12**: Status register

This is a read-only register. Reading it automatically clears all
interrupt bits, so does bringing the RESET\* pin low.

not used

**IRQF**: 1=an interrupt occured, the IRQ\* pin is low. Cleared when
reading the status register.
**PF**: 1=a periodic interrupt occured since last time the status was
read. Set even if PIE is 0 (but IRQ\* stays high).
**AF**: 1=an alarm occured since the status was last read. Set whether
or not the interrupt is enabled by AIE.
**UF**: 1=an update was completed since status was last read. Set
whether or not UIE is enabled.

**Register 13**: Power-status register

This read-only register should be read twice as it provides a different
information each time.

not used

VLT: 1=voltage normal. The first time it is read VLT indicate what
happened while Vcc was off and the clock operated on its batteries. The
second (and subsequent) reading indicate what the power-status while Vcc
is powered.

**Registers 14 to 63**: User defined values, ignored by the RTC.

####  Extended RAM

The RTC-65271 contains an extra 4 Kbytes of static RAM. The clock
battery maintains the integrity of the data in the SRAM even when power
is off. To access the SRAM, pull down the XRAM\* pin (instead of the
RTC\* pin for clock access). The SRAM is arranged as 128 pages of 32
bytes. The address of the byte in a page must be places on A0-A5 with A6
low, then the byte can be accessed via D0-D7 (pulse RD\* down for a
read, pulse WR\* down for a write).

To select a page, pull A6 high: this accesses the page register (A0-A5
are ignored). Write the number of the desired page, 0 to 127 (higher
values loop back to 0-127). Then bring A6 back low to access data on
this page.

The IDE card maps the XRAM at \>4000-401F, which allows to store an
embyonnic DSR in it. All it does is to fetch IDEAL 1.0 from disk and
load it into the SRAM chip on the card. And yes, that's quite tricky to
do: imagine switching pages every 32 bytes! If you are curious to know
how this is done, here is the source.

####  Setting/reading the time and date

To read time and date, just read the relevant registers. The only
problem is that time may change as you are reading it (e.g. if its
2:59:59), so you may end up reading 2:00:00 if you read the hour before
the minutes. A simple solution is to always check the seconds last. If
it is 0, read the other registers once more (no need to check the
seconds this time).

Alternatively, you can set bit \>80 in register B, so as to freeze the
registers while you are reading them. This won't stop the clock that's
still ticking internally, it only prevents register update. This is
mandatory when you want to set the clock. Freeze it first, then set the
proper time and date, then restart it. In fact, if you really want split
second accuracy, you should first set the RV bits in register A as 6 and
restart the clock. This resets all stages of the frequency divider,
below 8192 Hz, and ensures that the first second will begin exactly 0.5
seconds later. Then freeze the clock again, set the RV bits as 2 and
restart the clock for normal operation.

You can enable summer-time (aka daylight saving time) by setting bit
\>01 in register B. When in this mode, the clock automatically jumps
from 1:59:59 am to 3:00:00 am on the first Sunday in April. It jumps
back from 1:59:59 to 1:00:00 am on the last Sunday in October.
**Alarms**

You can set an alarm for a given time in the day, by writing a value
into the three alarm registers: hour, minutes and seconds. If a register
is not used, set the two leftmost bits as 1 (i.e. values in the range
\>C0-FF). This way you can set repetitive alarms, e.g. every hour on the
hour (hours = \>C0, minutes=0, seconds=0). To disable alarms altogether,
enter an impossible value in one of the registers: e.g. minutes=64.

By setting bit \>xx in register B, you can cause the alarm to trigger an
external interrupt when it goes off. The IRQ will be brought low until
the alarm is acknowledged by reading register C. The IDE card sends this
signal to the EXTINT\* line, which causes an external interrupt. The
card SRAM contains an interrupt service routine that clears the alarms
and takes the appropriate (user selectable) action.
**Square wave**

The clock can generate a square-wave signal on its SQW pin. To do this,
set bit \>80 in register B. The signal frequency is determined by
register A, bits \>08 to \>01. The IDE card does not use this signal,
but makes it available as an external connection. The signal period is
also the interval between two periodic interrupts sent on pin IRQ\* if
the PIE bit is enabled in register B.

|     |               |                  |
|-----|---------------|------------------|
| RS  | SQW frequency | Interrupt period |
| 0   | \-            | \-               |
| 1   | 256 Hz        | 3.90625 ms       |
| 2   | 128 Hz        | 7.8125 ms        |
| 3   | 8192 Hz       | 122.07 us        |
| 4   | 4096 Hz       | 244.141 us       |
| 5   | 2048 Hz       | 488.281 us       |
| 6   | 1024 Hz       | 976.5625 us      |
| 7   | 512 Hz        | 1.953125 ms      |
| 8   | 256 Hz        | 3.90625 ms       |
| 9   | 128 Hz        | 7.8125 ms        |
| 10  | 64 Hz         | 15.625 ms        |
| 11  | 32 Hz         | 31.25 ms         |
| 12  | 16 Hz         | 62.5 ms          |
| 13  | 8 Hz          | 125 ms           |
| 14  | 4 Hz          | 250 ms           |
| 15  | 2 Hz          | 500 ms           |

###  Timing diagrams

#### Read cycle

      |            >395  ns               |
    XXX              valid                XXXX A0-A5
    __|__|a|                       |a|    |
      |   \___                      /     |    RTC*/XRAM*
      |   |>50 |a|            |a|>20|     |
      |   >50   \___>325 ns____/|   >20   |    RD*
                |25-240|        |10-100|
    -------------------X     valid     X------ D0-D7

    a)< 30 ns

####  Write cycle

      |          >395  ns                 |
    XXX              valid                XXXX A0-A5
    __|__|a|                       |a|    |
      |   \___                      /     |    RTC*/XRAM*
      |   |  0 |               |>20|      |
      |   0    \___>325 ns____/|   >20    |    WR*
            |    >200 ns       |0|
    --------X     valid          X------------ D0-D7

    a)< 30 ns

####  Interrupts

    ___     ___________________________
       \___/|                           RD*
    ________|_____________         ____
            |             \ >5 us /     RESET*
            |     _ _ _   |<2us|_______
            |<2us/     \      /         IRQ*

####  Square wave

                           __    __    __
    ______________________|  |__|  |__|  |__ SQW
                 | < 1 sec|
          \     /|                           WR*
                 |
                 X                           DV: 0 to 2

###   Electrical characteristics

#### Absolute maximum ratings

    Supply voltage Vcc:               -0.3V to 7V
    Input voltage:           Vss-0.3V to Vcc+0.3V
    Storage temperature:             -40 to 85 `C
    Soldering temperature: max 260 `C for 10 secs

####   Recommended operating conditions

|                      |     |     |     |      |
|----------------------|-----|-----|-----|------|
| Parameter            | Min | Nom | Max | Unit |
| Supply voltage, Vcc  | 4.5 | 5   | 5.5 | V    |
| Free-air temperature | -10 | .   | 70  | \`C  |

####  Electrical characteristics under recommended conditions


XRAM*, D0-D7, A0-A5

####  Frequency characteristics


25 `C reference
-120

------------------------------------------------------------------------

### IDE controller

#### Cable pinout

40-pins two-row connector:

    RESET*   1 2   GND
        D7   3 4   D8
        D6   5 6   D9
        D5   7 8   D10
        D4   9 10  D11
        D3  11 12  D12
        D2  13 14  D13
        D1  15 16  D14
        D0  17 18  D15
       GND  19     (Missing)
      DMAQ  21 22  GND
     DIOW*  23 24  GND
     DIOR*  25 26  GND
     IORDY  27 28  SPSYNC
     DMACK  29 30  GND
       IRQ  31 32  IO16*
       DA1  33 34  PDIAG*
       DA0  35 36  DA2
    CS1Fx*  37 38  CS3Fx*
      DASP  39 40  GND

The connections have the following functions:

     #  Name   I/O  Use
    --  -----  ---  ---------------------------------------
     1  RESET*  >   Resets all connected devices when low
     2  GND     -   Ground
     3  D7      <>  Data bus (D8-D15 only used with data register)
     4  D8      <>    "
     5  D6      <>    "
     6  D9      <>    "
     7  D5      <>    "
     8  D10     <>    "
     9  D4      <>    "
    10  D11     <>    "
    11  D3      <>    "
    12  D12     <>    "
    13  D2      <>    "
    14  D13     <>    "
    15  D1      <>    "
    16  D14     <>    "
    17  D0      <>  Least significany bit
    18  D15     <>  Most significant bit
    19  GND     -   Ground
    20   -      -   Missing pin used to prevent missconnecting the cable
    21  DMAQ    <   DMA request (optional)
    22  GND     -   Ground
    23  DIOW*   >   Pulse low to write to the IDE controller
    24  GND     <   Ground
    25  DIORD*  >   Pulse low to read from the IDE controller
    26  GND     -   Ground
    27  IORDY   <   Access completed, drive ready (optional)
    28  SPSYNC  ><  Spindle synchronisation between drives (leave open)
    29  DMACK   >   DMA acknowledge (optional)
    30  GND     -   Ground
    31  IRQ     <   Interrupt request from the IDE devices (active high)
    32  IO16*   <   Used by the controller to signal 16-bit operations (optional)
    33  DA1     >   Address bus, selects a register among 8
    34  PDIAG*  ><  Communication between master and slave drive (leave open)
    35  DA0     >   Address bus, least significant bit
    36  DA2     >   Address bus, most significant bit
    37  CS1Fx*  >   Selects the first group of registers
    38  CS3Fx*  >   Selects the second group of registers
    39  DASP    <   Drive active/slave present signal
    40  GND     -   Ground

####  Registers

The IDE controller interface consists in 13 registers, all 8-bit wide
except the data register which is 16-bit. Most are bidirectional, but
there also are 3 read-only and 3 write-only registers. Generally, a
read-only register shares its address with a write-only register.
Registers are organized as two blocks of 8 addresses. To access a
register, place its address on pins A0-A2, and select the proper block
by bringing CS1Fx or CS3Fx low. To read from a register, pulse RD\* low.
To write to it, pulse WR\* low. All registers are 8-bit wide and
communicate via D0-D7, except for the data register that is 16-bit wide
and uses D0-D15. With the IDE card, all register map into the least
significant byte of a word, the data register maps into the whole word.

CS

CS1Fx
CS3Fx

Data register
This is the only 16-bit register. It is used to pass data to/from
the IDE contoller.

Sector count register
Used to indicate to the controller how many sectors are to be
transfered (0 means 256).

Sector number register
Originally used to pass the sector number to/from the controller.
With modern controller, this is just the 4th (least significant) byte of
the LBA (logical block address).

Cylinder number lsb register
Originally used to pass the least significant byte of the track
number (called cylinder with hard drives) to/from the controller.
Nowadays its the 3rd byte of the LBA.

Cylinder number msb register
Originally used to pass the most significant byte of the cylinder
number (and was limited to 6 bits). Now, its the second byte of the LBA.

Drive/head register
Originally used to pass the drive number (0 or 1) and the head
number (0 to 15). If the proper bit is set, it causes the controller to
use logical block addressing, and the content of the register will be
the first (most significant) byte of the LBA.

Head number

**LBA**: a 1 means LBA addressing mode, a 0 means CSH addressing.
**DRV**: a 0 means master drive, a 1 means slave drive.

Write pre-compensation register
Not used. It only exits for backward compatibility, but all IDE
controllers ignore it.

Status register
This is a read only register. It is used by the IDE controller to
return some information that depends on the command being executed.
Common bits are:

|      |      |      |      |      |      |      |      |
|------|------|------|------|------|------|------|------|
| \>80 | \>40 | \>20 | \>10 | \>08 | \>04 | \>02 | \>01 |
| BSY  | RDY  | WFT  | SKC  | DRQ  | COR  | IDX  | ERR  |

**BSY**: a 1 means that the controller is busy executing a command. No
register should be accessed (except the digital output register) while
this bit is set.
**RDY**: a 1 means that the controller is ready to accept a command, and
the drive is spinning at correct speed..
**WFT**: a 1 means that the controller detected a write fault.
**SKC**: a 1 means that the read/write head is in position (seek
completed).
**DRQ**: a 1 means that the controller is expecting data (for a write)
or is sending data (for a read). Don't access the data register while
this bit is 0.
**COR**: a 1 indicates that the controller had to correct data, by using
the ECC bytes (error correction code: extra bytes at the end of the
sector that allows to verify its integrity and, sometimes, to correct
errors).
**IDX**: a 1 indicates the the controller retected the index mark (which
is not a hole on hard-drives).
**ERR**: a 1 indicates that an error occured. An error code has been
placed in the error register.

Error register
This is a read only register. Its content is only meaningfull when
the ERR bit is set in the status register.

|      |      |      |      |      |      |      |      |
|------|------|------|------|------|------|------|------|
| \>80 | \>40 | \>20 | \>10 | \>08 | \>04 | \>02 | \>01 |
| BBK  | UNC  | MC   | NID  | MCR  | ABT  | NT0  | NDM  |

**BBK**: 1 = bad block. Sector marked as bad by host.
**UNC**: 1= uncorrectable data error.
**MC**: 1 = medium changed (e.g. CD-ROM. Enhanced IDE only).
**NID**: 1 = No ID mark found.
**MCR**: 1 = medium change required (Enhanced IDE only).
**ABT**: 1 = command aborted.
**NT0**: 1 = No track 0 found.
**NDM**: 1 = No data mark found.

Command register
This is a write-only register. It accepts commands to the IDE
controller. Note that most commands imply some values to have been
placed in the sector, cylinder, etc registers. Always sends the command
last. Here is a non exhaustive list of IDE commands:

Digital output register
This is a write-only register. It is used to send advanced
intructions to the controller. Only two bits are active:

|      |      |      |      |      |      |      |      |
|------|------|------|------|------|------|------|------|
| \>80 | \>40 | \>20 | \>10 | \>08 | \>04 | \>02 | \>01 |
| \-   | \-   | \-   | \-   | \-   | \-   | RST  | IEN  |

**RST**: when set to 1 issues a reset signal to all connected drives.
Must be reset to 0 for proper operation.
**IEN**: when set to 1, the controller won't issue interrupts. When 0,
an interrupt is issued after each sector or in advance of the result
phase (command completion).

Drive address register
This is a read-only register that provides info on the current drive
status.

Head*
DS*

**WGT**: Status of the write gate. A 0 means the gate is open and the
read/write head is currently wrinting on the disk.
**Head\***: Currently active head. Caution: the bits are inverted!
**DS\***: Currently active drive.

#### IDE commands

To be passed to the command register, after having placed the required
parameters (if any) in the other registers.

**\>1X** Recalibrate the disk. All commands \>10 to \>1F will result in
a recalibrate disk command being executed. This command has no
parameters. You simply write the command code to the command register
and wait for ready status to become active again.

**\>20** Read sector, with retry. (**\>21** = read sector without retry,
**\>22** = also pass ECC bytes, **\>23**: both). For this command you
have to load LBA first. When the command completes (DRQ goes active) you
can read 256 words (16-bits) from the data register. Commands \>22 and
\>23 will pass the four error correction code bytes after the data
bytes.

**\>30** Write sector, with retry. (**\>31** = without retry, **\>32** =
ECC bytes will be passed by host, **\>33**: both). Here too you have to
load cylinder/head/sector. Then wait for DRQ to become active. Feed the
disk 256 words of data in the data register. Next the disk starts
writing. When BSY goes not active you can read the status from the
status register. With commands \>30 and \>31, the controller calculates
the four ECC bytes (error correction codes), with commands \>32 and \>33
you should provides these four bytes after the data bytes.

**\>40** Verify sector, with retry (**\>41** = without retry). Reads
sector(s) and checks if the ECC matches, but doesn't transfer data.

**\>50** Format track. Extremely dangerous command: if the controller
performs address translation it may hopelessly mess-up the disk. Don't
use: IDE drives should be formatted in factory.

**\>70** Seek. This normally does nothing on modern IDE drives. Modern
drives do not position the head if you do not command a read or write.

**\>90** Diagnostic. Tells the controller to perform a self test and to
test the drives. Results are passed in the error register: 1=master ok,
2=format circuit error in master, 3=buffer error in master, 4=ECC logic
error in master, 5=microprocesssor error in master, 6=interface error in
master. \>80=at least one error in slave.

**\>91** Set drive parameters. Lets you specify the logical geometry
used for address translation. Pass the number of heads (and the drive of
interest) in the drive/head register, and the number of sectors per
track in the sector count register.

Optional command (enhanced IDE)

**\>3C** Write verification.

**\>C4** Read multiple sectors.

**\>C5** Write multiple sectors.

**\>C6** Set multiple mode.

**\>C8** DMA read, with retry (**\>C9** without retry). Reads sector in
direct memory access mode.

**\>CA** DMA write, with retry (**\>CB** without retry).

**\>DB** Acknowledge medium change.

**\>DE** Lock drive door.

**\>DF** Unlock drive door.

**\>E0** or **\>94** Standby immediate. Spins down the drive at once. No
parameters.

**\>E1/\>95** Idle immediate.

**\>E2/\>96** and **\>E3/\>97** Set standby mode. Write in the sector
count register the time (5 seconds units) of non-activity after which
the disk will spin-down. Write the command to the command register and
the disk is set in an auto-power-save modus. The disk will automatically
spin-up again when you issue read/write commands. \>E2 (or \>96) will
spin-down, \>E3 (or \>97) will keep the disk spinning after the command
has been given. Example: write \>10 in the sector count register, give
command \>E2 and the disk will spin-down after 16\*5=80 seconds of
non-activity.

**\>E4** Read buffer. Reads the content of the controller buffer
(usefull for tests).

**\>E5/\>98** Checks for active, idle, standby, sleep.

**\>E6/\>99** Set sleep mode.

**\>E8** Write buffer. Loads data into the buffer but does not write
anything on disk.

**\>E9** Write same sector.

**\>EC** Identify drive. This command prepares a buffer (256 16-bit
words) with information about the drive. To use it: simply give the
command, wait for DRQ and read the 256 words from the drive.

**\>EF** Set features.

**\>F2** and **\>F3** The same as\> E2 and \>E3, only the unit in the
sector count register is interpreted as 0.1 sec units.

Codes \>80-8F, \>9A, \>C0-C3, and \>F5-FF are manufacturer dependent
commands. All other codes are reserved for future extension.

#### Operating the IDE controller

Lets take the read command as an example. First we should set the
address of the desired sectors in the sector, cylinder, etc registers.
Let's say we are using logical block addressing and want to access
logical block \>123456.

Sector register \<--- 56
Track register \<---34
xxx register \<---12
Head/drive register \>E0 (bit \>40 indicates LBA)
Sector size register \<--256 (hard drive sectors are 512-byte in length,
i.e 256 words).

Then we can send the read command to the command register

Status register --\> wait till ready and not busy
Command register \<--xx
Status register --\> wait till ready and not busy (while the controller
seeks and read data into its buffer).

Now we can read the data

Status register --\> wait for DRQ
Data register --\> read 256 16-bit words.
Status register--\> check ERR: if set check the error register.

The controller triggers its interrupt line each time a sector is ready
to be read. The IDE card lets you see this signal on CRU bit 0. For
write operations, the IRQ line is activated each time a sector has been
written and the controller is waiting for another one (including after
the last sector: when the user can read the registers again). For all
other commands, an interrupt is issued at the result phase, i.e. when
the user can access the registers again after command completion.

Note that the controller contains a 512-byte buffer, so when writing
data to the disk, you don't have to wait until each byte is physically
written: just place the data into the buffer and leave the rest to the
controller. You will have to wait between sectors though. Remember that
hard-drive sectors are 512 bytes in length.

#### LBA versus CSH

Hard drives consists in several disks, each accessed via a reading head.
Each disk contains a number of concentric track and a set of tracks with
the same number on each disk is called a cylinder. Tracks are subdivided
in sectors. The number of sector per track may vary from drive to drive
or even from track to track on the same drive (tracks at the inside of
the disk are shorter and can accomodate less data).

Originally, to access a sector you had to know its number on the track,
the track (or cylinder) number and which head should be used. This was
called CHS (cylinder-head-sector) adressing. It was quite incovenient
because it requires you to know the physical geometry of the drive
(number of heads, etc). In addition, the PC BIOS and DOS placed
limitations on the maximum values that could be sent (e.g. the cylinder
number could be 1023 at max), whereas other values were limited by the
hardware (no track can accomodate 256 sectors).

The second generation of IDE controllers started to perform "address
translation", i.e. they take the CSH address passed by the user and
adapt it to their internal geometry. For instance, a drive with more
than 16 heads could decide the the most-significant bit of the sector
number should be used to select heads 16-31. This also allowed smart
controllers to remap deffective tracks: they always are a few extra
tracks at the end of a disk. When the controller determines that a track
is deffective, it replaces it with one of the extra track, in a
completely user-transparent manner (except that it may be a tad longer
to move the to extra track than to the original one).

The next step was to get rid of CSH altogether and to let the controller
decide how to arrange data on its disks. This is the concept behind
logical block addressing (LBA). With LBA you consider your drive as a
collection of blocks, numbered from 0 to \>0FFF FFFF for the master
drive, and from \>1000 0000 to \>1FFF FFFF for the slave drive. To
access a sector, just pass its number to the controller and let it
determine where the sector is physically located. LBA is far easier to
handle than CSH as it places the burden on the controller, not on the
programmer. On top of that, it is portable from drive to drive, which is
a big plus. Finally, it lets you use the whole address space (no
hardware or software limitations), upto a maximum of 128 gigabytes per
drive.

The IDE card can perform both CSH and LBA addressing, but IDEAL 1.0 only
knows LBA.

#### Direct memory access

The LBA controller is able to transfer data directly into the computer
memory, bypassing the CPU. Unfortunately, this cannot be done with the
TI-99/4A, so the DMA commands should never be used.

#### Drives considerations

The IDE bus is intended for two devices: a master and a slave. These
could be hard-drives, CD-ROM drives, or even a ZIP-drive. I only have
tested the card with a single hard-drive, but it should also work with
two. Just make sure you place the jumpers properly on each drive. If you
want to access a CD drive, you'll have to write your own routines to
translate the PC format.

Your drive must be recent enough to support LBA (logical block
addressing). This is purely a software requirement: that's what the DSRs
are expecting. Of course, if you are willing to write CSH addressing
routines yourself, feel free to do so. The maximum supported size is 128
Gigabytes per drive.

Most drives have three jumpers. CS (cable select) is almost never used,
so your choice is between master and slave. If you have a single drive
leave all jumpers open (some drives allow a "storage" position for the
unused jumper: accross MA and SL). If you want two drives, install one
as the master, the other as the slave (duh!).

    MA o o            MA o-o         MA o o
    SL o o            SL o o         SL o-o
    CS o o            CS o o         CS o o
    One drive         Master         Slave

NB: Both devices get their registers written, but only the selected
device will execute commands. Any data or register read will come from
the selected device only.

------------------------------------------------------------------------

Software

[CRU map
](#CRU%20map)[Memory map
](#Memory%20map)[Low-level access
](#Low-level)[High-level access](#High%20level)

### CRU map


&lt;&gt; switch: SRAM maps at &gt;4000-40FF

###  Memory map

When CRU bit 1 matches the position of the DIP-switch (i.e. when you
write back what you read), the address space \>4000-40FF is used to map
the clock and IDE registers. Otherwise, the SRAM maps here, just as in
the rest of the address space (\>4100-5FFF).

*  Check DIP-switch for register mapping, select proper instructions
*---------------------------------------------------------------------
DIPREG LI   R1,&gt;1D01          This is SBO 1
       LI   R2,&gt;1E01          This is SBZ 1
       TB   1                 Test DIP-switch
       JNE  SK3
       MOV  R1,@REGON         DIP is on
       MOV  R2,@REGOFF        Select registers with SBO
       B    *R11
SK3    MOV  R1,@REGOFF        DIP is off
       MOV  R2,@REGON         Select registers with SBZ
       B    *R11`
REGON  NOP                    Register selection instruction
REGOFF NOP                    SRAM selection instruction
*`
       X    @REGON            Maps registers at &gt;4000-40FF`
       X    @REGOFF           Maps SRAM at &gt;4000-40FF

#### Register mapping

    >4000-401F: clock XRAM
    >4020: RTC address register
    >4030: RTC data register
    >4040: IDE data register (read)
    >4042: IDE error register (read)
    >4044: IDE count register (read)
    >4046: IDE sector register (read)
    >4048: IDE cylinder lsb register (read)
    >404A: IDE cylinder msb register (read)
    >404C: IDE drive+head register (read)
    >404E: IDE status register (read)
    >4050: IDE data register (write)
    >4052: IDE pre-comp register, not used (write)
    >4054: IDE count register (write)
    >4056: IDE sector register (write)
    >4058: IDE cylinder lsb register (write)
    >405A: IDE cylinder msb register (write)
    >405C: IDE drive+head register (write)
    >405E: IDE command register (write)
    >406C: IDE alternate status register(read)
    >406E: IDE drive address register (read)
    >407C: IDE digital output register (write)
    >407E: IDE drive address register (write)
    >4080: XRAM page register

###  Low-level access

#### SRAM paging

To switch pages, all we have to do it to enable switching with CRU bit 2
and then write to an address in the SRAM. Address lines A8-A14 will be
used as a page number (although you probably won't need that many pages,
unless you install 2 Megs of SRAM on board). Just to make sure we don't
mess up any data, it may be a good idea to write-protect the SRAM
beforehand.

* SRAM page selection routine. Page # in  R1
*---------------------------------------------------------------------
SELPG  SLA  R1,8             Max 256 pages
       SRL  R1,7             A15 is not latched: shift to the left
       SBO  5                Write protect SRAM
       SBO  2                Enable latching
       MOV  R1,@&gt;5F00(1)     Latch page #, from address lines A7-A14
       SBZ  2                Disable latching
       SBZ  5                Enable writing
       SRL  R1,1             Restore page # for caller (optional)
       B    *R11

####  RTC access

To access a register in the real-time clock, first write the register
number (\>0000 to \>3F00) at address \>4020. For some reason, this
number cannot be read back, I'm not sure if it's a characteristic of the
chip, or a timing problem with my design. The specified register can
then be accessed byte-wise at address \>4030. Remember to stop clock
updates by setting bit \>80 in register B, before you attempt to modify
registers 0 to 9.

* Write to a register in the RTC. Register # in R3, data in R1
*---------------------------------------------------------------------
RTCWR  SLA  R3,8              Put register # in msb
       MOVB R3,@&gt;4020         Pass register #
       SWPB R3                Restore R3
       SLA  R1,8              Put value in msb
       MOVB R1,@&gt;4030         Set clock data
       SWPB R1                Restore R1
       B    *R11`
*---------------------------------------------------------------------
Read a register from the RTC. Register # in R3, result in R1
*---------------------------------------------------------------------
RTCRD  SLA  R3,8              Put register # in msb
       MOVB R3,@&gt;4020         Pass it
       SWPB R3                Restore R3
       MOVB @&gt;4030,R1         Get clock data
       SRL  R1,8              Make it a word
       B    *R11

#### XRAM access

The SRAM can be accessed at addresses \>4000-401F. To change page, write
the new page number at address \>4080. Valid page numbers are \>0000 to
\>7F00, although other values won't cause any problem (all extra bits
are ignored). The value can be read back to know what the current page
is.

* Set XRAM page. Page # in R0
*---------------------------------------------------------------------
SELXPG SLA  R0,8            Put it in MSB
       MOVB R0,@&gt;4080       Set page
       MOVB @&gt;4080,R0       Read it back (optional)
       SRL  R0,8            Make it a word for caller
       B    *R11            Page in now available at &gt;4000-401F

#### IDE access

You can read IDE registers at addresses \>4040-404E and \>406C-406E. You
can write to them at \>4050-405E and \>407C-407E. Make sure you check
the status bits before sending a command, otherwise you will read
meaningless data and the controller will ignore your writes. Apart from
the 16-bit data register, all registers are 1 byte long and are accessed
at odd addressed (i.e. in the least significant byte of a word).

Remember that sectors are 512 bytes in length on a hard disk. This will
require some manipulation if you want to emulate the 256-byte sectors of
floppy disks...

* Send a command to the IDE controller. Command in R0
*---------------------------------------------------------------------
SENDCM MOV  @&gt;404E,R1         Get status
       COC  @STRDY,R1         Check &quot;ready&quot; bit
       JNE  SENDCM            Not set yet: wait
       COC  @STBSY,R1         Check &quot;busy&quot; bit
       JEQ  SENDCM            Set: wait`
       MOV  R0,@&gt;405E         Send command to command register
LP2    MOV  @&gt;404E,R1         Read status
       COC  @STBSY,R1         Check &quot;busy&quot; bit
       JEQ  LP2               Wait till not busy
       COC  @STERR,R1         Now check &quot;error&quot; bit
       JNE  SK1               Not set
       MOV  @&gt;4042,R1         Get error code from error register
       B    *R11
SK1    INCT R11               Skip jump if ok
       B    *R11`
STBSY  DATA &gt;0080             &quot;Busy&quot; status bit
STRDY  DATA &gt;0040             &quot;Ready&quot; status bit
STDRQ  DATA &gt;0008             &quot;Data requested&quot; bit
STERR  DATA &gt;0001             &quot;Error&quot; status bit`
*---------------------------------------------------------------------
* Set logical block address. Block # in R0. Base LBA in data (optional)
*---------------------------------------------------------------------
SETLBA MOV  @&gt;404E,R1          Get status
       COC  @STRDY,1R          Check &quot;ready&quot; bit
       JNE  SETLBA             Not set yet: wait
       COC  @STBSY,R1          Check &quot;busy&quot; bit
       JEQ  SETLBA             Set: wait
*      SRL  R0,1               Optional: if using 256-byte sectors #
       A    @LBA3,R0           Add R0 to base LBA
       MOV  R0,@&gt;4056          Set LBA address
       SRL  R0,8
       A    @LBA2,R0
       MOV  R0,@&gt;4058
       SRL  R0,8
       A    @LBA1,R0
       MOV  R0,@&gt;405A
       SRL  R0,8
       A    @LBA0,R0
       MOV  R0,@&gt;405C
       LI   R0,&gt;0001
       MOV  R0,@&gt;4054           Transfer 1 sector
       B    *R11`
LBA0   DATA &gt;00E0               LBA buffer (only LSB are used)
LBA1   DATA &gt;0000
LBA2   DATA &gt;0000
LBA3   DATA &gt;0000`
*---------------------------------------------------------------------
Set drive parameters. Must be sent before any other command
*---------------------------------------------------------------------
SETPAR MOV  R11,R10
       LI   R0,&gt;20             (Arbitrary value)
       MOV  R0,@&gt;4054          Number of logical sectors/track
       LI   R0,&gt;E8             (Arbitrary value)
       MOV  R0,@&gt;405C          Number of logical heads
       LI   R0,&gt;0091           &quot;Set drive parameters&quot;
       BL   @SENDCM            Send command
       B    *R10               Skipped if ok
       INCT R10                Skip JMP if ok
       B    *R10`
*---------------------------------------------------------------------
* Read a 512-byte sector into buffer in R2, sector # in R0
*---------------------------------------------------------------------
SECRD2 MOV  R11,R10
       BL   @SETLBA           Set LBA for sector # in R0
       LI   R0,&gt;0020          &quot;Read sector&quot;
       BL   @SENDCM           Send command
       B    *R10              Skipped if no error
LP3    MOV  @&gt;404E,R1         Wait for data to be ready
       COC  @STERR,R1         Test error bit
       JEQ  UHO               Error
       COC  @STDRQ,R1         Test data request bit
       JNE  LP3               Not ready yet
       LI   R0,512
LP4    MOV  @DATAR,*R2+       Read 512 bytes
       DECT R0
       JNE  LP4
       AI   R2,-512           Restore R2
       INCT R10               Skip JMP if ok
       B    *R10`
*---------------------------------------------------------------------
*  Write a 512-byte sector from buffer in R2, sector # in R0
*---------------------------------------------------------------------
SECWR2 MOV  R11,R10
       BL   @SETLBA           Set LBA for sector # in R0
       LI   R0,&gt;0030          &quot;Write sector&quot;
       BL   @SENDCM           Send command
       B    *R10              Skipped if no error
LP5    MOV  @&gt;404E,R1         Wait for data to be ready
       COC  @STERR,R1         Test error bit
       JEQ  UHO               Error
       COC  @STDRQ,R1         Test &quot;data request&quot; bit
       JNE  LP5               Not yet
       LI   R0,512
LP6    MOV  *R2+,@&gt;4050       Write 512 bytes
       DECT R0
       JNE  LP6
       AI   R2,-512           Restore buffer ptr
       INCT R10               Skip JMP if ok
       B    *R10`
UHO    MOV  @&gt;4002,R0         Error detected: get error code
       B    *R10              And abort

###  High-level access

The card DSR, or disk operating system is known as IDEAL, for IDE Acess
Layer. A dedicated loader, called IDELOAD, serves to load it into the
card SRAM. You also have the option to install a boot routine in the
clock XRAM that will automatically load IDEAL from the hard disk upon
power-up. IDEAL is described in details on [another page](ideal.md).
Revision 1. 7/28/00. Preliminary, not for release.
Revision 2. 9/12/00. Added code examples. Still not for release.
Revision 3. 3/19/01. Schematic changed: 74LS85 wiring,clock reset.
Revision 4. 7/2/01. Multiplexer modified, RD pulse genrator added. OK to
release.
[Back to the TI-99/4A Tech Pages](titechpages.md)
