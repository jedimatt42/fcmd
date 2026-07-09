# The USB - Smartmedia card

This is actually a 2-in-1 card: it consists in a USB card, and a
SmartMedia interface card, sharing their buffers, the CRU interface, and
the DSR memory circuitery. It is quite possible for you to build only
the USB part, or only the Smartmedia part, but I thought it more
cost-effective to build only one card that would serve both purposes.

Main features:

- Philips ISP1161 USB 1.1 controller.
- Supports half-speed and full-speed, but not high-speed.
- Two downstream ports to control upto 127 devices (with hubs).
- One upstream port, to communicate with another computer.
- A connector for SmartMedia cards (digital camera type), either 3.3
  volts or 5 volts.
- One megabyte of SRAM and 8 megabytes of Flash-EEPROM for DSR memory.
- CRU interface, 24 output bits, 8 input bits.
- CRU address selectable with DIP switches.
- Compatible with Myarc's Geneve.
- DMA capable.

The present page describes the card in details. For intructions on how
to build it, where to buy the parts, and a picture of the card, follow
[this link](usbhowto.md).

[Hardware description
](#Hardware)[Software issues](usb2.md)

#### Legal disclaimer. Read this first.

The contents of this webpage are for educative purposes only. I'm not an
electrical engineer, I have little knowledge of electronics and
therefore cannot guaranty that the device described in this document as
an USB-SmartMedia interface card will function properly. In fact, it
probably won't. It may even cause damage to your computer. And quite
possibly catch on fire and burn your house to ashes, sending toxic fumes
in the whole neighbourhood. Actually, it may even kill you. So if you
know where your interest it, don't build it! Don't even think of it.
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

##  Hardware

[Bus buffering
](#Buffers)[Power supply
](#Power)[Address decoding
](#Address%20decoding)[CRU interface
](#CRU%20interface)[Data bus demultiplexer
](#Demux)[Memory](#Memory) (Flash-EEPROM and SRAM)
[USB controller wiring
](#ISP1161%20wiring)[DMA capability
](#DMA)[SmartMedia connector
](#SM%20connector)[Chip pinouts](#chip%20pinouts)

#### Bus buffering

The PE-box bus is buffered as per TI recommendations. The only variation
is that I used 74ALS245 transceivers to buffer the address bus, instead
of 74ALS244. The reason being that the pin layout on the '245 made it
easier to design a surface-mount board. The '245 are hard-wired as
one-direction buffers by grounding their DIR pin, and are permanently
enabled by grounding their OE\* pin.

The data bus is buffered by another 74ALS245, which direction is
controlled by the DBIN signal, and which is enabled by the selection
signal CardSD\* described in the DMA section. CardSD\* is a combination
of the card selection signal CardSel\* with signals active during DMA
operations.

There is also a 74LS125 3-state quad-buffer to generate the ExtInt\*
interrupt request signal and the RDBENA\* (remote data bus enable)
signal. In both cases, the input is connected to ground, and the OE\*
pin is controlled by the appropriate signal in the card circuitery.
RdbEna\* is simply driven by the CardSel\* signal, but the circuitery
for ExtInt\* is a bit more complex. The two interrupt signals from the
ISP1161 are combined via an AND gate (74ALS08), then masked with the
inverse of CRU bit 4, through a 74ALS32 OR gate. This way, interrupts
are disabled upon power-up but can be enabled by software, by setting
CRU bit 2 to '1'.

           +-------+                         +-------+   
    A15&gt;---|B0   A0|---A15             A7&gt;---|B0   A0|---A7   
      .    |       |    .               .    |       |    .       
      .    |       |    .               .    |       |    .           
      .    |       |    .               .    |       |    .         
      .    |       |    .               .    |       |    .    
      .    |       |    .               .    |       |    .    
      .    |       |    .               .    |       |    .    
     A8&gt;---|B7   A7|---A8              A0&gt;---|B7   A7|---A0    
      Gnd--|OE* Dir|---Gnd              Gnd--|OE* Dir|---Gnd       
           +-------+                         +-------+
                   74ALS245                          74ALS245
           +-------+                         +-------+  
CLKOUT*&gt;---|B0   A0|---Phi3*           D7&gt;---|B0   A0|----D7 
CRUCLK*&gt;---|       |---Cruclk*          .    |       |     .
   DBIN&gt;---|       |----------+--DBIN   .    |       |     .
    WE*&gt;---|       |---WE*    |         .    |       |     .
  MEMEN&gt;---|       |---Memen* |         .    |       |     .
 RESET*&gt;---|       |---Reset* |         .    |       |     .
      Gnd--|       |-         |         .    |       |     .
      Gnd--|B7   A7|-         |        D0&gt;---|B0   A0|----D0 
      Gnd--|OE* Dir|-Gnd      &#39;--------------|Dir OE*|--------------CardSD*
           +-------+                         +-------+   74LS125             
                        RDBENA*&lt;--------------------------&lt;|--Gnd
                                                          &#39;---------CardSel*
                                      74LS125          74ALS04
                        EXTINT*&lt;--------&lt;|---Gnd     ,---o&lt;|---Bit2
                                        &#39;---------&lt;(=-,
                                              74ALS32 |      ,--ISP1161/INT2
                                                      &#39;---(|=---ISP1161/INT1
                                                       74ALS08

####  Power supply

The Smartmedia part of the card requires a +3.3 volts power supply,
whereas the rest requires +5 volts. Therefore, there are two voltage
generator on board: a TL78-05 regulator for the +5V and a UA78M33
regulator for the +3.3V. Both have 100 nF decoupling capacitors, and
share a common 47 uF electrolytic cap on the unregulated +8V input.
Because these regulators are likely to generate a lot of heat, I
recommend unsing a large heat sink.

In addition, each chip also need a 100 nF decoupling capacitor between
its power pins.

                  TL780-05                         All chips
                +----------+      +5V              +-----+
+8V-----+---+---|Vin   Vout|---+-----... ---+------|Vcc  |    
        |   |   |   Ref    |   |            |      |     |
 47 uF  =   |   +----------+   = 0.1 uF     =0.1uF |     |
        |   |        |         |            |      |     |
       Gnd  |       Gnd       Gnd      Gnd--+------|Vss  |
            |                                      +-----+
            |
            |      78M33                        2x 74LVT245
            |   +----------+     +3.3V             +-----+
            &#39;---|Vin   Vout|---+-----... ---+------|Vcc  |    
                |   Ref    |   |            |      |     |
                +----------+   = 0.1 uF     =0.1uF |     |
                     |         |            |      |     |
                    Gnd       Gnd      Gnd--+------|Vss  |
                                                   +-----+

####  Address decoding

The address decoder is fairly simple: a 74ALS138 breaks up the address
space in chunks of \>2000 bytes thanks to its selection inputs connected
to address lines A0 through A2. Its G1A\* enabling input is driven by
Memen\* so that it only responds to memory oparations. Its G1 input is
controlled by CRU bit 0, so that the card only answers when this bit is
'1' as per TI conventions. A main switch can be mounted on the G2B\*
enabling input, so that the card can be disabled during power-up. This
may come handy if the DSRs get corrupted and prevent booting. The card
is enabled when the G2B\* input is low.

For the card to work properly with the Myarc "Geneve", it is necessary
to further decode five extra address lines: AMA (pin \#46 of the PE-box
bus), AMB (pin \#45), AMC (pin \#48), AMD (pin \#8, for use with Genmod)
and AME (pin \#9, Genmod). The decoding if performed by an extra
74ALS138, which intercepts the enabling signal from the main switch. One
position on the main switch lets you bypass the decoder for use with a
TI-99/4A, on which lines \#8 and \#9 are not controled.

The main switch should be placed on "TI" (i.e. ground) in a
TI-99/4A-based system, and on "Gen" (i.e. 74LS138 output) with a Geneve.
Using the "Gen" with a TI-99/4A won't work. Using the "TI" posititon
with a Geneve will make the card answer requests in the range
\>4000-5FFF, no matter what Rompage was selected (whereas the "Gen"
position demands page \>BA).

Ouput Y2\* of the first '138 is active (low) for memory operations in
the range \>4000-5FFF. It constitutes the card selection signal
(CardSel\*), which enables the data bus by activating the RDBENA\* line
through the 74LS125 (see [bus buffering](#Buffers) above).

The address space is further broken down by half a 74ALS139 dual
decoder, which is enabled by CardSel\*. The selection inputs of the
decoder take their signal from address line A3 (splitting \>4000-4FFF
from \>5000-5FFF) and from a 13-inputs NAND gate, 74ALS133. The gate is
active (low) when address lines A4 through A11 are high, as well as CRU
bit 1. As a results, the DSR space is broken in four domains, each
activated by an output of the 74ALS139:

Y1\*: \>4000-4FEF. This is the EEPROM domain.
Y0\*: \>4FF0-4FFF (when CRU bit 1 is high). This is the SmartMedia
domain.
Y3\*: \>5000-5FEF. This is the SRAM domain.
Y2\*: \>5FF0-5FFF (when CRU bit 1 is high). This is the USB controller
domain.

When CRU bit 1 is low, the EEPROM domain goes upto \>4FFF, and the SRAM
domain to \>5FFF.

        A4---|  )
         .   |   )
         .   |    )
         .   |     )
       A11---|      )o------,
      Bit1---|     )        |
       +5V---|    )         |
         .   |   )          |
       +5V---|__)           |
           74ALS133         |
                            |
                            |                   
            +-------+       |         +-------+
       A0---|S0     |       &#39;---------|S0  Y0*|----Smart*
       A1---|S1     |            A3---|S1  Y1*|----Flash*
       A2---|S2     |                 |    Y2*|----USB*
            |    Y2*|----+------------|G*  Y3*|----SRAM*
            |       |    |            +-------+
   Memen*---|G2A*   |    |            74ALS139
     Bit0---|G1     |    &#39;---CardSel*
         ,--|G2B*   |                
         |  +-------+
         |  74ALS138
         &#39;--------------,
                        |
                        /                Key:
                +5V---o o o---Gnd        Off  Gen  TI
            +-------+   |
      Gnd---|G2A*   |   |
