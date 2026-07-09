# IDE interface card

Building a hard drive controller card for my TI-99/4A has long been a
dream of mine. Unfortunately, it was far beyond my abilities. And yet,
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
"small" IDE drives. Three years ago I bought a 540 Megs drive for \$18,
to give you an idea. And 540 Megs is equivalent to about 6000 floppies!
This interface card supports **two IDE drives**. It may also work with
CD-ROMs but you'll have to write your own control program.

While I was at it, I also included a 32K to 512K **SRAM memory** chip on
the card. It serves to hold the DSRs and the buffers for opened files.
The memory is also available to the user in the area \>6000-7FFF,
according to the RAMBO protocol introduced with the Horizon Ramdisk.

Last but not least, the card comprises a **real-time clock**. This
allows the software to time-stamp the files, which is something I always
missed with the TI-99/4A. The clock chip I originally selected also
contains 4K of battery-backed memory, so we can stuff the DSRs in there,
to be transfered in the SRAM at power-up time. This was meant to save me
the trouble of having a battery-backed SRAM. Unfortunately, this chip is
now very hard to find. Thus, the new version of the card accepts four
different clock chips, hopefully you can find one of them.

Oh yes, this is the second version of this board. The first one was a
point-to-point soldered prototype, which worked well except that the
wires had a tendency to break every now and then. I got fed up fixing it
and decided to lay out a PCB, based on the one I did for the USB-SM
card. Apart for supporting 4 different clock chips, the new version
features the 74F543-based demultiplexer, which should make it compatible
with Myarc's Geneve. It also has DMA capability, in case I come up with
a DMA controller board. If you would like to know about the previous
version, look in [here](ide.md).

I have no intention to market that card, but if you want to build your
own you'll find all the necessary instructions in this page. You can
order the PCB from any commercial venture, by sending them this file,
which contains the PCB design, in Gerber format. You can also download
the operating system (and its instruction manual), tentatively named
[IDEAL](ideal.md) for "IDE Access Layer".

Please, let me know if you built such a card and if you're happy with
it. Suggestions for improvements, and software bug reports are always
appreciated.

height="112" alt="2001 Mauk Award" />I'm proud to announce that the
first version of this card was granted the 2001 Edgar Mauk award, in the
category "hardware".

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
copyright and a link to the present webpage. Thanks.

**Hardware
  **