AMA---------|S0  Y7*|---&#39;
AMB---------|S1     |
AMC---------|S2     |
AMD---------|G2B*   |
AME---------|G1     |
            +-------+
            74ALS138

####  CRU interface

The CRU address is verified by a 74ALS688 comparator, that checks
address lines A0 through A7. The first four lines have fixed values
(\>1xxx), the last four are compared with a DIP-switch: pull-up
resistors make these lines high when their respective switches is open,
whereas they are low when the switch is closed. This lets you assign any
CRU address to the card, from \>10xx to \>1Fxx.

The output of the comparator enables a second 74ALS138 decoder, whose
selection inputs are connected to address lines A10 and A11, and to the
CruClk\* line that signals CRU write operations. Under these conditions,
the decoder outputs Y0\* through Y3\* each select 8 CRU ouput bits,
whereas output Y3\* through Y7\* each select 8 CRU input bits.

      +-------+              4k7                         470   |
 A4---|A0   B0|----------+---WWW---+---+5V         +-------+   V  LED
 A5---|A1   B1|---------+|---WWW---+      Reset*---|RST*   |   |
 A6---|A2   B2|--------+||---WWW---+         A12---|S2   Q0|---+---Bit0
 A7---|A3   B3|-------+|||---WWW---&#39;         A13---|S1   Q1|-------Bit1
 A1---|B5   A5|-Gnd   ||||                   A14---|S0   Q2|-------Bit2
 A2---|B6   A6|-Gnd   oooo  DIP                    |     Q3|-------Bit3
 A3---|B7   A7|-+5V   oooo  switch           A15---|D    Q4|-------Bit4
 Gnd--|B4   A4|-Gnd   ||||                         |     Q5|-- Bit5
 A0---|G*     |       ++++       ,-----------------|EN*  Q6|--  .
      |   A=B*|--,     Gnd       |                 |     Q7|--  .
      +-------+  |               |                 +-------+    .
   ,-------------&#39;               |                             Flash
   |       +-------+             |                 +-------+   EEPROM
   &#39;-------|G2A*   |             |        Reset*---|RST*   |   Page
     A11---|S0  Y0*|-------------&#39;           A12---|S2   Q0|--  .
     A10---|S1  Y1*|-------------,           A13---|S1   Q1|--  .
 Cruclk*---|S2  Y2*|-------,     |           A14---|S0   Q2|--  .
           |       |       |     |                 |     Q3|--  .
     +5V---|G1  Y4*|---,   |     |           A15---|D    Q4|--  .
     Gnd---|G2B*   |   |   |     |                 |     Q5|--  .
           |       |   |   |     &#39;-----------------|EN*  Q6|--  .
           |       |   |   |                       |     Q7|--Bit15
           +-------+   |   |                       +-------+  
           74ALS138    |   |                       +-------+ 
                       |   |              Reset*---|RST*   |
                       |   |                 A12---|S2   Q0|--Bit16
                       |   |                 A13---|S1   Q1|--  .
                       |   |                 A14---|S0   Q2|--  .
                       |   |                       |     Q3|-- SRAM
                       |   |                 A15---|D    Q4|-- Page
                       |   |                       |     Q5|--  .
                       |   &#39;-----------------------|EN*  Q6|--  .
                       |                           |     Q7|--Bit23
                       |        74ALS251           +-------+ 
                       |        +-------+           74ALS259 (x3)
                       |        |     D0|---Bit0in
                       |  A12---|S1   D1|---Bit1in
                       |  A13---|S1   D2|---Bit2in
                       |  A14---|S0   D3|---Bit3in
                       |        |     D4|---Bit4in  
                       &#39;--------|G*   D5|---Bit5in
          CRUIN&gt;----------------|Y    D6|---Bit6in 
                                |Y*   D7|---Bit7in  
                                +-------+ 
 

There are three 74ALS259 connected to output Y0\* through Y2\* of the
'138, thereby providing a 24-bit CRU output interface.

- Bit 0 turns the card on, and controls the indicator LED.
- Bit 1 controls mapping of the USB controller and SmartMedia card into
  DSR space.
- Bit 2 enables interrupts.
- Bit 3 enables the SmartMedia card.
- Bit 4 prevents reading from the Flash-EEPROM and unprotects it, so it
  can be programmed.
- Bits 5 through 15 select the EEPROM page.
- Bits 16 through 23 select the SRAM page.

A single 74ALS251 is connected to output Y4\* providing 8 bits of CRU
input.

- Bit 0 and 1 sense the interrupt pins of the USB controller.
- Bit 2 and 3 sense the "suspended" pins of the USB controller (host &
  device respectively).
- Bit 4 senses the STS pin of the Flash-EEPROM
- Bit 5 senses the R/B\* pin of the SmartMedia connector.
- Bit 6 senses the presence of a card inside the connector.
- Bit 7 checks whether the SmartMedia card is write-protected.

The selection inputs of these four chips are controlled by address lines
A12 trough A14. The '259s take their data input from the A15/CruOut
line, whereas the '251 send its output directly to the Cruin line of the
PE-box bus. Additionally, the '259s are reset by the Reset\* signal, to
ensure that all bits will be '0' upon power up.

####  Data bus demultiplexer

The USB controller has a 16-bit data bus, which makes it necessary to
demultiplex the data bus. As you know, the TI-99/4A console also has a
16-bit data bus, but the bus is multiplexed as 2 x 8 bits on the side
port (talk about a short-sighted design decision...). To demultiplex the
data bus, we'll make use of two 74ALS543. These nifty chips incorporate
a bidirectional bus transceiver with two latches, one on each input.
Both chips have their 'A' pins connected to the 8-bit data bus, and
their 'B' pins together form the 16-bit bus. The OExx\* pins enable
output in the indicated direction (e.g. OEAB\* in the A-\>B direction),
whereas the LExx\* pins latch data when low and retain it when high.
Both chips are permanently enabled by tying their CEAB\* and CEBA\* pins
to the ground.

                                                 +-------+   
                                           D0&gt;---|A0   B0|---D15   
                                            .    |       |    .       
                                            .    |       |    .           
                                            .    |       |    . 16-bit bus  
                                            .    |       |    .    (MSB)
                                            .    |       |    .    
                                            .    |       |    .    
                                           D7&gt;---|A7   B7|---D8    
                           T     WE*---,    Gnd--|CEAB*  |       
                      ,----o-----------=)&gt;-------|LEAB*  |       
           74ALS139   |    o----+----------------|OEAB*  |       
           +-------+  |  G o    |           Gnd--|CEBA*  |       
    Dbin---|S0  Y0*|--&#39;    |    |  ,-------------|OEBA*  |       
     A15---|S1  Y2*|--+----&#39; G  |  |  ,----------|LEBA*  |       
           |    Y1*|--|------o--|--&#39;  |          +-------+
CardSD*----|G*  Y3*|--|--,   o--|-----+-----------------------------------RD*
           +-------+  |  +---o  |     |             74F543 (odd)          
                      |  |   T  |     |          +-------+   
                      |  |      |     |    D0&gt;---|A0   B0|---D7   
                      |  |      |     |     .    |       |    .       
                      |  |      |     |     .    |       |    .           
                      |  |      |     |     .    |       |    .  16-bit bus   
                      |  |      |     |     .    |       |    .    (LSB)
                      |  |      |     |     .    |       |    .    
                      |  |      |     |     .    |       |    .    
                      |  |      |     |    D7&gt;---|A7   B7|---D0    
                      |  |      |     |     Gnd--|CEAB*  |       
                      |  |      |     &#39;----------|LEBA*  |       
                      |  &#39;------|----------------|OEBA*  |       
                      |         |  WE*---,  Gnd--|CEBA*  |       
                      &#39;---------|--------=)&gt;-----|LEAB*  |       
                                +----------------|OEAB*  |
                                |                +-------+      
                          WE*---=)&gt;---------------------------------------WR*
 

The second half of the 74LS139 that was used for address decoding serves
to distinguish four possible situations, thanks to the wiring of its
selection inputs to A15 (S1) and DBIN (S0).

Y0\*: write even byte (MSB, 2nd)
Y1\*: read even byte (MSB, 2nd)
Y2\*: write odd byte (LSB, 1rst)
Y3\* read odd byte (LSB, 1rst)

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

The first half of the DPDT switch serves to determine which byte comes
last: the even one with the TI-99/4A (Y0\* signal) or the odd one with
the Geneve (Y2\* signal). The selected signal enables both 74F543 for
output in the B-\>A direction, thereby presenting a 16-bit word on the
16-bit databus. The same signal is combined with WE\* via an OR gate to
provide the WR\* write pulse to 16-bit devices.

For read operations, the second half of the DPDT switch determines which
byte comes first: the odd byte with the TI-99/4A (Y3\* signal), the even
byte with the Geneve (Y1\* signal). The selected signal causes both
74F543 to latch their respective half of the 16-bit data bus in the
B-\>A direction. It also makes up the RD\* signal sent to 16-bit
devices. Technically, it's a bit dangerous to control the latches with
the RD\* signal, as they will latch data when RD\* goes inactive, at
which time the data may not be available any more! But it turned out
that all devices on the card hold the data long enough after RD\* goes
high for the 74F543 to latch it properly.

The latched odd byte is made available for reading by the Y1\* signal,
which enables the "odd" 74F543 for output in the B-\>A direction.
Similarly, the even byte is made avilable by the Y3\* signal.

PS: Note that, for Texas Instruments, D0 is the most significant bit,
whereas for the rest of the world it's the least significant bit. This
was taken into account in the wiring of the 'A' inputs of the 74ALS543s.

####  Memory

This card has both ROM and RAM. The ROM maps at \>4000-4FFF and is
implemented with an Intel StrataFlash, which is an 8 megabytes
Flash-EEPROM. Optionally, you could cut costs and replace it with a 4
meg StrataFlash.