[Building the card
](#Components)**Circuitery description  **
[Power supply
](#Power)[Bus buffering
](#Bus%20buffers)[CRU logic
](#CRU%20logic)[Address decoding
](#Addressing)[DMA support
](#DMA)[SRAM access
](#SRAM%20logic)[RTC-65271 clock
](#Clock%20logic)[bq4847 clock
](#bq4847%20wiring)[bq4842 / 4852 clock
](#bq48x2%20wiring)[IDE port access
](#IDE%20logic)**Specifications  **
[RTC-65271 clock
](#RTC%2065271)[bq4847 clock
](#bq4847)[bq4842 & bq4852 chips
](#bq4842/4852)[IDE interface

](#IDE%20pinouts)**Software**

[CRU map
](#CRU%20map)[Memory map
](#Memory%20map)[Low-level access
](#Low-level)[High-level access](#High%20level)

------------------------------------------------------------------------

Hardware

### Building the card

[Components required
](#Components)[Printed circuit board
](#PCB)[Soldering components
](#Soldering)[Installing the card](#Installing)

#### Components required

I ordered all components from two suppliers: Arrow
([www.arrow.com](http://www.arrow.com)) or Digikey
([www.digikey.com](http://www.digikey.com)). Prices are of spring 2004.
Arrow is cheaper than Digikey, but their search engine is a nightmare
unless you know exactly what you want. The 74LS are a bit cheaper than
their advanced 74ALS counterpart but not by much, so I generally went
for the ALS series (except for the 74LS125, because I wasn't 100% sure
an ALS was ok). Make sure you check the descriptions when you place your
order, as I do not guarantee that all catalog numbers are correct...

Most chips are SOIC surface-mount packages, but there are a few
exceptions (e.g.the SRAM is a TSOP, the clock modules are DIPs). The
individual resistors and capacitors are all 0805 packages, except for
the big 47 uF cap.

|  |  |  |  |
|----|----|----|----|
| \# | Component | Arrow | Digikey |
| 1 | 74(A)LS02 | SN74ALS02AD \$0.19 | 296-14705-1-ND \$0.48 |
| 1 | 74(A)LS04 | SN74ALS04BD \$0.10 | 296-1122-1-ND \$0.49 |
| 1 | 74(A)LS08 | SN74ALS08D \$0.16 | 296-1123-1-ND \$0.49 |
| 3 | 74(A)LS32 | SN74ALS32D \$0.17 | 296-1127-1-ND \$0.49 |
| 1 | 74(A)LS74 | SN74ALS74AD \$0.11 | 296-1130-5-ND \$0.49 |
| 1 | 74LS125 | SN74LS125AD \$0.23 | 296-14715-1-ND \$0.48 |
| 2 | 74(A)LS138 | SN74ALS138AD \$0.28 | 296-14714-1-ND \$0.56 |
| 1 | 74(A)LS139 | SN74ALS139D \$0.19 | 296-14715-1-ND \$0.72 |
| 4 | 74(A)LS245 | SN74ALS245ADW \$0.19 | 296-1125-1-ND \$0.48 |
| 1 | 74(A)LS251 | SN74ALS251D \$0.16 | Not available |
| 1 | 74(A)LS259 | SN74ALS259D \$1.57 | 296-14729-1-ND \$1.93 |
| 1 | 74(A)LS373 | SN74ALS373ADW | 296-14765-ND \$0.64 |
| 2 | 74F543 | 74F543SC \$0.61 | 296-14821-1-ND \$1.05 |
| 2 | 74(A)LS688 | SN74ALS688DW \$2.52 | Non-stock item |
| 1 | +5V regulator 1.5A | ? | 296-12396-1-ND \$0.72 |
| 1 | electolytic cap 47 uF | ? | 4070PHCT-ND \$0.49 |
| 30 | ceramic cap100 nF | ? | 311-1141-1-ND \$0.84 (for 10) |
| 1 | 7x 22 Ohms resistors network | ? | 767-143-R22-ND \$0.66 |
| 1 | 7x 4.7K resistors network | ? | 767-143-R4.7K-ND \$0.66 |
| 1 | 7x 10K resistors network | ? | 767-143-R10K-ND \$0.66 |
| 10 | Resistor 220 Ohm | ? | 311-220ACT-ND \$0.76 (for 10) |
| 10 | Resistor 4.7 kOhm | ? | 311-4.7KACT-ND \$0.76 (for 10) |
| 10 | Resistor 10 kOhm | ? | 311-10KACT-ND \$0.76 (for 10) |
| 2 | LEDs | ? | MV63539MP7-ND \$0.49 |
| 1 | SP3T switch | ? | EG2485-ND \$0.82 |
| 1 | DPDT switch | ? | EG1940-ND \$0.82 |
| 1 | DIP-switch 4x | ? | CT2194MST-ND \$0.59 |
| 1 | Rotary encoder hex | ? | GH1319-ND \$3.02 |
| 1 | 40-pin breakaway header | ? | A26512-ND \$1.27 |
| 1 | Heat sink | ? | 294-1035-ND \$1.50 |

You will also need an IDE connection cable, and at least one IDE drive,
two at most. The drive(s) must support LBA (logical block addressing) to
work with my IDEAL DSRs, but most drives do nowadays.

Note: rather than an expensive plastic-molded IDE connector, I picked a
cheap 40-pin breakaway header. Break it in half and install it as two
rows of 20 pins. A good thing to do is to pull out pin \#20 (the middle
pin in the back row, corresponding to a square pad in the PCB) as some
IDE cables have no hole in this position. This serves as a failsafe to
prevent you from connecting the cable the wrong way. Actually, if your
cable plug does have a hole in position \#20, it may be a good idea to
plug it with a little piece of wire...

As I mentionned, you have the choice between four different clock chips.
According to what you prefer, or can find, select ONE of the following
options:


A
B

Option A

This it the original design I used for my prototype, it is the cheapest
solution, but the clock chip is becoming hard to find. The RTC-65271
clock chip has a built-in crystal, and a battery holder for two BR1225
batteries. It contains 4K of battery-backed SRAM, arranged as 128 pages
of 32 bytes. This is not enough to load the whole DSRs, so an external
SRAM is required. I recommend 512K, although 128K would do. The SRAM in
the side the clock holds a bootstrap programs that loads the DSRs from
disk into the external SRAM upon power-up. Note that you will need to
solder in two jumpers to power the SRAM and to select it, since you are
not using the bq4847 for this purpose.
Option B

The bq4847 has an built-in crystal and battery. You cannot change the
battery, but it's supposed to last for 10 years. The chip contains no
SRAM, so an external SRAM is required. I recommend 512K, although 128K
would do. The clock chip has the ability to battery-back the SRAM, so
the DSRs can remain permanently loaded into the SRAM.
Option C and D

Both these clock chips contain a built-in crystal and battery. You
cannot change the battery, but it's supposed to last 10 years. These
chips also comprise SRAM: 128K for the bq4842, 512K for the bq4852, so
there is no need for an external SRAM. The clock registers map into the
last addresses of the SRAM. This is probably the most elegant solution,
unfortunately it's also quite expensive: these chips are grossly
overpriced! Be aware that I didn't buy them, so I cannot guarantee that
they will work...

####  Printed circuit board

Here you have the choice between etching the board yourself, or ordering
one. If you want to etch the board yourself, contact me and I'll send
you a postscript file containing the PCB layout for you to print on
overhead transparencies. Otherwise, there are many outfits that will
make a professional quality PCB for you. You will need to send them the
files included within [this zip file](ide7.zip) (about 50 kB). Make sure
you read the 'readme.txt' file that identifies the individual files. I
ordered my second prototype board from Advanced Circuits at
[www.4pcb.com](http://www.4pcb.com) They charged me \$33 for one board
(plus \$15 S&H), which is reasonnable given the great job they did.

####  Soldering components

Components can be soldered by hand, or with a toaster oven. Refer to my
"[USB howto](usbhowto.md)" page for detail, including a pictorial guide
on how to solder those tiny surface-mount chips.

Components are numbered as follows on the PCB:

IC1: 74ALS245
IC2: 74ALS245
IC3: 74ALS245
IC4: 74ALS245
IC5: 74LS125
IC6: 74F543
IC7: 74F543
IC8: 74ALS138
IC9: 74ALS139
IC10: 74ALS688
IC11: 74ALS32
IC12: 74ALS32
IC13: 74ALS32
IC14: 74ALS02
IC15: 74ALS688
IC16: 74ALS259
IC17: 74ALS251
IC18: 74ALS373
IC19: 74ALS08
IC20: 74ALS04
IC21: 74ALS74
IC22: 74ALS138 (Geneve)
IC23: SRAM 512K (if needed)
IC24: RTC65271 clock}
IC25: bq4847 clock }
IC26: bq4842 clock } Pick one clock
IC27: bq4852 clock }
Q1: 5V voltage regulator TL780-05CKTER

C1-C27: 100 nF ceramic capacitors
C28: 47 uF electrolytic capacitor (watch polarity!)

R1: 7 x 22 Ohms resistor network
R2: 7 x 4.7 kOhms resistor network
R3: 7 x 10 kOhms resistor network
R4-R5: 220 Ohms resistors
R6-R8: 10 kOhms resistors
R9: 4.7 kOhms resistor
R10-R11: 4.7 kOhms resistor (for Geneve with no GenMod)

LED1: Yellow light-emiting diode
LED2: Red (optional)

SW1: SP3T switch
SW2: DPST switch
SW3: 4 x DIP switch
SW4: Hex rotary selector

CN1: PE-box slot
CN2: 2 x 20-pin header
**Installing the card**

The PCB I designed will fit in any slot in the PEB, except for \#5 which
has a bolt protruding at the back. I suggest you use slot \#8 and place
your hard drive in the bay meant for the floppy drive. This way, you can
fit the IDE cable though the opening on the left side of the drive bay
and power your drive with the leads meant for the floppy (the power
connector is identical).

If you want to retain a floppy drive, you will likely need an external
power supply for either the floppy or the hard drive, as the one in the
PE-box cannot power both (unless you modify it, but this is another
kettel of fish). At least, this was the case with my Caviar-2540 hard
drive and the original TI disk drive.

Before installing the card, pick a CRU address for it and select it on
the rotary selector, using a small screwdriver to turn the knob. Make
sure the address is unique, i.e. does not conflict with any other card.
If you don't know what CRU addresses are in use in your system, download
my [Module Explorer package](download.htm#modexp), start the program and
press Ctrl-7 then Fctn-3 to review existing CRU addresses. Then pick one
that is free.

Place the other switches in the required position for the system you are
using, TI-99/4A or Geneve. The switch at the back has 3 positions: off
(all the way to the front) - Geneve (middle) - TI-99/4A (all the way
back). The switch in the center of the board has only two positions:
TI-99/4A to the front, Geneve to the back. The tiny DIP switches at the
bottom front of the card are only used with the RTC-65271 clock chip.
They should be left open with the other clocks.

The very first time you install your new board, do a smoke test: remove
all other cards from the PE-box, including the connection card. Turn
power on for a few seconds, then turn it off. No chip should be hot on
the IDE board. Then leave power on for a minute or two. The voltage
regulator will get hot but there should not be any "burnt" smell...

###  IDE card circuitery

#### Power supply

                   TL780-05                        All chips
                +----------+      +5V              +-----+
+8V-----+-------|Vin   Vout|---+-----... ---+------|Vcc  |    
        |       |   Ref    |   |            |      |     |
        = 47 uF +----------+   = 0.1 uF     =0.1uF |     |
        |            |         |            |      |     |
       Gnd           Gnd       Gnd     Gnd--+------|Vss  |
                                                   +-----+

Nothing special here: the TL780-05 voltage regulator can supply 1.5 Amp
at +5 volts. Make sure you use a large heat sink: it's gonna heat a lot.
To filter transients, each chip has a 100 nF cap connected accross its
supply pins.

####  Bus buffering

As recommended by Texas Instruments, all lines to/from the peripheral
bus are buffered on-card. This is done with three 74ALS245 for the
address bus (16 bits) and some of the control lines: MEMEN\*, CRUCLK,
RESET\*, DBIN, and WE\*. I could have used '244s, but the pinout of the
'245 is more convenient.

The data bus, of course requires a bidirectional 74ALS245 buffer. The
DIRection is controlled by the DBIN signal from the peripheral bus, the
ENable pin by a signal generated by the card (see [below](#DMA)).

The selection signal is also connected via a 74LS125 to the RDBENA\*
line of the peripheral bus to enable the drivers in the connection card
and the console-end of the cable. A 74LS125 is necessary, so as not to
hold the line high when we are not using it, as another card may need
it. The CardSel\* signal enables the '125 which input is connected to
the ground, the rest of the time, the 74LS125 is in high-impedance (i.e.
isolated) state.

The only line that is not buffered in the CRUIN line that comes directly
from the 74ALS251 (this chip has a 3-state output anyhow).

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
           +-----+                        220 Ohm
     A8&gt;---|     |---&gt;A8              Gnd---WWW---,
      .    |     |    .                           |    
      .    |     |    .                      LED  ^     
      .    |     |    .                   &#39;259    |           &#39;251
      .    |     |    .                +-------+  +-Bit0    +-------+      
      .    |     |    .    ,-----------|RST*   |  |         |       |
     A15&gt;--|     |---&gt;A15  |    A12----|S2   Q0|--&#39;      ---|A0   S2|---A12 
      Gnd--|EN*  |         |    A13----|S1   Q1|--Bit1   ---|A1   S1|---A13 
           +-----+         |    A14----|S0   Q2|--Bit2   ---|A2   S0|---A14
            &#39;244           |           |     Q3|--Bit3   ---|A3     |  
           +-----+         |    A15----|D    Q4|--Bit4------|A4    Q|---CRUIN
 RESET*&gt;---|     |---------&#39;   &#39;32     |     Q5|--Bit5------|A5     | 
CRUCLK*&gt;---|     |--------------=)&gt;----|G*   Q6|--Bit6      |A6     |  
    WE*&gt;---|     |---&gt;WE*      |       |     Q7|--Bit7      |A7   G*|---,   
  MEMEN&gt;---|     |---&gt;MEMEN*   |       +-------+            +-------+   |   
CLKOUT*&gt;---|     |             &#39;------------------------------,         | 
           |     |                           +-------+        |         | 
   DBIN&gt;---|     |---,                  A0---|G* A=B*|--------+---------&#39;     
      Gnd--|EN*  |   |                  A4---|A3   B3|-----------+---WWW---,    
           +-----+   |                  A5---|A0   B0|----------+|---WWW---+-Gnd
            &#39;245     |                  A6---|A2   B2|---------+||---WWW---+    
           +-----+   |                  A7---|A1   B1|--------+|||---WWW---&#39;  
           |  DIR|&lt;--&#39;                 Gnd---|A4   B4|--Gnd   ||||   4k7
    D0&lt;---&gt;|     |&lt;--&gt; D0              Gnd---|A5   B5|---A1  +----+             
     .     |     |      .              Gnd---|A6   B6|---A2  |1248|          
     .     |     |      .              +5V---|A7   B7|---A3  |++++|---+5V 
     .     |     |      .                    +-------+       +----+
     .     |     |      .                    74LAS688        Rotary
     .     |     |      .                                    encoder            
     .     |     |      .                                                   
    D7&lt;---&gt;|     |&lt;--&gt; D7                                                   
           |  OE*|&lt;------,                                                      
           +-----+       +----------&lt; CardSel*                            
                         |                                                  
RDBENA*&lt;-----------------&lt;|--Gnd
                       &#39;125  

####  CRU logic

The CRU logic is pretty standard. A 74ALS688 comparator is used to
compare the CRU address to a value set with a rotary encoder. The
comparator also considers address lines A0 to A3 and make sure they code
for \>1000-1FFF. The encoder sets the second digit in the CRU address:
\>1x00.

The A=B\* output of the 74ALS688 is used to enable the 74ALS251 that
performs CRU input. The same signal is further combined with CRUCLK\*
via an OR gate to enable the 74ALS259 that accepts CRU output from the
peripheral bus. The OR gate is necessary so that CRU input operations
don't alter the contents of the '259 (the opposite circuit is not needed
on the '251 because the TMS9900 CPU just ignores the CRUIN line during
CRU output operations).

Most CRU bits have different meanings in input vs output. However, bits
4 and 5 are reflected on themselves by connecting each output of the
'259 to the corresponding input of the '251.

####  Address decoding

       +-------+                          +-------+                 +-------+
 Gnd---|G* A=B*|--------------------------|S0  Y0*|--+--------------|G*  Y0*|-----------XRAMsel*
  A7---|A0   B0|--,        Bit0---,       |    Y1*|--|-+--&gt;SRAMsel* |    Y1*|-----------RTCsel*
  A6---|A1   B1|--+  A2--+--------==)&gt;o---|S1     |  | |            |       |
  A5---|A2   B2|--+      &#39;---|&gt;o--, &#39;02   |       |  | |     A10&gt;---|S0     |      22 Ohms
  A4---|A3   B3|--+          &#39;04  |       |       |  | |      A9&gt;---|S1  Y2*|--+-----WWW---CS1Fx* 
  A3---|B7   A7|--+-Gnd    Bit4---==)&gt;o---|S2     |  &#39;-==|)--,      |    Y3*|--|-+---WWW---CS3Fx*
  A2---|B6   A6|--+             Memen*----|G2A*   |     &#39;08  |      |       |  | |
 Gnd---|B5   A5|--&#39;                  A1---|G1     |          |      |       |  &#39;-==|)-+-IDEsel*
Bit1---|B4   A4|----+---o o---Gnd      ,--|G2B*   |      CardSel*   |       |     &#39;08 |
       +-------+    |   4k7            |  +-------+                 +-------+         V LED
         Bit1in-----+---WWW--- +5V     |                                  +5V---WWW---&#39;
                                       /           Key:                         220
                               +5V---o o o---A0    Off  Geneve  TI
                         +-------+     |
                    A0---|G2A*   |     |
             AMA---------|S0  Y7*|-----&#39;
             AMB---------|S1     |
      4k7    AMC---------|S2     |
Gnd---WWW----AMD---------|G2B*   |
+5V---WWW----AME---------|G1     |
   Optional              +-------+
                         74ALS138

The heart of the address decoding logic is the 74ALS138 3-to-8 decoder,
which I am using as a 1-to-2 decoder with multiple enabling lines. It is
enabled by MEMEN\*, A1 and A0 (indirectly), i.e. for memory operations
in the range \>4000-7FFF. Line A2 is combined with CRU bits 0 and 4, so
as to select outputs Y0\* and Y1\* in two cases:

When CRU bit 0 is high and A2 is low (address range \>4000-5FFF, i.e.
card ROM space)

When CRU bit 4 is high and A2 is high (address range \>6000-7FFF, for
RAMBO banks)

The 74ALS688 is an 8-bit comparator whose A=B\* output is active low
when the address is in the range \>4000-40FF and CRU bit 1 matches the
DIP-switch. Line A2 is included in the comparison, so that the '688
never gets active in the RAMBO space. The DIP-switch allows you to
decide whether the SRAM or the clock XRAM (for RTC-65271 only) should be
present at power-up time. As CRU bits are reset to 0 upon power-up, the
clock XRAM is selected when the switch is closed, the SRAM when the
switch is open. The status of this switch can be read with CRU bit 1, so
the software always know how to switch the SRAM on (when output does not
match input) or off (when bit 1 output mirrors its input). If you are
using any other clock chip, make sure the switch is always open.

The A=B\* output of the '688 selects either Y0\* or Y1\* of the '138.
Both are combined with an AND gate to produce the CardSel\* signal that
is used to enable the data bus. Y1\* activates the SRAM, Y0\* activates
a 74ALS139 decoder that decides which of the 4 extra devices is
accessed. Lines A9 and A10 are used for this selection:

Addresses \>4000-401F correspond to the RTC-65271's extended RAM (128
pages of 32 bytes)

Addresses \>4020-403F access the real-time-clock registers (see
[below](#Clock%20logic))

Addresses \>4040-405F access the first set of registers in the IDE
controller

Addresses \>4060-407F access the second set.

Addresses \>4080-40FF just repeat the above pattern (only the RTC-65271
uses them differently).

Note that the two IDE access lines are combined through an AND gate, to
provide an IDEsel\* signal for the bus demultiplexer. Just for the fun
of it, this signals also shines a second LED, that lets you know when a
drive is accessed. This otional LED can be any color, and is mounted at
the rear of the card. By contrast the yellow LED triggered by CRU bit 0
should be mounted so as to shine through the tiny window, in front of
the PE-Box.

You may wonder why I have a SP3T switch on the G2B\* input of the '138.
It was done so you can disable the card by connecting this input to +5
volts. This may come handy when the DSRs get messed up and prevent the
TI-99/4A from booting. In addition, the middle position of the switch
connects it to yet another 74ALS138 which decodes Geneve-specific
addresses. For the card to work properly with the Myarc "Geneve", it is
necessary to further decode five extra address lines: AMA (pin \#46 of
the PE-box bus), AMB (pin \#45), AMC (pin \#48), AMD (pin \#8, for use
with Genmod) and AME (pin \#9, Genmod).

Note that , if you have a Geneve but did not perform the "Genmod"
modification, you will need to pull line AMD low and line AME high. This
can be done easily by installing two extra 4.7 Kohms resistors. These
resistors will make your PE-box GenMod-like, so if you have other cards
with similar decoding logic (e.g. my USB-SM card), you will not need to
pull AMD nor AME on these cards: one card is enough. If you do have
GenMod, you do not need to install these two resistors.

####  DMA support

This is intended to work with a yet-to-be-designed DMA controller card,
which will double as a memory expansion card and replace the PE-box
connection card.

Since there aren't enough spare lines in the PE-box to implement DREQ
and DACK lines for the 4 devices typically handled by a DMA controller
(let alone for 7, handled by two controllers), these lines are
multiplexed with the data bus and the address bus, respectively. The
only DMA-dedicated lines are DmaEn\* and EOP\*, which correspond to the
unused SenilB\* and SenilA\* lines in the PE-box. EOP\* is not used
here, because the IDE port does not support it.

D1----------------&lt;|------+-----WWW---------&lt;IDE/DMAQ
         74LS04   |       &#39;-----WWW---Gnd
RdbEna*-+---|&gt;o---&#39;             10K
        |         ,-------------------,
        &#39;---------+--&lt;|---Gnd         |
                     |                |
                 ,---+---CardSel*     |
     245/G*---(|=---,           ,-----&#39;
             &#39;08    +--------&lt;(=---,
                 ,--&#39;        &#39;32   |
     139/G*---(|=-----IDEsel*      |
                                   |
            74ALS74                |
           +-------+               |  22 Ohms
A8---------|D1   Q1|---------------+--WWW---&gt;IDE/DMACK
           |       |
DmaEn*---+-|Ck1    |
         &#39;-|Ck2    |
         ,-|Pr1*   |
Reset*---+-|Pr2*   |
         ,-|Clr1*  |
    +5V--+-|Clr2*  |
           +-------+         

When the data bus is not in use, the IDE controller uses it to output
its DMA request signal DMAQ on one of the data lines (on the PE-box side
of the 74ALS245 buffers), via a spare 3-state gate in the 72LS125. The
RdbEna\* signal enables this gate when it's high, i.e. when the data bus
is inactive. The data line used here, D1, selects DMA channel 1.

The DMA acknowledgment signal from the DMA controller is to be found on
the address bus, lines A8 through A14, when the dedicated line DmaEn\*
changes from low to high. A 74ALS74 is used to latch address line A8
(channel 1) and thereby issue an active low DMACK signals to the
IDEcontroller. The latched bit is reset to "high" upon power-up by
applying the Reset\* signal to the Pr\* pins of the 74ALS74. The Clr\*
lines are disabled by connecting them to +5 volts.

The DACK signal is also masked with the PE-box RdbEna\* signal via an OR
gate and combined with either the CardSel\* signal to enable the
74ALS245 bus buffers, or the IDEsel\* signal to enable the
demultiplexer. During regular memory access, CardSel\* brings RdbEna\*
low via a 74LS125 gate, and enables the data bus via one of the AND
gates. If the memory address is that of an IDE port, the IDEsel\* signal
will enable the demultiplexer via the decond AND gate, connected to the
74ALS139.

During DMA operations, both CardSel\* and IDEsel\* will remain inactive.
However, the DMACK signal from the 74ALS74 has the ability to enable
both the 74ALS245 data bus buffer and the 74ALS139 controlling the
demultiplexer. But this only happens when RdbEna\* is low, a condition
controlled by the DMA controller.

####  SRAM access

An external SRAM is not necessary if you are using the pb4842 or the
bq4852, since these so-called "time keeping SRAM" contain both SRAM and
a real-time clock. However, you will need an external SRAM if you are
using the RTC-65271 or the bq4847 as your clock chip.

                                           ||||||   +----------+   ,-- ---+5V
                                74ALS373    10 K    |       Vcc|---+----bq4847/Vout
                                +-------+  ||||||   |          |
                           A9---|D2   Q2|--++++++---|A18     D7|---D0
                            .   | .   . |           | .      . |    .
                            .   | .   . |           | .      . |    .
                            .   | .   . |           | .      . |    .
                         A14----|D7   Q7|-----------|A13     D0|---D7
                                |       |     A3----|A12       |     &#39;04
SRAMsel*----, &#39;32        &#39;02    |       |      .    | .     RD*|-----o&lt;|---DBIN
WE*---------==)&gt;-------==)&gt;o----|LE     |      .    | .        |       ,----Bit5
CRU bit 2---|&gt;o--------&#39;        |       |      .    | .     WE*|---&lt;(==----&lt;WE*
           &#39;04  &#39;32     &#39;02     |       |     A15---|A0        |   &#39;32
         A3---==)&gt;----==)&gt;o-----|OE*    |           |       CS*|---+----bq4847/CEout*
         A2---&#39;       |         +-------+           +----------+   &#39;- --SRAMsel*
  CRU bit 3-----------&#39;                                              J2

The SRAM data lines are connected to the data bus. Note that Texas
Instruments numbers the bits the opposite way to everyone else: D0 (or
A0) is the most significant bit. That's why I connected D0 to D7, D1 to
D6, etc. This is not critical for the SRAM, but it is for all other
devices!

The SRAM address lines are driven (in part) by the address bus, lines A3
through A15. This provides us with a \>2000-byte access window. As the
SRAM is much larger than 8K, it has extra address lines. These serve as
"page selection" lines and are set by a 74ALS373 latch.

The 74ALS373 latches the address bus, lines A9 through A14, according to
a convention set by the cartridges ROMs. The latching occurs when one
writes to the SRAM while CRU bit 2 is set to one (which it isn't at
power-up time). To this end, a selection circuit combines SRAMsel\* and
WE\* via an OR gate and combines the resulting signal with the inverse
of CRU bit 2, via a NOR gate.

An additional and critical feature is the little circuit that controls
the OE\* (output enable) pin of the '373. When this pin is high, the
outputs of the '373 are in high impedance state, so the corresponding
address lines will reflect a default state. The six 4.7K resistors
grounding these pins ensure that page 0 will be selected in this
situation.

An OR and a NOR gate ensure that these outputs are only disabled when an
access occurs in the range \>4000-4FFF (i.e. A2 and A3 are low), and CRU
bit 3 is 0. This is critical so that a default DSR page is always
selected upon power-up and won't be paged off when the user switches
RAMBO pages. Also, it allows routines in the \>4000-4FFF memory space to
switch pages in the \>5000-5FFF and \>6000-7FFF area without kicking
themselves out of memory. The cartridge ROMs do this by having a copy of
the \>6000-6FFF area at the bottom of each page, but wasting all this
memory is kind of a shame! The above hardware trick allow us to freeze
the bottom half of page 0, while still paging the top part.

Finally, the WE\* input pin is controlled by the WE\* line, after due
masking by CRU bit 5. When this bit is set to '1', it prevents any
writing to the SRAM. This "ROM emulation" feature is usefull to switch
banks without altering the content of the SRAM.

When used with the bq4847, the SRAM receives both its CS\* input and its
power supply from dedicated pins of the clock chip, which provides
battery backup when power is off. This way, the DSRs remain permanently
in memory. This possibility does not exist with the RTC-65271, which
loads the DSRs from disk upon power-up. In the latter case, jumpers J1
and J2 should be closed with a piece of wire, so as to bypass the absent
bp4847.

####  The RTC-65271 clock

                       +----------+
                  A8---|A5      D7|-----D0
                 A15---|A4        |      .
                 A14---|A3        |      .
                 A13---|A2        |      .
                 A12---|A1        |      .
                 A11---|A0        |      .
         &#39;125          |        D0|-----D7
IntA*&lt;----&lt;|---Gnd     |          |
          +------------|INTRQ*    |   &#39;04
 251/A2---&#39;            |       RD*|---o&lt;|---DBIN
                       |       WR*|---------WE*
         RTCsel*------&gt;|RTC*      |
        XRAMsel*------&gt;|XRAM*     |
Reset*---o o------+---&gt;|Reset*    |
            4k7   |    +----------+
       +5V--WWW---&#39;

The RTC-65271 chip serves two functions: it contains a real-time clock
(RTC) similar to the standard MC146818 clock found on the PC computers,
and it contains 4 Kbytes of battery-backed RAM, accessible as 128 pages
of 32 bytes. This is a nice feature because it allows us to store the
DSRs in the clock RAM, without the need for a battery-backed SRAM. At
power-up time, the DSRs will be copied into the SRAM, since it is not
very convenient to switch pages every 32 bytes!

The clock data lines must be connected to the data bus in reverse order,
i.e. D7 to D0, D6 to D1, etc. The address lines must be connected as
shown above. Under these conditions, the extended RAM maps at
\>4000-401F, and the page selection register (controlled by pin A5,
address line A8) at \>4080.

The RTC contains 64 registers that are accessible via two ports. First
you write the number of the desired register at \>4030, then you can
read or write to this register at \>4020. (NB: Only one address line,
A11, is considered during RTC access, so the remaining addresses map to
the same ports).

The WR\* pin is connected to WE\*, the RD\* pin to the inverse of DBIN.
The two chip select lines, RTC\* and XRAM\*, are connected to the
outputs of the 74ALS139 in the address selection logic. They select the
Real-Time-Clock or the eXtended-RAM respectively.

Finally, the RTC chip can generate interrupts. These will pull down the
IntA\* line of the peripheral bus, via a 74LS125 buffer whose input is
grounded. When the INTRQ\* pin is high, the '125 is in high impedance
and won't affect the IntA\* line. The clock interrupt line is also
connected to an input of the 74ALS251, so you can read the CRU to check
whether an interrupt came from the clock.

Because interrupts can occur at any time, even when the computer is off,
there is a risk that the system becomes unbootable: as soon as you turn
the console on it sees the interrupt and tries to answer it. But if
IDEAL is not loaded yet, it won't clear the interrupt condition and the
system will keep scanning cards, in between each GPL instruction. To
prevent this from happening, the Reset\* line is connected to the
Reset\* pin of the clock chip. This will disable all interrupts when the
console is powered on. This feature can be disabled by opening the DIP
switch in the Reset\* line. For instance, if you have a battery-backed
SRAM, you may be reasonnably sure that IDEAL will always be loaded and
ready to handle interrupts.

####  The bq4847 clock

                       +----------+         |
                       |      Vout|---------|Vcc
           SRAMsel*----|CEin CEout|---------|CS*
                       |          |         +-------
                 A15---|A0      D7|-----D0    SRAM
                 A14---|A1        |      .
                 A13---|A2        |      .
                 A12---|A3        |      .
         &#39;125          |        D0|-----D7
IntA*&lt;----&lt;|---Gnd     |          |
          +------------|INTRQ*    |    &#39;04
 251/A2---&#39;            |       RD*|---o&lt;|-----DBIN
                       |WDI    WR*|-----------WE*
                       |WDO    CS*|---RTCsel*
                       |          |
                       |RESET*    |
                       +----------+

The bq4847 has an 8-bit data bus, and only 4 address lines (i.e. 16
clock registers). Again, data and address pins must be connected to the
TI-99/4A lines in reverse order, because of TI's numbering convention.
The WR\* pin is connected to WE\*, the RD\* pin to the inverse of DBIN.
The chip select line CS\* comes from the 74ALS39.

In addition, the clock chip provides baterry-backed power to the SRAM
and intercepts its selection signal. This ensures that the SRAM will
never be selected when power is off (CS\* is active low !), which would
quickly drain the battery.

This clock chip can generate interrupts. These will pull down the IntA\*
line of the peripheral bus, via a 74LS125 buffer whose input is
grounded. When the INTRQ\* pin is high, the '125 is in high impedance
and won't affect the IntA\* line. The clock interrupt line is also
connected to an input of the 74ALS251, so you can read the CRU to check
whether an interrupt came from the clock. The chip can also issue a
reset signal during power-up, or use an external "watchdog" timer, two
features that are not used with this board.

####  The bq4842 and bq4852 chips

These two chips are very similar: both contain a real-time clock and
some SRAM. The difference is in the size of the SRAM: the bq4842 offers
only 128 Kbytes, whereas the bq4852 has 512 Kbytes. This requires
additional pins, so the bq4852 has 36 pins, versus 32 for the bq4842.

                                           ||||||   +----------+
                                74ALS373    10 K    |          |
                                +-------+  ||||||   |          |
                           A9---|D2   Q2|--++++++---|A18     D7|---D0
                            .   | .   . |           | .      . |    .
                            .   | .   . |           | .      . |    .
                            .   | .   . |           | .      . |    .
                         A14----|D7   Q7|-----------|A13     D0|---D7
                                |       |     A3----|A12       |     &#39;04
SRAMsel*----, &#39;32        &#39;02    |       |      .    | .     RD*|-----o&lt;|---DBIN
WE*---------==)&gt;-------==)&gt;o----|LE     |      .    | .        |       ,----Bit5
CRU bit 2---|&gt;o--------&#39;        |       |      .    | .     WE*|---&lt;(==----&lt;WE*
           &#39;04  &#39;32     &#39;02     |       |     A15---|A0        |   &#39;32
         A3---==)&gt;----==)&gt;o-----|OE*    |           |       CS*|--------SRAMsel*
         A2---&#39;       |         +-------+           |          |
  CRU bit 3-----------&#39;     IntA*&lt;---------&lt;|---Gnd |      Rst*|-
                                           +--------|Int*      |
                                  251/A2---&#39;        +----------+                          

The address bus is 8-bit wide, D7 being the most significant bit.
Therefore, it is connected to D0 of the TI-99/4A, and conversely.
Similarly, the address pins A0 through A12 are connected to the TI-99/4A
address lines A15 through A3, respectively. The extra address lines,
four with the bq4842, six with the bq4852, are controlled by the
74ALS373 which latches the SRAM page (see description of this feature in
the SRAM section [above](#SRAM%20logic)). Note that, because A15 is
required by the SRAM part of the chip, the clock registers will always
be accessed two at a time. This is because the TI-99/4A always performs
word-wide access, even when you use a MOVB instruction.

The OE\* pin is controlled by the inverse of DBIN, and the WE\* by the
WE\* signal masked with CRU bit 5. This is needed to write-protect the
SRAM., but note that the clock portion of these chips will also be
write-protected, contrarily to what happens with the other clock chips,
RTC-65271 and bq4847.

The CS\* pin is controlled by the SRAMsel\* line from the 74ALS138, so
the whole chip, including the RTC portion, will be accessed as SRAM.

The RTC part of the chip can generate interrupts. These will pull down
the IntA\* line of the peripheral bus, via a 74LS125 buffer whose input
is grounded. When the Int\* pin is high, the '125 is in high impedance
and won't affect the IntA\* line. The clock interrupt line is also
connected to an input of the 74ALS251, so you can read the CRU to check
whether an interrupt came from the clock. The chip can also issue a
reset signal upon power-up, and has an internal watchdog timer, but
these features is not used with this board.

####  IDE port access

The IDE port consists in 10 registers, that map as two separate blocks.
The two blocks are enabled by CS1Fx and CS3Fx, controlled by two outputs
of the 74ALS139 in the address selection logic (see
[above](#Addressing)). CS1Fx maps at \>4040-405F and CS3Fx maps at
\>4060-407F. The register number is passed to the IDE controller on its
A0-A2 lines, from the address lines A14-A13 (in reverse order because A2
is the msb for the IDE controller).

All registers are 8 bits wide, except the data register that is 16-bit.
For this reason, we must demultiplex the peripheral data bus, and bring
it back to what it was in the console. As you know, the TI-99/4A console
also has a 16-bit data bus, but the bus is multiplexed as 2 x 8 bits on
the side port (talk about a short-sighted design decision...). To
demultiplex the data bus, we'll make use of two 74F543. These nifty
chips incorporate a bidirectional bus transceiver with two latches, one
on each input. Both chips have their 'A' pins connected to the 8-bit
data bus, and their 'B' pins together form the 16-bit bus. The OExx\*
pins enable output in the indicated direction (e.g. OEAB\* in the A-\>B
direction), whereas the LExx\* pins latch data when low and retain it
when high. Both chips are permanently enabled by tying their CEAB\* and
CEBA\* pins to the ground.

If you compare these schematics with those of my first IDE prototype,
you will notice that I swaped the bytes in the 16-bit data bus. I did
this so that it will be easier to read PC-formated drives, i.e. without
having to SWPB bytes upon each rear/write operation.

Note that there is an optional 10K pull-down resistor on D7: this serves
to detect whether a drive is connected to the card or not. Currently,
the DSRs do not use this feature, though.

                                                 +-------+        10K
                                           D0&gt;---|A0   B0|---D7---WWW---Gnd   
                                            .    |       |    .       
                                            .    |       |    .           
                                            .    |       |    . 16-bit bus  
                                            .    |       |    .    (LSB)
                                            .    |       |    .    
                                            .    |       |    .    
                                           D7&gt;---|A7   B7|---D0    
                           T     WE*---,    Gnd--|CEAB*  |       
                      ,----o-----------=)&gt;-------|LEAB*  |       
           74ALS139   |    o----+----------------|OEAB*  |       
           +-------+  |  G o    |           Gnd--|CEBA*  |       
    Dbin---|S0  Y0*|--&#39;    |    |  ,-------------|OEBA*  |       
     A15---|S1  Y2*|--+----&#39; G  |  |  ,----------|LEBA*  |       
           |    Y1*|--|------o--|--&#39;  |          +-------+  A11---,    22 Ohms
CardSD*----|G*  Y3*|--|--,   o--|-----+---------------------------=)&gt;---WWW---&gt;IDE/RD*
           +-------+  |  +---o  |     |             74F543 (odd)          
                      |  |   T  |     |          +-------+   
                      |  |      |     |    D0&gt;---|A0   B0|---D15   
                      |  |      |     |     .    |       |    .       
                      |  |      |     |     .    |       |    .           
                      |  |      |     |     .    |       |    .  16-bit bus   
                      |  |      |     |     .    |       |    .    (MSB)
                      |  |      |     |     .    |       |    .    
                      |  |      |     |     .    |       |    .    
                      |  |      |     |    D7&gt;---|A7   B7|---D8    
                      |  |      |     |     Gnd--|CEAB*  |       
                      |  |      |     &#39;----------|LEBA*  |       
                      |  &#39;------|----------------|OEBA*  |       
                      |         |  WE*---,  Gnd--|CEBA*  |       
                      &#39;---------|--------=)&gt;-----|LEAB*  |       
                                +----------------|OEAB*  |
                                |                +-------+   22 Ohms   
                          WE*---=)&gt;---------------------------WWW------&gt;IDE&gt;WR*
                                                       A12------------&gt;IDE/A2
                                                       A13------------&gt;IDE/A1
                                                       A14------------&gt;IDE/A0
                                 &#39;04            Bit7------,  &#39;32 
                &#39;125      Bit6---|&gt;o--+------------------=)&gt;----------&gt;IDE/RESET*
 IntA*&lt;---------&lt;|---Gnd       ,------&#39;                      22 Ohms
                &#39;-----------&lt;(=-------o&lt;|--+-------------------WWW---&lt;IDE/IRQ
                           &#39;32             |
                                 251/A0----+---WWW---Gnd    4.7K
                                               10K      ,---WWW---+5V
                                 251/A3-----------------+------------&lt;IDE/IORDY

The second half of the 74LS139 that was used for address decoding serves
to distinguish four possible situations, thanks to the wiring of its
selection inputs to A15 (S1) and DBIN (S0).

Y0\*: write even byte (MSB, 2nd on TI-99/4A, 1rst on Geneve)
Y1\*: read even byte
Y2\*: write odd byte (LSB, 1rst on TI-99/4A, 2nd on Geneve)
Y3\* read odd byte

The TI-99/4A console always deals with the odd, least significant byte
first; then with the even, most significant byte. However, when dealing
with 16-bit devices we want only one such operation to occur. When
reading, we must read both bytes, latch the even byte for further use,
and pass the odd byte directly to the 8-bit bus. When writing, we must
latch the odd byte first, then pass both bytes to the 16-bit bus,
together with the WR\* signal.

By contrast, the microprocessor inside the Myarc Geneve, the TMS9995,
always deals with the even byte first, then the odd byte. So that the
card can be used with both a TI-99/4A and a Geneve, a DPDT switch was
included. In the "T" position, the switches set the demultiplexer for
use with a TI-99/4A, and the "G" position with a Geneve. I'm indebted to
Jeff White for coming up with this nifty modification of my original
design.

For write operations, the even byte is latched in the A-\>B direction by
the Y0\* signal, combined with the WE\* write pulse via an OR gate. This
is necessary to ensure that the data bus will still be valid when
latching occurs (as it's not guaranteed to be when A15 toggles).
Similarly, the odd byte is latched by a combination of WE\* and Y2\*.

The first half of the DPDT switch serves to determine which byte is
written last: the even one with the TI-99/4A (Y0\* signal) or the odd
one with the Geneve (Y2\* signal). The selected signal enables both
74F543 for output in the A-\>B direction, thereby presenting a 16-bit
word on the 16-bit databus. The same signal is combined with WE\* via an
OR gate to provide the WR\* write pulse to the IDE port.

For read operations, the second half of the DPDT switch determines which
byte is read first: the odd byte with the TI-99/4A (Y3\* signal), the
even byte with the Geneve (Y1\* signal). The selected signal causes both
74F543 to latch their respective half of the 16-bit data bus in the
B-\>A direction. It also makes up the RD\* signal sent to the IDE
controller. Technically, it's a bit dangerous to control the latches
with the RD\* signal, as they will latch data when RD\* goes inactive,
at which time the data may not be available any more! But it turned out
that most IDE controllers hold the data long enough after RD\* goes high
for the 74F543 to latch it properly. Besides, the extra OR gate on RD\*
introduces a slight delay that gives us a safety margin.

The RD\* input is combined with A11 via an OR gate, so as to restrict
read operations to the range \>4040-404F and \>4060-406F. This is
required because the TMS9900 CPU performs a read before each write. The
IDE controller does not expect this and may not answer properly to write
operations if they were intermingled with reads (although I did not try
this). Masking RD\* with A11 ensures that no read operation will occur
in the range \>4050-405F nor in \>4070-407F, which is where we'll
perform the write operations. The true reads will be done at \>4040-404F
and \>4060-406F.

The latched odd byte is made available for reading by the Y1\* signal,
which enables the "odd" 74F543 for output in the B-\>A direction.
Similarly, the even byte is made avilable by the Y3\* signal.

Finally, the active high IRQ pin is pulled down with a 10K resistor and
connected to input A0 of the 74ALS251 chip, so that it can be read with
CRU bit 0. The signal can also generate an interrupt by pulling down the
IntA\* line through a 74LS125, but it is masked by CRU bit 6 (inverted).
This way, IDE interrupts are disabled by default, and can be enabled by
setting CRU bit 6 to '1'.

The IORDY pin is connnected to input A3 of the 74ALS251, so CRU bit 3
can be read to determine when the controller is ready. It is pulled up
to +5 volts with a 4.7K resistor.

The Reset\* pin is controled by CRU bit 7, masked by the inverse of CRU
bit 6, and is used to perform a hardware reset. A reset condition exists
when bit 6 is '1' and bit 7 is '0'.

The 22 ohms resistors on lines CS1Fx, CS3Fx, RD\*, WR\*, DMACK\*, DMAQ
and IRQ are here to try and decrease noise on these lines, as per ATA-3
specifications (although, strictly speaking, the last two should be 82
ohms).

------------------------------------------------------------------------

### The RTC-65271 clock

[Pinout
](#Pinout%20RTC)[Batteries
](#Batteries)[Registers
](#RTC%20registers)[XRAM
](#XRAM)[Time & date
](#Time%20and%20date)[Alarms
](#alarms)[Periodic interrupts
](#square%20wave)[Timing diagrams
](#Timing%20diagrams)[Electrical characteristics](#Electrical)

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

The RTC contains 64 registers. To access a register, you must first pass
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

By setting bit \>20 in register B, you can cause the alarm to trigger an
external interrupt when it goes off. The IRQ will be brought low until
the alarm is acknowledged by reading register C. The IDE card sends this
signal to the IntA\* line, which causes an external interrupt. The card
SRAM should contain an interrupt service routine that clears the alarms
and takes the appropriate (user selectable) action.

####  Periodic interrupts

The clock can generate a square-wave signal on its SQW pin. To do this,
set bit \>80 in register B. The signal frequency is determined by
register A, bits \>08 to \>01. The IDE card does not use this signal.
The signal period is also the interval between two periodic interrupts
sent on pin IRQ\* if the PIE bit is enabled in register B.

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

### The bq4847 clock

[Pinout
](#pinout%20bq4847)[Registers
](#RTC%20registers%204847)[Time & date
](#Time%20and%20date%204847)[Alarms
](#alarms%204847)[Periodic interrupts
](#Periodic%20int%204847)[Power-fail interrupts
](#power-fail%20int%204847)[Watchdog timer
](#watchdog%204847)[Timing diagrams
](#Timing%20diagrams%204847)[Electrical
characteristics](#Electrical%204847)

The bq4847 is a fairly standard clock chip, with a built in crystal and
battery. The latter means that you cannot change the battery, but the
datasheet specifies that it should last about 10 years (130 mAh). In
addition, the chip has the necessary pins to battery-back an external
SRAM. This implies: 1) A power supply pin that sends +5 volts to the
SRAM when power is on, and switches to + 3 volts battery power when
power is off (i.e. lower than battery power, which is about +3 volts).
2) A "chip enable" signal buffer that remains high when power is off,
thereby desabling the SRAM.

#### Pinout

           +----+--+----+
      Vout |1 o       28| Vcc
      n.c. |2    R    27| WE*
      n.c. |3    T    26| CEin
      WDO* |4    C    25| CEout
      Int* |5         24| n.c.
      Rst* |6    6    23| WDI
        A3 |7    5    22| OE*
        A2 |8    2    21| CS*
        A1 |9    7    20| n.c.
        A0 |10   1    19| D7
        D0 |11        18| D6
        D1 |12        17| D5
        D2 |13        16| D4
       GND |14        15| D3
           +------------+

Power supply
**Vcc**: +5V
**GND**: ground

Control signals
**CS\***: Accessed the real-time clock registers when low.
**RD\***: Read data from a register when low.
**WE\***: Latch data into a register when low. RD\* and WE\* should
never be low together.

Busses
**A0-A3**: register address. A0 is the least significant bit.
**D0-D7**: Bidirectional data bus. D0 is the least significant bit, D7
the most significant one (opposite of the TI-99/4A).

Extra pins
**IRQ\***: the clock bring this open-collector output low when
issuing an interrupt (unless it is in stand-by or in battery-backed
mode). The load voltage should not excess Vcc. Leave n.c. when not in
use.
**RST\***: open-collector output. Remains low for 200 msec after power
goes on. Goes low if watchdog times out.
**WDI**: watchdog timer input. Internally connected to a voltage divider
by 100K resistors, about 1.6 volts.
**WDO\***: watchdog timer output.

SRAM control pins
**Vout**: Provides battery backed power for an external SRAM.
**CEin**: Input for the SRAM selection signal.
**CEout**: Output of the SRAM selection signal, will remain high when
power is off. Stays inactive for 100 ns after power goes on, then
reflects CEin.

#### Registers

The RTC contains 16 registers, each mapping at its own address. In most
cases, the internal data representation is in binary-coded decimal, i.e.
\>59 means 59, not 89 as it would in true hexadecimal.

**Register 0**: Seconds. Valid values: \>00 to \>59.

**Register 1**: Alarm seconds. As above. Add \>C0 to disregard seconds
in the alarm settings.

**Register 2**: Minutes. Valid values: \>00 to \>59.

**Register 3**: Alarm minutes. As above. Add \>C0 to disregard minutes
in alarm settings.

**Register 4**: Hours. Valid values: \>00 to \>23, and \>81 to \>92 (add
\>80 for pm).

**Register 5**: Alarm hours. As above. Add \>C0 to disregard hours in
alarm settings.

**Register 6**: Day of the month. Valid values: \>01 through \>31.

**Register 7**: Alarm day of the month. As above. Add \>C0 to disregard
day in alarm settings.

**Register 8**: Day-of-week. Valid values: \>01 (Sunday) to \>07.

**Register 9**: Month. Valid values \>01 to \>12.

**Register 10**: Year. Valid values \>00 to \>99.

**Register 11**: Programmable rates register

WD
RS

**WD**: Watchdog timeout rate (bit \>80 not used)
**RS**: Frequency of the periodic interrupt.

**Register 12**: Interrupt enable register

|      |      |      |      |      |      |       |      |
|------|------|------|------|------|------|-------|------|
| \>80 | \>40 | \>20 | \>10 | \>08 | \>04 | \>02  | \>01 |
| 0    | 0    | 0    | 0    | AIE  | PIE  | PWRIE | ABE  |

**AIE**: 1=Enable alarm interrupts.
**PIE**: 1=Enable periodic interrupts at a frequency determined by RS in
register 11.
**PWRE**: 1=Enable interrupts to signal power failure.
**ABE**: 1=Enable alarm interrupts even while on battery power.
The first 4 bits are not used.

**Register 13**: Flags register

|      |      |      |      |      |      |      |      |
|------|------|------|------|------|------|------|------|
| \>80 | \>40 | \>20 | \>10 | \>08 | \>04 | \>02 | \>01 |
| 0    | 0    | 0    | 0    | AF   | PIF  | PWRF | BVF  |

**AF**: 1=An alarm occured since the status was last read. Set whether
or not the interrupt is enabled by AIE.
**PF**: 1=A periodic interrupt occured since last time the status was
read. Set even if PIE is '0'.
**PWRF**: 1=A power failure interrupt occured. Set even if PWRIE is
'0'.
**BVF**: 1=Battery valid. 0=Battery voltage lower than 2.1 volts: time &
date (and external SRAM contents) may be incorrect.
The first 4 bits are not used.

**Register 14**: Control register

|      |      |      |      |      |        |       |      |
|------|------|------|------|------|--------|-------|------|
| \>80 | \>40 | \>20 | \>10 | \>08 | \>04   | \>02  | \>01 |
| 0    | 0    | 0    | 0    | UTI  | STOP\* | 24/12 | DSE  |

**UTI**: 1=Update Transfer Inhibit. The registers can be safely
read/written to. 0=Keep up with current values.
**STOP\***: 0=Stops RTC when power is off (e.g for storage, since
battery cannot be taken out). 1=Oscillator always on.
**24/12**: 0=24 hours representation. 1=12 hours (am / pm)
representation.
**DSE**: 1=Daylight savings time enable.
The first 4 bits are not used.

**Register 15:** not used. Reads as \>00.

####  Setting/reading the time and date

To read time and date, just read the relevant registers. The only
problem is that time may change as you are reading it (e.g. if its
2:59:59), so you may end up reading 2:00:00 if you read the hour before
the minutes. Thus, you should always set bit \>08 in register 14 before
reading, which will prevent register update. The clock still keeps the
correct time, but the "display" is frozen, so you can access it safely.
Once done, reset byte \>08 and the registers will catch up with the
current time and date.

You can enable summer-time (aka daylight saving time) by setting bit
\>01 in register 14. When in this mode, the clock automatically jumps
from 1:59:59 am to 3:00:00 am on the first Sunday in April. It jumps
back from 1:59:59 to 1:00:00 am on the last Sunday in October.
**Alarms**

You can set an alarm for a given time and date (whithin a month), by
writing a value into the four alarm registers: day, hour, minutes and
seconds. If a register is not used, set the two leftmost bits as 1 (i.e.
values in the range \>C0-FF). This way you can set repetitive alarms,
e.g. every hour on the hour (day = \>C0, hours = \>C0, minutes=0,
seconds=0). To disable alarms altogether, enter an impossible value in
one of the registers: e.g. minutes=64.

By setting bit \>08 in register 12, you can cause the alarm to trigger
an external interrupt when it goes off. The IRQ\* will be brought low
until the alarm is acknowledged by reading register 13. The IDE card
sends this signal to the IntA\* line, which causes an external
interrupt. The DSRs in the SRAM shoudl contain an interrupt service
routine that clears the alarms and takes the appropriate (user
selectable) action.

Setting bit \>01 in register 12 would allow an alarm to trigger an
interrupt even when power is off. Obviously, we have no need for this
feature, so leave this bit as '0'. You can find out whether an alarm
occured during power-off time by reading the flag register 13 at
power-up time. Note that the interrupt enable register is automatically
reset upon power-up, thus alarms interrupts need to be re-enabled after
each power-up.

####  Periodic interrupts

Optionally, the clock can generate constants interrupts at a preset
frequency. This interrupt can be detected by checking bit \>04 in the
flags register, or it can be used to trigger an external interrupt, if
bit \>04 is set in register 12. The interruption period is set with
register 11, according to the following table:

|     |           |                  |
|-----|-----------|------------------|
| RS  | Frequency | Interrupt period |
| 0   | \-        | \-               |
| 1   | 32768 Hz  | 30.5175 us       |
| 2   | 16384 Hz  | 61.035 us        |
| 3   | 8192 Hz   | 122.07 us        |
| 4   | 4096 Hz   | 244.141 us       |
| 5   | 2048 Hz   | 488.281 us       |
| 6   | 1024 Hz   | 976.5625 us      |
| 7   | 512 Hz    | 1.953125 ms      |
| 8   | 256 Hz    | 3.90625 ms       |
| 9   | 128 Hz    | 7.8125 ms        |
| 10  | 64 Hz     | 15.625 ms        |
| 11  | 32 Hz     | 31.25 ms         |
| 12  | 16 Hz     | 62.5 ms          |
| 13  | 8 Hz      | 125 ms           |
| 14  | 4 Hz      | 250 ms           |
| 15  | 2 Hz      | 500 ms           |

Note that only conditions 1 and 2 differ from those of the RTC-65271 and
those of the bq4842 and bq4852.

####  Power-fail interrupts

When Vcc falls below the battery power (about +3 volts), the clock
switches into backup mode: it disables the SRAM, etc. It also sets bit
\>02 in register 13, and may generate an interrupt if bit \>02 is set in
register 12.

####  Watchdog timer function

This function could be used to monitor a microprocessor and make sure
it's not frozen or traped inside a program loop. When the WRI pin is
connected, it must toggle low/high within a defined interval. If it
doesn't, WDO\* and Rst\* go low. Rst\* comes back high after one sixth
of the time-out period, then low again after one time-our period, etc.
By contrast, WDO\* stays low until WDI toggles. None of this happens if
WDI is left unconnected, which the chip detects thanks to the internal
voltage divider. The watchdog function is not used in the IDE board.

|     |                 |                |
|-----|-----------------|----------------|
| WD  | Timeout period  | Rst\*=low time |
| 0   | 1.5 s (default) | 250 ms         |
| 1   | 23.4375 ms      | 3.90625 ms     |
| 2   | 46.875 ms       | 7.8125 ms      |
| 3   | 93.75 ms        | 15.625 ms      |
| 4   | 187.5 ms        | 31.25 ms       |
| 5   | 375 ms          | 62.5 ms        |
| 6   | 750 ms          | 125 ms         |
| 7   | 3 s             | 500 ms         |

###  Timing diagrams

#### Read cycle

         |         >70 ns          |
         \___                      /        CS*
         |    _<70 ns |            |
         |     \______|________/   |<25|    OE*
         |     |  <35 |        |  <25  |
    ZZZZZZZZZZZZZ-XXXXX    valid       ZZZZ D0-D7
         | < 5 ns |

####  Write cycle

       |           >70  ns                 |
    XXX              valid                 XXXX A0-A5
    ___|>0 |                               |
       |   \___      >65 ns          /  >5 |    CS*
       |  >0    |               _____      |
                \____>55 ns____/|    >15   |    WE*
             |       >30 ns     |>10|
    ZXZZZZZZZZ     valid            ZZZZZZZZZZZ D0-D7

####  SRAM cycle
        \_________________/|                   CEin
    ____|9-12 ns|          |9-12 ns|
                \___               /           CEout

####  Power down/up

    _____
         \                                /___Vpfd(max)
    Vpfd__\                              /____Vpfd           Vcc
          |\                            /|
          | \___                   _   /-|----Vso
          |                              |
          |                              |100-300 ms |
    valid |  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX valid  CS*
          |                              |100-300 ms |
          |                   /XXXXXXXXXXXXXXXXXXXXXX\______ CEin
          |       |90-125 us  |    Vohb  |
    ______|___    |           /   '----' |             \____ CEout
          |____   |                      |
          |       |           \          |100-300 ms /       Rst* (+ pull-up)
          |6-24 us|           |               _____
                  \ 90-125 us /ZZZZZZZZZZZZZZ/               Int* (+ pull up)

Voltage slews:
4.75 volts down to 4.25 volts: \>300 us
4.25 volts down to Vso: \>10 us
Vso up to Vpfd(max): \>100 us

###  Electrical characteristics

#### Absolute maximum ratings

    Supply voltage Vcc:               -0.3V to 7V
    Input voltage:                    -0.3V to 7V
    Operating temperature:             0 to 70 `C
    Storage temperature:             -55 to 125 `C
    Soldering temperature: max 260 `C for 10 secs

####   Recommended operating conditions



####  Electrical characteristics under recommended conditions


Ditto on battery (Vohb)
Vcc &lt; Vbc I= -10 uA
Vbc-0.3
bq4847
4.55
4.62
4.75
Vcc &lt; Vbc I = 100 uA
Vbc-0.3
WDO* I source = 2mA
2.4

------------------------------------------------------------------------

### The bq4842 and bq4852 chips

[Pinouts
](#pinout%204842%20+%204852)[Registers
](#RTC%20registers%204842)[Time & date
](#Time%20and%20date%204842)[Alarms
](#alarms%204842)[Periodic interrupts
](#Periodic%20int%204842)[Power-fail interrupts
](#Power%20fail%20int%204842)[Watchdog timer
](#watchdog%204842)[Clock calibration
](#calibration%204842)[Timing diagrams
](#Timing%20diagrams%204842)[Electrical
characteristics](#Electrical%204842)

Both these chips combine a fairly standard real-time clock with built-in
battery and crystal, and a battery-backed SRAM. The clock registers map
at the end of the SRAM space. The only difference between the two is the
size of the SRAM: 128K for the bq4842, versus 512K for the bq4852. Thus,
with these chips, you don't need to install an external SRAM chip.

#### Pinouts

           +----+--+----+
      Rst* |1 o       36| Vcc
      n.c. |2         35| n.c.            +----+--+----+
       A18 |3 o       34| Int*       Rst* |1 o       32| Vcc
       A16 |4         33| A15         A16 |2         31| A15
       A14 |5         32| A17         A14 |3         30| Int*
       A12 |6    b    31| WE*         A12 |4    b    29| WE*
        A7 |7    q    30| A13          A7 |5    q    28| A13
        A6 |8         29| A8           A6 |6         27| A8
        A5 |9    4    28| A9           A5 |7    4    26| A9
        A4 |10   8    27| A11          A4 |8    8    25| A11
        A3 |11   5    26| OE*          A3 |9    4    24| OE*
        A2 |12   2    25| A10          A2 |10   2    23| A10
        A1 |13        24| CE*          A1 |11        22| CE*
        A0 |14        23| D7           A0 |12        21| D7
        D0 |15        22| D6           D0 |13        20| D6
        D1 |16        21| D5           D1 |14        19| D5
        D2 |17        20| D4           D2 |15        18| D4
       GND |18        19| D3          GND |16        17| D3
           +------------+                 +------------+

Power supply
**Vcc**: +5V
**GND**: ground

Control signals
**CE\***: Access the SRAM and the real-time clock registers when
low.
**OE\***: Read data while low. OE\* and WE\* should never be low
together/
**WE\***: Latch data when low (on the falling edge of WE\* or CE\*
whichever comes last)

Busses
**A0-A18**: register address. A0 is the least significant bit.
**D0-D7**: Bidirectional data bus. D0 is the least significant bit, D7
the most significant one (opposite of the TI-99/4A).

Extra pins
**Int\***: the clock bring this open-collector output low when
issuing an interrupt (unless it is in stand-by or in battery-backed
mode). The load voltage should not excess Vcc. Leave n.c. when not in
use.
**Rst\***: open-collector output. Remains low for 200 msec after power
goes on. Goes low if watchdog times out.

#### Registers

The RTC interface consists in 16 registers, each mapping to a dedicated
address at the end of the SRAM. With the bq4852, this will be addresses
\>7FFF0-7FFFF, which on the IDE card translates as \>5FF0-5FFF at page
\>3F. With the bq4842, the addresses are \>1FF0-1FFF, which translates
as \>5FF0-5FFF at page \>0F, \>1F, \>2F or \>3F (since pages \>00-0F
also map at \>10-1F, etc).

In most cases, the internal data representation is in binary-coded
decimal, i.e. \>59 means 59, not 89 as it would in true hexadecimal.

**\>7FFF0**: Interrupt flags. These flags are set whether or not a given
interrupt is enabled in register \>7FFF6. Reading register \>7FFF0 will
clear the interrupt condition and reset all flags.

|      |      |      |      |      |      |      |      |
|------|------|------|------|------|------|------|------|
| \>80 | \>40 | \>20 | \>10 | \>08 | \>04 | \>02 | \>01 |
| WDF  | AF   | PRWF | BLF  | PF   | x    | x    | x    |

**WDF**: 1=Watchdog timed out.
**AF**: 1=An alarm occured.
**PWRF**: 1=Main power went down.
**BLF**: 1=Battery is low.
**PF**: 1=A periodic interrupt occured.
The last 3 bits are not used, and can be written/read at will.

**\>7FFF1**: 100th of second. Valid values: \>00 to \>99.

**\>7FFF2**: Alarm seconds. Valid values: \>00 to \>59. Add \>80 to
disregard seconds in the alarm settings.

**\>7FFF3**: Alarm minutes. Valid values: \>00 to \>59. Add \>80 to
disregard minutes in the alarm settings.

**\>7FFF4**: Alarm hours. Valid values: \>00 to \>23. Add \>80 to
disregard hours in the alarm settings.

**\>7FFF5**: Alarm day of the month. Valid values: \>01 to \>31. Add
\>80 to disregard day in alarm settings.

**\>7FFF6**: Interrupts enable.

RS

**AIE**: 1=Enable alarm interrupts.
**PWRIE**: 1=Enable interrupts to signal power failure.
**ABE**: 1=Enable alarm interrupts even while on battery power.
**PIE**: 1=Enable periodic interrupts at a frequency determined by RS.
**RS**: Frequency of the periodic interrupt.

**\>7FFF7**: Watchdog register. To prevent watchdog from firing, write
to this register before the timeout period is over.

BM
WD

**WDS:** Watchdog steering upon timeout: 1=Pulse Reset\* low, then
disable whatchdog.
0=keep INT\* low till watchdog is reset (by writing to this register).
**BM**: Watchdog multiplier: timeout period, in units. 0=disable.
**WD**: Watchdog units: 00=1/16 sec, 01=1/4 sec, 10=1 sec, 11=4 sec

**\>7FFF8**: Control

Calibration

**W**: 1=Enable writing to clock registers \>7FFF9-7FFFF. 0=Accept new
values.
**R**: 1=Enable reading clock registers, i.e. freezes the register
contents. 0=Catch up with actual values.
**S**: Calibration sign: 0=negative, 1=positive.
**Calibration**: +1= + 10.7 sec/month. -1 = -5.35 sec/month. Compensates
for temperature effects on crystal accuracy.

**\>7FFF9**: Seconds. Valid values: \>00 to \>59. Add \>80 to stop the
oscillator (e.g. for storage, since battery cannot be removed).

**\>7FFFA**: Minutes. Valid values: \>00 to \>59.

**\>7FFFB**: Hours. Valid values: \>00 to \>23.

**\>7FFFC**: Day-of-week. Valid values: \>01 (Sunday) to \>07. Add \>40
to enable frequency test mode: bit \>01 in register \>7FFF9 will toggle
at exactly 512K (do not set the R bit). Usefull to adjust calibration
without timing the clock for a month.

**\>7FFFD**: Day of the month. Valid values: \>01 through \>31.

**\>7FFFE**: Month. Valid values \>01 to \>12.

**\>7FFFF**: Year. Valid values \>00 to \>99.

####  Setting/reading the time and date

To read time and date, just read the relevant registers. The only
problem is that time may change as you are reading it (e.g. if its
2:59:59), so you may end up reading 2:00:00 if you read the hour before
the minutes. Thus, you should always set bit \>40 in register \>7FFF8
before reading, which will prevent register update. The clock still
keeps the correct time, but the "display" is frozen, so you can access
it safely. Once done, reset byte \>40 and the registers will catch up
with the current time and date within the next second.

Similarly, to set the clock you should first set bit \>80 in register
\>7FFF8. Then write the desired values to the proper registers. Finally,
reset bit \>80 to '0', which will transfer the new values to the
internal clock registers. This is only necessary when writing to the
time registers, \>7FFF9 through \>7FFFF.

There is no automatic daylight savings time with this clock, so you will
need to adjust it yourself: one hour forward on the first Sunday in
April, one hour back on the last Sunday in October.
**Alarms**

You can set an alarm for a given time and date (whithin a month), by
writing a value into the four alarm registers: day, hour, minutes and
seconds. If a register is not used, set the two leftmost bits as 1 (i.e.
values in the range \>C0-FF). This way you can set repetitive alarms,
e.g. every hour on the hour (day = \>C0, hours = \>C0, minutes=0,
seconds=0). To disable alarms altogether, enter an impossible value in
one of the registers: e.g. minutes=64.

By setting bit \>80 in register \>7FFF6, you can cause the alarm to
trigger an external interrupt when it goes off. The Iint\* pin will be
brought low until the alarm is acknowledged by reading register \>7FFF0.
The IDE card sends this signal to the IntA\* line, which causes an
external interrupt. The DSRs in the SRAM should contain an interrupt
service routine that clears the alarms and takes the appropriate (user
selectable) action.

Setting bit \>20 in register \>7FFF6 would allow an alarm to trigger an
interrupt even when power is off. Obviously, we have no need for this
feature, so leave this bit as '0'. You can find out whether an alarm
occured during power-off time by reading the flag register \>7FFF0 at
power-up time. Note that the interrupt enable register is automatically
reset upon power-up, thus alarms interrupts need to be re-enabled after
each power-up.

####  Periodic interrupts

Optionally, the clock can generate constants interrupts at a preset
frequency. This interrupt can be detected by checking bit \>08 in the
flags register at \>7FFF0, or it can be used to trigger an external
interrupt, if bit \>01 is set in the interrupt enable register at
\>7FFF6. The interruption period is set in the same register, according
to the following table:

|     |           |                  |
|-----|-----------|------------------|
| RS  | Frequency | Interrupt period |
| 0   | \-        | \-               |
| 1   | 32768 Hz  | 10 msec          |
| 2   | 16384 Hz  | 100 msec         |
| 3   | 8192 Hz   | 122.07 us        |
| 4   | 4096 Hz   | 244.141 us       |
| 5   | 2048 Hz   | 488.281 us       |
| 6   | 1024 Hz   | 976.5625 us      |
| 7   | 512 Hz    | 1.953125 ms      |
| 8   | 256 Hz    | 3.90625 ms       |
| 9   | 128 Hz    | 7.8125 ms        |
| 10  | 64 Hz     | 15.625 ms        |
| 11  | 32 Hz     | 31.25 ms         |
| 12  | 16 Hz     | 62.5 ms          |
| 13  | 8 Hz      | 125 ms           |
| 14  | 4 Hz      | 250 ms           |
| 15  | 2 Hz      | 500 ms           |

Note that only conditions 1 and 2 differ from those of the RTC-65271 and
those of the bq4847.

####  Power-fail interrupts

When Vcc falls below a certain point (typically, +4.37 volts), it sets
bit \>20 in register \>7FFF0, and may trigger an external interrupt if
bit \>40 is set in register \>7FFF6. After about 100 microseconds, it
sends a reset signal on the Rst\* pin and disables access to the SRAM.
However, it will only switch to battery power when the main power falls
under 3 volts. This allows to finish any pending write operation that
may have been interrupted by a power-down.

When the main power reaches +3.0 volts, the chip switches off its
battery. However, it remains write-protected for 100 milliseconds after
the main power passes +4.37 volts. During that time, it also brings the
Rst\* pin low. Not that it will also reset then interrupt enable
register, which will need to be re-enabled upon power-up.

A battery low warning flag is also provided: bit \>10 in register
\>7FFF0 will be set to '1' if the battery voltage falls under +2.2
volts. Under these conditions, the validity of the clock registers and
the integrity of the SRAM are not guaranteed when the main power is off.
Since the battery cannot be changed, you must buy a new clock chip...

To spare the battery, you should set bit \>80 to the seconds register at
\>7FFF9 before taking the card out of the PE-box. This will stop the
clock while the card is in storage, which saves a lot of power.
Obviously, you will need to set the clock again when re-installing the
card. This bit is set to '1' upon shipping from the factory, so you must
reset it to '0' (i.e. while setting the clock for the first time) to
start the clock. The contents of the SRAM are not affected by this bit.

####  Watchdog timer function

This function could be used to monitor a microprocessor and make sure
it's not frozen or traped inside a program loop. To enable the watchdog,
write a non-zero value in the watchdog register at \>7FFF7. The two
rightmost bits define the unit of measurement: 1/16th of a second, 1/4
of a second, 1 second, or 4 second. The next 5 bits encode the desired
timeout value. For instance, writing binary 0 00011 10, (i.e. \>E0
hexadecimal) encodes a timeout period of 3 seconds.

Bit \>80 in the same register serves to determine the action to be taken
when the watchdog times out. When this bit is '1', a timeout condition
causese a low pulse on the Rst\* pin (this also occurs at power up, by
the way). Since the Rst\* pin is not used on the IDE card, this option
is not very useful. Leaving bit \>80 as '0' in register \>7FFF7 causes a
timeout condition to bring the Int\* pin low, thereby generating an
interrupt that cannot be masked in register \>7FFF6. Int\* will stay low
until register \>7FFF7 is written to again (or register \>7FFF6 since
the TI-99/4A always accesses two at a time).

To prevent the watchdog timer from firing, the microprocessor should
write to register \>7FFF7 before the timeout period is over. This will
restart the watchdog timer with the value just written. Note the the
watchdog timer is disabled automatically when the main power goes down.

####  Clock calibration

Because the oscillation rate of the built-in crystal changes with
temperature, the accuracy of the clock is only guaranteed within 1
minute per month. You may improve on this by changing the calibration
value in register \>7FFF8. Positive values are entered in increments of
10.7 seconds per month (when the sign bit \>20 is set), negative values
in increments of 5.35 seconds per month (sign bit \>20 is '0').

The best way to do so is to let the clock run for a month and check it
versus a reliable source, e.g. the Greenwich clock. Then enter the
proper correction. An alternative way is provided, by putting the clock
in "frequency test mode" and checking the seconds register: but \>01
should toggle at 512 Hz. Unfortunately, this method requires a very
reliable frequency meter, and continuous access to the clock chip, which
the IDE card is not really designed for...

###  Timing diagrams

#### Read cycle

         |         >85 ns          |
         \___                      /        CS*
         |    _<85 ns |            |<35|
         |     \______|________/  <25  |    OE*
         |      | <45 |                |
    XXX        valid  |                | XXXX  Address
      |  |  >5    |   |                |
    ZZZZZZZZZZZZZ-XXXXX    valid       ZZZZ D0-D7
      |    < 85 ns    |

####  Write cycle

       |           >85  ns                 |
    XXX              valid                 XXXX Address
    ___|>0 |                               |
       |   \___      >75 ns          /  >5 |    CS*
       |  >0    |               _____      |
                \____>65 ns____/|    >15   |    WE*
             |       >35 ns     |>10  |
    ZXZZZZZZZZ     valid              ZZZZZZZZZ D0-D7

####  Power down/up

    _____
         \                                /
    Vpfd__\                              /____Vpfd           Vcc
          |\                            /|
          | \___                   _   /-|----Vso
          |                              |
          |40-160 us|                    |40-200 ms |
    valid |          XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX valid  CS*
          |40-160 us|                    |40-200 ms |
    valid |          ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ valid  All outputs
          |                                         |
          |____                                     |
          |                     \                   /       Rst* (+ pull-up)
          |40-160 us|           |              _____
                    \           /ZZZZZZZZZZZZZ/             Int* (+ pull up)

Voltage slews:
4.50 volts down to 4.20 volts: \>300 us
4.20 volts down to Vso: \>10 us
Vso up to Vpfd: \>0

###  Electrical characteristics

#### Absolute maximum ratings

    Supply voltage Vcc:               -0.3V to 7V
    Input voltage:                    -0.3V to 7V
    Operating temperature:             0 to 70 `C
    Storage temperature:             -40 to 70 `C
    Soldering temperature: max 260 `C for 10 secs

####   Recommended operating conditions

|                        |      |     |         |      |
|------------------------|------|-----|---------|------|
| Parameter              | Min  | Nom | Max     | Unit |
| Supply voltage, Vcc    | 4.5  | 5   | 5.5     | V    |
| Input low voltage Vil  | -0.3 | .   | 0.8     | V    |
| Input high voltage Vih | 2.2  | .   | Vcc+0.3 | V    |

####  Electrical characteristics under recommended conditions


CS*&gt;Vcc-0.2V Vin&lt;0.2V or &gt;Vcc-0.2V
2
4

------------------------------------------------------------------------

### IDE controller

[Cable pinout
](#IDE%20pinouts)[Registers
](#IDE%20registers)[Commands
](#IDE%20commands)[Operating the IDE controller
](#Operating%20the%20IDE%20controller)[LBA vs CSH
](#LBA%20vs%20CSH)[DMA
](#DMA)[Drives](#Drive%20considerations)

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

####  IDE commands

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

####  Operating the IDE controller

Lets take the read command as an example. First we should set the
address of the desired sectors in the sector, cylinder, etc registers.
Let's say we are using logical block addressing and want to access
logical block \>123456.

Sector register \<--- 56
Cylinder LSB register \<---34
Cyinder MSB register \<---12
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
to be read. For write operations, the IRQ line is activated each time a
sector has been written and the controller is waiting for another one
(including after the last sector: when the user can read the registers
again). For all other commands, an interrupt is issued at the result
phase, i.e. when the user can access the registers again after command
completion.

The IDE card lets you check the state of the IRQ pin by reading CRU bit
0, and can optionally trigger an interrupt, if the corresponding DIP
switch is closed.

Note that the controller contains a 512-byte buffer, so when writing
data to the disk, you don't have to wait until each byte is physically
written: just place the data into the buffer and leave the rest to the
controller. You will have to wait between sectors though. Remember that
hard-drive sectors are 512 bytes in length.

####  LBA versus CSH

Hard drives consists in several disks, each accessed via a reading head.
Each disk contains a number of concentric tracks and a set of tracks
with the same number on each disk is called a cylinder. Tracks are
subdivided in sectors. The number of sector per track may vary from
drive to drive or even from track to track on the same drive (tracks at
the inside of the disk are shorter and can accomodate less data).

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

The IDE card can perform both CSH and LBA addressing, but the DSRs I
wrote demand LBA.

####  Direct memory access

The LBA controller is able to transfer data directly into the computer
memory, bypassing the CPU. Unfortunately, this cannot be done with the
TI-99/4A, at the moment. But I'm planning on building a DMA controller
card that should make it possible. The IDE card already comprises the
necessary circuitery.

####  Drives considerations

The IDE bus is intended for two devices: a master and a slave. These
could be hard-drives, CD-ROM drives, or even a ZIP-drive. I only have
tested the card with a single hard-drive, but it should also work with
two. Just make sure you place the jumpers properly on each drive. If you
want to access a CD drive, you'll have to write your own routines to
translate the PC format, as IDEAL uses its own format.

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


1: turn card on
&lt;&gt; switch: SRAM maps at &gt;4000-40FF
1: Enable SRAM page switching upon write
1: Same page throughout &gt;4000-5FFF
1: Enables RAMBO banks in &gt;6000-7FFF
1: Write-protects the SRAM
1: Enables IDE interrupt and reset by bit 7
1: Normal operations

###  Memory map

When CRU bit 1 matches the position of the DIP-switch (i.e. when you
write back what you read), the address space \>4000-40FF is used to map
the clock and IDE registers. Otherwise, the SRAM maps here, just as in
the rest of the address space (\>4100-5FFF). .

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

    >4000-401F: clock XRAM (RTC-65217)
    >4020: RTC address register (RTC-65271)
    >4030: RTC data register (RTC-65217)

    >4020-403E: RTC registers (bq4847, even bytes)

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

    >4080: XRAM page register (RTC-65271)

N.B. The clock registers for bq4842 or bq4852 map in the SRAM domain,
\>5FF0-5FFF, at page \>7F.

###  Low-level access

#### SRAM paging

To switch pages, all we have to do it to enable switching with CRU bit 2
and then write to an address in the SRAM. Address lines A9-A14 will be
used as a page number. Just to make sure we don't mess up any data, it
may be a good idea to write-protect the SRAM beforehand.

* SRAM page selection routine. Page # in  R1
*---------------------------------------------------------------------
SELPG  SLA  R1,8             Max 256 pages
       SRL  R1,7             A15 is not latched: shift to the left
       SBO  5                Write protect SRAM
       SBO  2                Enable latching
       MOV  R1,@&gt;5F00(1)     Latch page #, from address lines A9-A14
       SBZ  2                Disable latching
       SBZ  5                Enable writing
       SRL  R1,1             Restore page # for caller (optional)
       B    *R11

####  RTC-65271 clock access

To access a register in the RTC-65271 real-time clock, first write the
register number (\>0000 to \>3F00) at address \>4020. For some reason,
this number cannot be read back, I'm not sure if it's a characteristic
of the chip, or a timing problem with my design. The specified register
can then be accessed byte-wise at address \>4030, if you access it
wordwise, its contents probably be repeated in each byte. Remember to
stop clock updates by setting bit \>80 in register B, before you attempt
to modify registers 0 to 9.

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

#### RTC-65271 XRAM access

The SRAM onboard the RTC-65271 can be accessed at addresses \>4000-401F.
This won't work with any other clock chip, though. To change page, write
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
       B    *R11           

#### bq4847 clock access

Simply access the required register in the area \>4020-403E. Registers
map at even address and should thus be accessed byte-wise. If you access
them word-wise, their contents will likely be repeated in both bytes.
The memory address for a given register is \>4020 + twice the register
number. Remember to freeze register update before you attempt to access
the clock.

* Reading the bq4847 clock
*---------------------------------------------------------------------
GETIME SOCB @H08,@&gt;403C     Set UTI bit in reg 14 to freeze updates
       MOVB @&gt;4020,@SECS    Get seconds from reg 0
       MOVB @&gt;4024,@MINS    Get minutes from reg 2
       MOVB @&gt;4028,@HRS     Get hours from reg 4
       SZCB @H08,@&gt;403C     Reset UTI bit to enable updates
       B    *R11            `
H08    BYTE &gt;08

Remember that most registers encode numbers in BCD format, i.e. twelve
is coded as \>12, not as \>0C as it would be in hexadecimal. The
procedure to write to the clock is very similar:

* Writing to the bq4847 clock
*---------------------------------------------------------------------
SETIME SOCB @H08,@&gt;403C     Set UTI bit in reg 14 to freeze updates
       MOVB @SECS,@&gt;4020    Write seconds to reg 0
       MOVB @MINS,@&gt;4024    Write minutes to reg 2
       MOVB HRS,@&gt;4028      Write hours to reg 4
       SZCB @H08,@&gt;403C     Reset UTI bit to enable updates
       B    *R11           

#### IDE access

You can read IDE registers at addresses \>4040-404E and \>406C-406E. You
can write to them at \>4050-405E and \>407C-407E. Make sure you check
the status bits before sending a command, otherwise you will read
meaningless data and the controller will ignore your writes. Apart from
the 16-bit data register, all registers are 1 byte long and are accessed
at even addressed (i.e. in the most significant byte of a word).

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
STBSY  DATA &gt;8000             &quot;Busy&quot; status bit
STRDY  DATA &gt;4000             &quot;Ready&quot; status bit
STDRQ  DATA &gt;0800             &quot;Data requested&quot; bit
STERR  DATA &gt;0100             &quot;Error&quot; status bit`
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
       SWPB R0
       MOVB R0,@&gt;4056          Set LBA address
       SB   R0,R0              Keep only next byte
       A    @LBA2,R0           Repeat for all 4 bytes
       SWPB R0
       MOVB R0,@&gt;4058
       SRL  R0,8
       A    @LBA1,R0
       SWPB R0
       MOVB R0,@&gt;405A
       SRL  R0,8
       A    @LBA0,R0
       SWPB R0
       MOVB R0,@&gt;405C
       LI   R0,&gt;0100
       MOV  R0,@&gt;4054           Transfer 1 sector
       B    *R11`
LBA0   DATA &gt;E000               LBA buffer (only MSB are used)
LBA1   DATA &gt;0000
LBA2   DATA &gt;0000
LBA3   DATA &gt;0000`
*---------------------------------------------------------------------
Set drive parameters. Must be sent before any other command
*---------------------------------------------------------------------
SETPAR MOV  R11,R10
       LI   R0,&gt;2000           (Arbitrary value)
       MOV  R0,@&gt;4054          Number of logical sectors/track
       LI   R0,&gt;E800           (Arbitrary value)
       MOV  R0,@&gt;405C          Number of logical heads
       LI   R0,&gt;9100           &quot;Set drive parameters&quot;
       BL   @SENDCM            Send command
       B    *R10               Skipped if ok
       INCT R10                Skip JMP if ok
       B    *R10`
*---------------------------------------------------------------------
* Read a 512-byte sector into buffer in R2, sector # in R0
*---------------------------------------------------------------------
SECRD2 MOV  R11,R10
       BL   @SETLBA           Set LBA for sector # in R0
       LI   R0,&gt;2000          &quot;Read sector&quot;
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
       LI   R0,&gt;3000          &quot;Write sector&quot;
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

The card DSR, or disk operating system is known as IDEAL, for "IDE Acess
Layer". A dedicated loader, called IDELOAD, serves to load it into the
card SRAM. You also have the option to install a boot routine in the
RTC-65271 clock XRAM that will automatically load IDEAL from the hard
disk upon power-up. IDEAL is described in details on [another
page](ideal.md).
Revision 1. 3/29/04. Version 2. Preliminary, not for release.
Revision 2. 7/12/04. Ok to release.
Revision 3. 7/15/04. Corrected some resistor values.
Revision 4. 2/7/06. Corrected "Geneve" circuitery.

[Back to the TI-99/4A Tech Pages](titechpages.md)