The nifty StrataFlash-EEPROM chips can be wired either to an 8-bit bus,
or to a 16-bit bus. Here, we are connecting it to the 16-bit bus, which
is required to correctly pass unlocking codes to the chip (since the
TMS9900 always writes 2 bytes at a time). Our decision is signified to
the chip by wiring the Byte\* pin to +5 volts via a 4.7 K resistor, and
leaving address pin A0 unconnected.

Address pins A1 through A11 are connected to address lines A14 through
A4, respectively. The remaining address pins to CRU bits 5 through 15,
at the output of the corresponding 74ALS259s. This will let us map the
EEPROM as 2048 pages of 4K in the \>4000-4FFF area. If you elected to
save cost by installing only a 4M EEPROM, A22 is not internally
connected, so CRU bit 15 has no effect and you have only 1024 pages.

Another handy feature of the StrataFlash is that it has three enabling
pins. Here, CE0\* is connected to the Y1\* output of the '139 (Flash\*
signal), CE2\* is grounded, and CE1\* is controlled by an AND gate
combining DBIN with CRU bit 4. The latter allows us to disable the chip
during read operations, which is necessary to write an unlocking code
(since the TI-99/4A always performs a read before each write, the read
operations would abort the unlocking sequence). When CRU bit 4 is '0',
reading functions normally, when it's '1' reading operations do not
enable the chip. I found out later on that this feature may not be
strictly necessary (at least not to write only 1 word), but since I had
already implemented it, I left it so. CRU bit 4 also controls the Vpen
pin, which prevents writing/erasing the EEPROM memory unless this bit is
'1'. Writing commands is still allowed when the bit is '0', but not
writing data.

It was pointed out to me that the existence of a Flash-EEPROM in a
TI-99/4A systems now allows for the creation of computer viruses!
Traditionally, the TI-99/4A was considered immune to viruses, because
nothing could survive a reboot, but now there is a possibility that a
malicious program would hide itself inside the EEPROM, from where it
could be launched at power-up, or upon USB access. To preclude this
possibility, I have installed a switch on the Vpen line. When the switch
is open, Vpen is grounded via a 47K resistor, and the EEPROM cannot be
written to, nor erased. When the switch is closed, the EEPROM can be
write-protected or write-enabled via CRU bit 4. Since Vpen has special
voltage requirements (i.e. 4.5 volts or more), a 4.7K pull-up resistor
had to be added on the "bit 4" line, to overcome the 47K pull-down.

The WE\* and OE\* pins respectively receive the WR\* and RD\* signals
from the demultiplexer, as described above. The RP\* (Reset/Power-down)
pin is connected to the Reset\* line, which will reset the StrataFlash
upon power up.

The STS status pin is connected to an input of the 74ALS251, so you can
tell whether the chip is busy writing/erasing data, or is ready to
operate. It needs a 4.7K pull-up resistor because it's an open-collector
output.

Finally, as per Intel specs, both Vcc (not shown) and Vccq are connected
to +5 volts, and decoupled to ground with 100 nF capacitors.

           +-----------+                      +---------+               +---------+  
     A14---|A1      DQ0|---D0           A14---|A17    D0|---D0    A14---|A1     D0|---D8 
      .    | .       . |    .           A13---|A16    D1|---D1    A13---|A2     D1|---D9 
      .    | .       . |    .           A12---|A14    D2|---D2    A12---|A3     D2|---D10 
      .    | .       . |    .           A11---|A12    D7|---D3    A11---|A4     D7|---D11
      .    | .       . |    .           A10---|A7     D6|---D4    A10---|A5     D6|---D12 
      .    | .       . |    .            A9---|A6     D5|---D5     A9---|A6     D5|---D13 
      .    | .       . |    .            A8---|A5     D4|---D6     A8---|A7     D4|---D14 
      A4---|A11      . |    .            A7---|A4     D3|---D7     A7---|A12    D3|---D15
    Bit5---|A12      . |    .            A6---|A3       |          A6---|A14      |
      .    | .       . |    .            A5---|A2       |          A5---|A16      |
      .    | .       . |    .            A4---|A1       |          A4---|A17      |
      .    | .       . |    .         Bit16---|A8       |       Bit16---|A8       |
      .    | .       . |    .         Bit17---|A9       |       Bit17---|A9       |    
   Bit14---|A21      . |    .         Bit18---|A11      |       Bit18---|A11      |    
   Bit15---|A22(8M)  . |    .         Bit19---|A10      |       Bit19---|A10      |
           |         . |    .         Bit20---|A0       |       Bit20---|A0       |
Flash*-----|CE0*   DQ15|---D15        Bit21---|A13      |       Bit21---|A13      |
Dbin--,    |           |              Bit22---|A18      |       Bit22---|A18      |
Bit4--=|)--|CE1*   Vccq|--+--+5V      Bit23---|A15      |       Bit23---|A15      |
      Gnd--|CE2*       |  &#39;---||--Gnd         |         |               |         |
           |           |   100 nF     SRAM*---|CE*      |       SRAM*---|CE*      |
      WR*--|WR*   Byte*|---WWW---+5V          |         |               |         |
      RD*--|RD*        |   4k7          WR*---|WR*      |         WR*---|WR*      |
Reset*-----|RP*      A0|-    47k        RD*---|RD*      |         RD*---|RD*      |
Bit4in--+--|STS    Vpen|--+--WWW--Gnd         +---------+               +---------+
        |  +-----------+  |      ,------Bit4   
        |   4k7           &#39;-o o--+  4k7
        &#39;---WWW---+5V            &#39;--WWW--+5V   

The RAM part of the card consists in two static RAM chips, each 512
Kbytes, for a total of 1 megabyte. One chip is connected to data lines
D0-D7, the other to D8-D15. Both chips are enabled by the Y3\* output of
the 74ALS139 (SRAM\* signal) and receive the RD\* and WR\* signals from
the bus demultiplexer.

The SRAM address pins are controlled by address lines A14 through A4 and
by the outputs of the third 74ALS259, which correespond to CRU bits 16
through 23. Here, it is not important which line controls which pin, as
all RAM address are equivalent (no unlocking code, etc). Similarly, the
data lines have been wired for convenience, not in numerical order.

####  USB controller wiring

The Philips ISP1161 is a dual processor: it contains both a host
controller and a device controller. Note that there are two subsequent
versions: ISP1161A and ISP1161A1. I have no idea what improvements were
made in these and Philips does not mention anything about it on their
website. I suggest that you buy the most recent version, in case it was
issued to correct some weird bug in the older versions. In the rest of
this page, I'll just refer to all three versions as "ISP1161".

The ISP1161 can be powered at either +5 volts or +3.3 volts. I chose 5
volts, since it allows us to make use of the internal overcurrent
control circuits. The two Vhold pins can thus simply be decoupled to
ground with a 10 nF cap. The analog ground AGND and all digital ground
pins are connected to the ground.

The D0-D15 data pins are connected to the 16-bit data bus, in such a way
as to automatically swap the data bytes. This is because the ISP1161
uses the Intel "LSbyte-first" convention to access its internal stacks
via a 16-bit data bus. To use it with a TI-99/4A, we would have to
perform a SWPB on each data word, which would substantially slow down
transmission. I decided that it would be easier to perform byte swapping
at the hardware level. The price to pay is that we'll have to swap byte
when accessing any register other than the stack ports.

Address lines A13 and A14 are connected to pins A1 and A0, respectively.
The CE\* pin is controlled by the Y2\* output of the 74ALS139 (USB\*
signal), whereas the RD\* and WR\* pins are controlled by the
corresponding RD\* and WR\* signals from the demultiplexer. The Reset\*
pin receives the Reset\* signal from the PE-box bus.

Because the TMS9900 microprocessor always perform a read before each
write, an additional trick was required so as not to confuse the ISP1161
during multiple write operations. The RD\* signal is thus further
combined with address line A12, via an OR gate. This results in masking
out the RD\* signal at addresses \>5FF8-5FFF. These addresses can thus
be used for writing to the ISP1161. Reading is performed at addresses
\>5FF0-5FF7.

The two interrupt request pins are combined through an AND gate
(74ALS08) and enable a 3-state gate in the 74ALS125 whose input is
grounded. This will send the interrupt request onto the ExtInt\* line to
the console. Additionally these pins are connected to CRU input bits 0
(INT1) and 1 (INT2) on the 74ALS251, so you can tell whether an
interrupt has occured without having to query the chip. The interrupt
signal is further masked by CRU bit 2, with the help of the remaining
two gates in the 74ALS125. This way, interrupts are disabled by default
at power-up time.

Pin H_SUSPEND and D_SUSPEND are respectively connected to CRU input bits
2 and 3 on the 74ALS251. This lets you check whether the host or the
device controller are in suspended mode. The inputs H_WAKEUP and
D_WAKEUP are not used since the controllers can be awaken by the
software. These two pins are thus grounded.

Pin NDP_SEL is connected to +5 volts via a 4.7K resistor, to indicate
the implementation of both downstream ports.

Since the TI-99/4A is incapable of direct memory access (DMA), the
dedicated pins in the ISP1161 are not used in principle: outputs DREQ1
and DREQ1 can be left unconnected, whereas inputs DACK1, DACK2 and EOT
should be connected to the same resistor that pulls NDP_SEL to +5 volts.
However, I'm planning to design a DMA controller board which will allow
DMA within the PE-box. For this reason, the DMA lines can be wired as
described in the next section, if you so elect.

A 6.00 MHz crystal is connected as per Philips specifications, with two
18 pF caps to the groud, to provide the internal timing within the
controller. The CLKOUT clock output is not used. Neither is the TEST
input (named READY on subsequent versions of the ISP1161).

                  +--------------+    22 ohms            A#1
             D8---|D0       H_DP1|-----WWW-----+--------&lt;DP    
             .    | .       H_DM1|-----WWW-----|-+------&lt;DM   A#2 
             .    | .       H_DP2|-----WWW-----|-|-+---------&lt;DP  
            D15---|D7       H_DM2|-----WWW-----|-|-|-+-------&lt;DM   
             D0---|D8            |             | | | |  
             .    | .            |             = = = = 47 pF    
             .    | .            |             | | | |
             D7---|D15           |             &#39;-+-+-+---Gnd
            A14---|A0            |        1k5          100 nF
            A13---|A1        Vreg|--------WWW--------+--||--Gnd    B#1
         Reset*---|Reset*    D_DP|---WWW-------------+------------&lt;DP
           USB*---|CE*       D_DM|---WWW-------------|-+----------&lt;DM
  A12---,    WR*--|WR*           |  22 ohms    47 pF = =
  RD*---=)&gt;-------|RD*           |                   | |
                  |              |            Gnd---+-+----------&lt;Gnd
  Bit0in------+---|INT1    D_Vbus|--------------------------------&lt;Vcc
  Bit1in------|-+-|INT2       GL*|----------------------WWW----,
              | | |              |   ,-+--------+--+5V  470    |
to ExtInt--(|=--&#39; |              |   | |        |              V LED
                  |              |   = R   ,--, |              |
         Bit2in---|H_SUSP        |   | |   | S|-&#39;        A#1  Gnd
         Bit3in---|D_SUSP  H_PWS1|---+-+---|G |    Gnd--&lt;Gnd
            Gnd---|H_WAKEUP      |  Mosfet | D|-+-------&lt;Vcc
            Gnd---|D_WAKEUP      |         `--&#39; |
                 -|DREQ1    H_OC1|--------------&#39;
(see DMA section)-|DREQ2         |   ,-+--------+--+5V
         4k7   ,--|DACK1         |   | |        |
  +5V----WWW---+--|DACK2         |   = R   ,--, |
               +--|EOT           |   | |   | S|-&#39;             A#2
    18 pF      &#39;--|NDP_SEL H_PWS2|---+-+---|G |        Gnd---&lt;Gnd
Gnd--||---+-------|XTAL1         |  Mosfet | D|-+------------&lt;Vcc
          |       |              |         `--&#39; |
   6 MHz ===      |         H_OC2|--------------&#39;
    18 pF |       |          TEST|-
Gnd--||---+-------|XTAL2   CLKOUT|-
       Gnd---||---|Vhold1        |
       Gnd---||---|Vhold2    AGND|---Gnd
           100 nF +--------------+                   

The DP and DM pairs of outputs are connected to two USB connectors.
There is a double type-A connector for the host controller, and a single
type-B connector for the device controller. Each line has a 22 ohm
serial resistor on it, followed with a 47 pF filter capacitor to the
ground. In addition, the device port has a 1.5K pull-up resistor on the
D_DP line, which indicates to the remote host that the device controller
can handle full-speed transactions. The resistor is pulled up to +3.3
volts, thanks to the dedicated Vreg pin, which is also decoupled by a
100 nF capacitor to the ground. This resistor is optional, as the
ISP1161 has an internal equivalent that can be switched on by software
(a.k.a. "soft-connect"). Having an external resistor insures that the
device will anounced itself as soon as it's powered, otherwise the
internal resistor will only make the device visible when the software is
ready for it. The only problem with that the internal resistor is that
it has a 25% tolerance (hurgh!) which is no in accordance with USB
specs. So really it's up to you, whether you want to implement the
external resistor or not. I did.

The type-A connector takes its power input from the +5V line, under the
control of a MOSFET switch. This is to avoid overloading the voltage
regulator in case a shortcut occurs on the USB bus. There are actually
two MOSFETs inside a single chip: each control one of the type-A ports.
The gate of each MOSFET is pulled up by a resistor and a capacitor (I'm
not sure which values are to be used. I chose 1.5K and 1 nF), and is
enabled by the dedicated ouputs of the ISP1161: H_PSW1 and H_PSW2. So
that the chip can monitor output current, it also needs to be connected
to the output of the MOSFET, via its overcurrent sensing pins H_OC1 and
H_OC2.

The type-B connector receives its output from the remote host, and its
power line is connected to the D_Vbus pin of the ISP1161. This will
allow the device controller to detect the presence of the host.

Optionally, a LED can be connected to the "good-link" GL\* output. It
will blink then the device controller is in use. This is not overly
usefull, as it will probably be fairly rare that you connect your
TI-99/4A to another computer. But you can implement it if it amuses you.

####  DMA support

This is intended to work with a yet-do-be-designed DMA controller card,
which will double as a memory expansion card and replace the PE-box
connection card.

Since there aren't enough spare lines in the PE-box to implement DREQ
and DACK lines for the 4 devices typically handled by a DMA controller
(let alone for 7, handled by two controllers), these lines are
multiplexed with the data bus and the address bus, respectively. The
only DMA-dedicated lines are DmaEn\* and EOP\*, which correspond to the
unused SenilB\* and SenilA\* lines in the PE-box.

D2--------------&lt;|---------------------|DREQ1
                |                      |
D3--------------&lt;|---------------------|DREQ2
       74LS04   |                      |
RdbEna*---|&gt;o---&#39;                      |
                                       |
EOP*-----------------------------------|EOT
            74ALS74                    |
           +-------+                   |
A9---------|D1   Q1|--+----------------|DACK1
           |       |  |                |
A10--------|D2   Q2|--|-+--------------|DACK2
           |       |  | | 74ALS08      +---------
DmaEn*---+-|Ck1    |  &#39;-=|)----,         ISP1161
         &#39;-|Ck2    |           | 74ALS08
         ,-|Pr1*   | CardSel*--=|)--,
Reset*---+-|Pr2*   |                |
         ,-|Clr1*  |      RdbEna*---=)&gt;---CardSD*
    +5V--+-|Clr2*  |              74ALS32
           +-------+         

When the data bus is not in use, the DMA devices use it to output their
DREQ signal on one of the data lines (on the PE-box side of the 74ALS245
buffers), via two spare 3-state gates in the 72LS125. The RdbEna\*
signal enables these gates when it's high, i.e. when the data bus is
inactive. The data lines used here, D2 and D3, select DMA channels 2 and
3.

The DMA acknowledgment signal from the controller is to be found on the
address bus, lines A8 through A14, when the dedicated line DmaEn\*
changes from low to high. A 74ALS74 is used to latch address lines A9
(channel 2) and A10 (channel 3), and thereby issue active low DACK
signals to the USB controller. The latched bits are reset to "high" upon
power-up by applying the Reset\* signal to their Pr\* pins. The Clr\*
lines are disabled by connecting them to +5 volts.

The two DACK signals are additionnaly combined with the CardSel\*
selection signal and the PE-box RdbEna\* signal via two spare AND gates
in the 74ALS08 and one spare OR gate in the 74LS32. This produces the
CardSD\* signal, which enables the data bus and the demultiplexer.
CardSD\* will be low when CardSel\* is low (since CardSel\* brings
RdbEna\* low, the OR gate is transparent), or when one of the DACK line
is low. But in the latter case, the RdbEna\* line masks out the signal:
the DMA controller board will activate RdbEna\* itself, when needed.

Note that CardSD\* does not enable any on-board devices (that half of
the 74ALS139 receives CardSel\* directly), as the address bus is not
valid during DMA. Unless it's memory to memory DMA transfer, in which
caseCardSel\* will be active too. This is possible becasue the ISP1161
ignores its CS\* input during DMA and will answer to any address while
one of its DACK inputs is low.

####  SmartMedia connector

Most large SmartMedia cards are +3.3 volts nowadays. However, there are
still old models (generally 2 megs or less) that require +5 volts. The
connector installed will accept both type of cards. A switch lets you
select the appropriate voltage. Be aware that sending 5 volts into a 3.3
volts card may damage it! If you prefer to yield to Murphy's law, you
can drop the swich and hard-wire the power supply pins to +3.3 volts.
You won't be able to use 5V Smartmedia cards, but at least you're sure
you'll never fry a 3.3V card! In this case, you may want to order a
connector that will only accept 3.3V cards.

Since the rest of the board is +5V, we'll need special interface chips
upstream of the SmartMedia connector. To this end, I'm using two
74LVT245 transceivers. LVT chips are 3.3 volts chips with 5V-tolerant
inputs. One '245 serves to connect the data pins D0-D7 to the most
significant byte of the 16-bit bus, i.e. lines D15-D8. The other one
buffers the various control lines. I could have used a '244 here, but
the pinout of the '245 is more convenient, so I just hardwired it as a
one-direction buffer.

            +3.3V--+--o o-------------------&lt;Vcc
                   |                  Gnd---&lt;Gnd
                   |   74LVT245    
                   |  +-------+
                   &#39;--|Vcc    |            
               D15&gt;---|B0   A0|-------------&lt;IO8     
                 .    |       |    .       
                 .    |       |    .            
                 .    |       |    .            
                 .    |       |    .         
                 .    |       |    .         
                 .    |       |    .          
                D8&gt;---|B7   A7|-------------&lt;IO1  
Smart*--+-------------|OE* Dir|---DBIN        
        &#39;--|&gt;o--,     +-------+            
     74ALS175   |
     +------+   |      74LVT245                 
+5V--|Vcc Ck|---&#39;     +-------+
+5V--|Rst*  | +3.3V---|Vcc    |        
A13--|D1  Q1|---------|B0   A0|-------------&lt;CLE
A14--|D2  Q2|---------|B2   A2|-------------&lt;ALE
     +------+ 74ALS04 |       |
         Bit3---|&gt;o---|B1   A1|-------------&lt;CE*
      WR*-----,       |       |
      Smart*--=)&gt;-----|B4   A4|-------------&lt;WE*
RD*---,    74ALS32    |       |
A12---=)&gt;---,     Gnd-|B7   A7|
Smart*------=)&gt;-------|B3   A3|-------------&lt;RE*
          ,-----------|B6   A6|-------------&lt;WP*
          |    ,------|B5   A5|---Bit5in
          |    | Gnd--|OE* Dir|-Gnd  
          |    |      +-------+           
+3.3V     |    &#39;---------------+------------&lt;R/B*  
 &#39;--WWW---+----------o o---,   &#39;---WWW---+3.3V
    1.5K      1.5K         |       1.5K
          ,---WWW---+3.3V  |          Gnd---&lt;PR1
 Bit7in---+----------------+----------------&lt;PR2
                            1.5K      Gnd---&lt;IN1
 Bit6in---------------------------+---------&lt;IN2
                    +3.3V---WWW---&#39;

Through the LVT buffer and a 74ALS175 latch, pin CLE (command latch
enable) is controlled by address line A13, and pin ALE (address latch
enable) by address line A14. I found out that some Smartmedia cards
don't like ALE and CLE to toggle in between bytes: the write operation
wasn't working properly, writing only \>00 bytes! So I had to include a
74ALS175 latch, to maintain ALE and CLE stable fot such cards. The latch
is triggered by the leading flank of the Smart\* signal, inverted by a
74ALS04 gate.

The main enabling pin, CE\* is controlled by the inverse of CRU bit 3
(it is inverted so that the Smartmedia card will be in standby upon
power-up). This is necessary because the CE\* pin interrupts some
commands when it goes high, so connecting it to the Y0\* output of the
'139 (Smart\* signal) wouldn't work.

Instead, the Smart\* signal is used to mask out the WR\* and RD\*
signals from the demultiplexer, by the mean of two OR gates (74ALS32).
This ensures that the SmartMedia card will only see the read and write
signals that are meant for it. In addition, the RD\* signal is further
masked with address line A12 by an OR gate (74ALS32, same signal used by
the USB controller), so that read operations are disabled in the range
\>4FF8-4FFF. This is necessary to prevent the read-before-write strategy
of the TI-99/4A from confusing the Smartmedia card during write
operations. Reading is only allowed at \>4FF0-4FF7.

The R/B\* (Ready/Busy) pin is connected to CRU input bit 5 (again via
the 74LVT245) so you can tell when the card is not ready to answer.
Since this is an open collector output, it needs to be pulled to 3.3
volts with a 1.5K pull-up resistor, before it goes through the 74LVT245.

CRU input bit 6 detects whether a card is inserted into the connector:
there is a tiny switch all the way inside the connector, so we'll ground
one of its pin and connect the other to the 74LS251, with a 1.5 Kohm
pull-up resistor to +3.3V. If the connector is empty, the switch is
closed and the CRU bit reads as '0'. Inserting a card opens the switch
and bring the CRU input high.

A similar system is used for write-protection detection, except it's not
a switch but a conductive patch on top of the card, whose presence is
sensed by two electrodes. One of them we ground, the other is pulled up
to +3.3V by a 1.5K resistor and connected to CRU input bit 7. If the
card is protected this bit will read as '0', otherwise it reads as '1'.
This signal also goes to the WP\* (write-protect) pin of the Smartmedia
card, via a DIP switch and the 74LVT245 buffer (with another 1.5K
pull-up to 3.3V on its input). When the switch is closed,
write-protection is enforced by the hardware: there is no way you can
write to the card. If you leave the switch open, a write-protected card
can be detected by software, but it is possible to ignore the protection
and write to the card nevertheless.

###  Pinouts

[SRAM M68Z512
](#SRAM%20pinout)[StrataFlash EEPROM
](#flash%20pinout)[SmartMedia card & connector
](#SM%20pinout)[ISP1161 USB controller
](#ISP1161%20pinout)[Voltage regulators
](#voltage%20regulators%20pinout)[Dual MOSFET
](#mosfet%20pinout)[USB connectors](#USB%20connectors)

Pinouts for the "74" chips can be found in my [TTL page](ttl.md).

#### SRAM M68Z512

            +-----+--+-----+
        A17 |1 o         32| Vcc
        A16 |2      M    31| A15
        A14 |3      6    30| A18
        A12 |4      8    29| WE*
         A7 |5      Z    28| A13
         A6 |6      5    27| A8
         A5 |7      1    26| A9
         A4 |8      2    25| A11
         A3 |9           24| OE*
         A2 |10          23| A10
         A1 |11     S    22| CS*
         A0 |12     R    21| D7
         D0 |13     A    20| D6
         D1 |14     M    19| D5
         D2 |15          18| D4
        Gnd |16          17| D3
            +--------------+

**D0-D8**: Data bus.
**A0-A18**: Address bus.
**CS\***: Chip select, active low.
**OE\***: Output enable, active low.
**WE\***: Write enable pulse, active low.

**Vcc**: Voltage supply, +5 volts.
**Gnd**: Ground.

####  StrataFlash EEPROM

            +-----+--+-----+
        CE0 |1 o         56| Vpen
        A12 |2      S    55| RP*
        A13 |3      t    54| A11
        A14 |4      a    53| A10
        A15 |5      t    52| A9
        A22 |6      a    51| A1
        CE1 |7      F    50| A2
        A21 |8      l    49| A3
        A20 |9      a    48| A4
        A19 |10     s    47| A5
        A18 |11     h    46| A6
        A17 |12          45| A7
        A16 |13          44| Gnd
        Vcc |14     2    43| A8
        Gnd |15     8    42| Vcc
         D6 |16     F    41| D9
        D14 |17     6    40| D1
         D7 |18     4    39| D8
        D15 |19     0    38| D0
        STS |20     J    37| A0
        OE* |21     5    36| Byte*
        WE* |22          35| n.c.
        n.c.|23          34| CE2
        D13 |24          33| D2
         D5 |25          32| D10
        D12 |26          31| D3
         D4 |27          30| D11
       Vccq |28          29| Gnd
            +--------------+

**Vcc**: Voltage supply to the chip (+5V)
**Gnd**: Ground
**Vccq**: Voltage supply to data outputs (+5V).
**Vpen**: High to enable writing, low to write-protect

**CE0, CE1, CE2**: Chip selection pins. Chip is enabled if: LLL, LLH,
HLH, or LHH.
**OE\***: Ouput enable. Output data from the chip to the data bus when
low.
**WE\***: Write enable. Input data from the data bus to the chip when
low.
**RP\***: Resets/Power-down. Resets the chip logic when low (the memory
is not affected).
**Byte\***: Data bus width. Low = 8 bits, high = 16 bits.
**STS**: Status pin. Open-collector: low when chip is busy,
high-impedance otherwise.

**A0**: Least-significant address line (weight \>00001). Only used with
8-bit data bus.
**A1-A21**: Address lines.
**A22**: Most significant address line. Not used on the 4M version
(28F320J5).
**D0-D7**: Data bus, least significant byte.
**D8-D15**: Data bus, most significant byte. Only used in 16-bit mode.

####  SmartMedia card & connector

           Card
      ,------------------------------------+
     /       _________        WP           |
    |   Vcc |    |    | Vss  (  )   ,------|
    |   CE* |  | | |  | CLE         |      |
    |   RE* |  | | |  | ALE         |      |
    |   R/B*|  | | |  | WE*         |      |
    |   Gnd |  | | |  | WP*         |      |
    |   LVD |    | |  | D1          |      |
    |    D8 |  | | |  | D2          |      |
    |    D7 |  | | |  | D3          |      |
    |    D6 |  | | |  | D4          |      |
    |    D5 |  | | |  | Vss         |      |
    |   Vcc |    |    | Vss         '------|
    |                                      |
    '--------------------------------------'

         Connector
      +---------------------------------------+
      |                                      1| Vss
      |                                     22| Vcc
    WP|                                      2| CLE
      |                                     21| CE*
    WP|                                      3| ALE
      |                                     20| RE*
      |                                      4| WE*
      |                                     19| R/B*
      |                                      5| WP*
      |                                     18| Gnd
      |                                      6| D1
      |                                     17| LVD
      |                                      7| D2
      |                                     16| D8
      |                                      8| D3
      |                                     15| D7
      |                                      9| D4
      |                                     14| D6
      |                                     10| Vss
      |                                     13| D5
      |                                     11| Vss
      |                                     12| Vcc
      |                                       |
      |                                       | In
      |                                       | In
      +---------------------------------------+

**D1-D8**: Data bus
**CE\***: Card enable, active low. High = abort and stand by.
**CLE**: Command latch enable. High when WE\* = command written.
**ALE**: Address latch enable. High when WE\* = address written. If CLE
and ALE are low: data access.
**RE\***: Read enable, active low.
**WE\*** Write enable, active low.
**WP\***: Write-protection. Low = prevent writing.
**R/B\***: Ready/busy output. Low when busy.

**Vcc**: +5 volts or +3.3 volts, depending on the card.
**Vss**, **Gnd**: Ground.
**LVD**: Low voltage detection. Connected to Vcc in 3.3 volts cards.

**In**: Switch sensing the presence of the card (open = present).
**WP**: Electrodes sensing a write-protection patch (connect =
protected).

#### USB controller ISP1161

                            V  A     H  H  H  H  H  H
                G           r  g  V  O  O  D  D  D  D  D   <--- Read vertically
          D  D  n  n  A  A  e  n  c  C  C  P  M  P  M  D
          1  0  d  c  1  0  g  d  c  2  1  2  2  1  1  P
         ,------------------------------------------------+
        /64 63 62 61 60 59 58 57 56 55 54 53 52 51 50 49  |
    Gnd|1   _                                           48|D_DM
     D2|2  (_)                                          47|H_PSW2*
     D3|3                                               46|H_PSW1*
     D4|4                                               45|Gnd
     D5|5                                               44|XTAL1
     D6|6                                               43|XTAL2
     D7|7                                               42|H_susp
    Gnd|8                                               41|ClkOut
     D8|9                                               40|H_wake
     D9|10                                              39|D_Vbus
    D10|11                                              38|GL*
    D11|12                                              37|D_Wake
    D12|13                                              36|D_susp
    D13|14                                              35|Gnd
    Gnd|15                                              34|EOT
    D14|16                                              33|Ndp_Sel
       | 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32  |
       +--------------------------------------------------+
          D  G  V  n  C  R  W  V  D  D  D  D  I  I  T  R
          1  n  h  c  S  D  R  h  R  R  A  A  n  n  e  s
          5  d  1     *  *  *  2  Q  Q  K  K  t  t  s  t
                                  1  2  1  2  1  2  t  *

**Vcc**: Can be +5 volts or +3.3 volts.
**Vreg**: Outputs 3.3 volts.
**Vh1-2**: Voltage hold. Internally connected to Vreg (+3.3 volts).
**Gnd**: Groud.
**Agnd**: Analog ground.

**D0-D15**: Data bus (MSbit = D0)
**A0-A1**: Address bus. A1 low = host, high = device controller. A0 low
= data, high = command.
**CS\***: Chip select, active low.
**RD\***: Read from chip, active low.
**WR\***: Write to chip, active low.
**INT1-2**: Interrupt request from host (1) & device (2) controllers.
Polarity & duration programmable.
**Rst\***: Chip reset, active low.
**DRQ1-2**: DMA request output from host (1) & device (2) controllers.
Polarity programmable.
**DAK1-2**: DMA acknowledged, to host (1) & device (2) controllers.
**EOT**: End-of-transfer input, for DMA operations. Polarity
programmable.
**Ndp_Sel**: Number of downstream ports selection input. Low = 1, High =
2.

**H_susp**: Host controller suspended output. High = suspended.
**D_susp**: Device controller suspended output. High = suspended.
**H_wake**: Wakes up suspended host controller when high.
**D_wake**: Wakes up suspended device controller when high.

**D_DP**, **D_DM**: Device controller USB signal lines.
**H_DP1**, **H_DM1**: Host controller USB signal lines, first port..
**H_DP2**, **H_DM2**: Host controller USB signal lines, second port..
**H_OC1-2**: Overcurrent sensing inputs for first and second downstream
ports.
**H_PSW1-2**: Power switching outputs, for first and second downstream
ports. Open collector.
**D_Vbus**: Downstream port power sensing to device controller.
**GL\***: Good link indicator output from device controller. Open
collector, 8 mA.

**XTAL1-2**: Connections for 6 MHz crystal.
**ClkOut**: Clock output signal. Frequency programmable.
**Test**: Not used (Ready signal, on later versions).
**n.c.** Not internally connected.

####  Voltage regulators

    +---+
    | ##|
    +++++ 1: Vin
     |||  2: Gnd
    1 2 3 3: Vout

#### Dual MOSFET

      +-------+
    S1|1 o   8|D1      Mosfet #1       Mosfet #2
    G1|2     7|D1      S1 = source     S2 = source
    S2|3     6|D2      G1 = gate       G2 = gate
    G2|4     5|D2      D1 = drain      D2 = drain (2 pins each)
      +-------+

####  USB connectors

Looking inside the female plugs:

     +------------+        ,---------.    1: Power, +5 volts
     ||__________||        |  2   1  |    2: DM signal (0 or +3.3V)
     | 1  2  3  4 |        | |_____| |    3: DP signal (0 or +3.3V)
     +------------+        |  3   4  |    4: Ground
                           +---------+
Revision 1. 6/11/03 Preliminary
Revision 2. 6/29/03 Ok to release

Revision 3.8/4/03 CRU selection by A10+A11. Bit 2 to Vpen. Added
74ALS175 + 74ALS04. Inverted bit3. Interrupt masking circuit redesigned.

Revision 4. 8/18/03 Added Geneve compatibility.

Revision 5. 9/17/03 Added DMA capability.

Revision 6. 10/1/03 Added write-protection switch for EEPROM. Swapped
bits 2 and 4.
Revision 7. 11/3/03. Split in two pages. Swapped data bus for ISP1161.

[Next page](usb2.md) (software issues)

[Back to the TI-99/4A Tech Pages](titechpages.md)
