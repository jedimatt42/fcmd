# The P-Gram card

The P-Gram card was designed by John Guion and sold by Bud Mills. Let me
point out that I don't have one, so the description that follows is
based on schematics, not on hands-on experience.

There was an advanced version, called P-Gram+. As far as I understand,
the only difference hardware-wise was that the P-Gram+ has more memory:
192K instead of 72K for the original version. This allows for the
implementation of four distinct GROM bases (i.e. \>9800, \>9804, \>9808
and \>980C).

**Hardware  **
[Power supply
](#Power)[Lights
](#LEDs)[Bus buffering & enabling
](#Bus%20buffers)[CRU interface
](#CRU)[Address decoding
](#Address%20decoding)[Memories
](#Memory)[Gram address counter
](#Gram%20counter)[Cartridge banks
](#Cartridge%20banks)[DSR space
](#DSR%20banks)[Real-rime clock
](#Clock)[The MM58167 RTC](#RTC58167)

**Software  **
[Low-level access
](#Software)[DSR subprograms
](#Mid-level)[File formats
](#File%20format)[Accessing the clock](#Acessing%20clock)

##  Hardware

#### Power supply

                    7805                          All chips
                +----------+      +5V  1N4001              +-----+
+8V-------------|Vin   Vout|---+----+---|&gt;|---...---+------|Vcc  |    
                |   Ref    |   |    |               |      |     |
                +----------+   = 100|               =0.1uF |     |
                     |         | uF |               |      |     |
               1N914 V        Gnd   |          Gnd--+------|Vss  |
                     |              V 1N914                +-----+
                    Gnd             |
      |                             |    +-----+
Gnd---||---|&gt;|---WWW---+------------+----|Vcc  | SRAMs &amp; clock
      |   1N914  470   |                 |     |
    +3V                = 2.2 uF          |     |
  Battery              |                 |     |
                      Gnd                +-----+

Power is supplied by a +5 volts regulator 7805, through a 1N4001 diode.
It has a 1N914 diode in its ground line to compensate for the voltage
drop caused by a similar diode on its output.

There is also a +3 volts battery to maintain the contents of the SRAMs
and the clock when power is off. A 1N914 diode and a 470 Ohm resistor
prevent the regular +5 volts power supply from fighting the battery when
power is on, whereas another 1N914 diode prevent the battery current
from flowing to the rest of the card (and the PE-box) when power is off.

Each chip on the card has its own 0.1 uF bypass capacitor, between its
power pin and the ground. This helps filtering power spikes. The
regulator itself has a 100 uF capacitor, the battery has a 2.2 uF
capacitor.

####  Lights

The P-Gram card contains two LEDs. The first one is visible through the
PE-box front window, it is permanently on (when power is on, that is).
For some reason, it is controlled by a transistor and a buffer, which
means that it could be toggled by CRU bit 0, if we wanted to.

                 ,--|&lt;|---+5V
     2.2K      |/
+5V--WWW--|&gt;---|
               |\   100 Ohm
                 &#39;--WWW---Gnd

The second LED faces upwards and is used for test purposes. See
[below](#LED%202) for schematics.

####  Bus buffering and enabling

As specified by Texas Instruments, all PE-box signals are re-buffered on
card. There are two 74LS244 to buffer the address bus, one more to
buffer some control lines (MEMEN\*, WE\*, CRUCLK\*, DBIN and RESET\*),
and a 74LS245 to buffer the data bus. The direction of the 74LS245 is
controlled by DBIN, and it is enabled by the P-Gram card, together with
the RDBENA\* line that activates remote data bus buffers in the
connection card and cable.

This enabling signal is a combination of the selection signals for the
three SRAMs, plus the \>9C02 signal that is used to load a GRAM address
into the counters (see below). If the optional real-time clock is
installed, its enabling circuitery generates signal Sel5\* to enable the
data bus. If the clock is absent, a 4.7K resistor maintain this line
permanently high.

           +-------+                 +-------+        
     A0&gt;---|       |---A0      A8&gt;---|       |---A8
      .    |       |    .       .    |       |    .      
      .    |       |    .       .    |       |    .      
      .    |       |    .       .    |       |    .  
      .    |       |    .       .    |       |    .      
      .    |       |    .       .    |       |    .
     A7&gt;---|       |---A7      A15&gt;--|       |---A15      
      Gnd--|EN*    |            Gnd--|EN*    |           
           +-------+                 +-------+    
              4LS244                        
           +-------+         
           |       |            
 MEMEN*&gt;---|       |---Memen*  
    WE*&gt;---|       |---WE*          
CRUCLK*&gt;---|       |---CruClk*      
           |       |          
 RESET*&gt;---|       |---Reset*      
   DBIN&gt;---|       |---+---DBIN   
      Gnd--|EN*    |   |     
           +-------+   |    
            74LS245    |      
           +-------+   |      
           |    DIR|&lt;--&#39;      
    D0&lt;---&gt;|       |-----D0       
     .     |       |      .     
     .     |       |      .    
     .     |       |      .   
     .     |       |      .               ,--Sel5* (optional)
     .     |       |      .        4.7K   |     _
     .     |       |      .   +5V---WWW---+    ( |---Sel1* 
    D7&lt;---&gt;|       |-----D7               |   (  |---Sel2* 
           |    OE*|&lt;--------+----+----(|=---(   |---Sel3*  
           +-------+         |    |           (  |---Sel4* 
                             |    |            (_|     
RDBENA*&lt;---------------------&lt;|---&#39;
                          74LS367 

####  CRU interface

The CRU interface is quite simple and offers a maximum of 8 bits in
output mode only. The address can be selected with a DIP switch, among a
choice of 8 addresses.

          +-------+              CRU          +-------+
A7--------|S0  Y0*|-----o o--,  &gt;1000   A12---|S2   Q0|-=|)o------Bit 0*
A6--------|S1  Y1*|-----o o--+  &gt;1100   A12---|S1   Q1|-----=|)o--Bit 1*
A5--------|S2  Y2*|-----o o--+  &gt;1200   A12---|S0   Q2|---Bit 2
A4--------|G2B*Y3*|-----o o--+  &gt;1300         |     Q3|---Bit 3
A3--------|G1  Y4*|-----o o--+  &gt;1400   A15---|D    Q4|---Bit 4
          |    Y5*|-----o o--+  &gt;1500         |     Q5|
CRUCLK*---|G2A*Y6*|-----o o--+  &gt;1600  Reset*-|RST* Q6|
          |    Y7*|-----o-o--+  &gt;1700     ,---|EN*  Q7|
          |       |          &#39;------------&#39;   +-------+
          +-------+

The decoder 74LS138 is enabled by the CRUCLK\* line for CRU output
operations. It is further enabled by lines A3, which must be high, and
A4 which must be low. Thus valid addresses are thus in the range \>1000
through \>1700. Lines A5-A7 enable one of the 8 outputs of the decoder,
according to the address. By closing the corresponding DIP-switch you
select this address as the active one. Note that only one of the
switches should be closed, so that the card only answers to one address.

The output of the switch then enables the 74LS259 addressable latch, in
a very standard manner. A12 through A14 provide the bit number, A15 the
bit value. RESET\* is used to make sure that all bits return to 0 at
power-up time.

Only five CRU bits are actually used, the first two being inverted by
NAND gates mounted as inverters (i.e. with their inputs tied together).

Bit 0: A 1 enables RAM at \>4000-5FFF for DSRs
Bit 1: A 1 enables GRAM and cartridge RAM banks at \>6000-7FFF
Bit 2: A 1 write-protects all memories
Bit 3: A 0 forces access to bank 0 in either RAMs (DSRs and cartridge
RAM).
Bit 4: A 0 forces access to bank 1 in either RAMs.
When both bit 3 and 4 are 1, banks are selected by writing at
\>6000/6002 for cartridge banks, or \>4000/4002 for DSRs. When both bits
are 0, bit 4 wins.

####  Address decoding

            +-------+ Bit 0*-,                   +-------+        +-------+
       A2---|S0   Q2|--------=)&gt;----+---DsrEn*   |    Y6*|--------|G*  Y0*|---,
       A1---|S1     | Bit 1*--,     |            |       |  A14---|S0  Y1*|---=|)-----+---Data*
       A0---|S2   Q3|---------=)&gt;---=|)---Sel4*  |       | DBIN---|S1  Y2*|--+----LD* |
            |       |             74LS08   Gnd---|S0     |        |       |  |        | 74LS08
   Memen*---|G2A*   |                       A4---|S1     |        |       |  |       ,-=|)--CK*
      Gnd---|G2B*   |             A5---,    A3---|S2     |        |       |  |       |
+5V--WWW-+--|G1     |           DBIN---=|)o------|G1     |        +-------+  |       |
    2.2K |  |       |     Bit 1*--, 74LS00 A15---|G2B*   |              WR*---=)&gt;----+---Sel1*
Gnd--o o-&#39;  |     Q4|---+---------=)&gt;------------|G2A*   |                  74LS32
            +-------+   |       74LS32           +-------+   
                        = 47 pF
                        |
                       Gnd

A first 74LS138 8-bit decoder is used to split the address range into
eight 8K blocks: it is enabled by MEMEN\* and takes its selection inputs
from address lines A0-A2. Note the main switch connected to the G1
enabling input: when closed, it prevents the card from answering to any
memory address. Very usefull if the DSRs got corrupted and prevent
booting...

Only three outputs are used: Y2\* which reacts to the range \>4000-7FFF
(DSR space), Y3\* which reacts to \>6000-7FFF (cartridge banks) and Y4\*
which reacts to \>8000-9FFF. The first two are combined with the
inverted CRU bits 0 and 1 respectively, to provide selection signals.
These two signals are futher combined to select the second 32K SRAM
chip.

The \>8000-9FFF address block is further decoded by another 74LS138,
which makes sure that the address is in the range \>98xx-9Cxx (by
checking A3 and A4), that it is even (by checking A15) and that no input
operation is allowed at the write ports \>9Cxx, i.e. when A5 is high (by
ANDing DBIN and A5). Note that the signal from the first decoder is
combined with CRU bit 1 via an OR gate, so that GRAMs can be
enabled/disabled at will. This bit was inverted by a NAND gate, so that
GRAM is disabled upon power-up (when all CRU bits will be 0). This is
critical to prevent lock-ups in case crazzy values would be found in the
GRAM.

Yet another decoder, one half of a dual 4-bit 74LS139, is used to
differentiate between the four ports, by checking A14 and DBIN. Only
three of its four outputs are used: Y0\* reacts to \>9800, Y1\* reacts
to \>9802, and Y2\* reacts to \>9C00. The first two signals are combined
with an AND gate to provide an active low Data\* signal when data is
accessed. The last one provides the LD\* signal that will load the
address counter (see [below](#Gram%20counter)). It is also combined with
WR\* via an OR gate to contribute to the Sel1\* data bus enabling
signal. Finally, the Sel1\* signal and the data signal are combined via
an AND gate to provide the CK\* clock signal for the GRAM address
counter (the 74LS161A needs a clock signal to load data as well as to
increment the count).

####  Memory

The P-Gram card comprises three memory chips: two 43256LP (SRAM 32K) and
one 6264LP (SRAM 8K). The 8K SRAM and one of the 32K are used to emulate
GROMs. The second 32K SRAMs serves a double purpose: it emulates
cartridge banks and holds the DSRs. The memory chips are low-power
SRAMs, which means that their power supply can be backed up by a +3
volts lithium battery, so their contents will be preserved even when the
PE-box power is off.

             +--------+         +--------+              +--------+
      Vbat---|Vcc     |---------|Vcc     |--------------|Vcc     |
D0-----------|D7      |---------|D7      |--------------|D7      |
...          |...     |         |...     |              |...     |
D7-----------|D0      |---------|D0      |--------------|D0      |
             |        |         |        |              |        |
GA15---------|A0      |---------|A0      |        A15---|A0      |
...          |...     |         |...     |        ...   |...     |
GA12---------|A12     |---------|A12     |         A3---|A12     |
GA2----------|A13     |         |        |   74LS74/Q---|A13     |
GA1----------|A14     |         |        |     DsrEn*---|A14     |
A13----------|(A15)   |---------|(A13)   |              |        |
A12----------|(A16)   |---------|(A14)   |              |        |
Bit2--,74LS32|        |         |        |              |        |
WE*---=)&gt;--+-|WR*     |---------|WR*     |--------------|WR*     |
 +5V--WWW--&#39; |        |         |        |              |        |
      1K     |        |         |        |              |        |
DBIN--=|)o---|RD*     |---------|RD*     |--------------|RD*     |
     74LS00  |        |         |        |              |        |
Sel2*--|&gt;-+--|CS*     | Sel3*-+-|CS1*    |  Sel4*--|&gt;-+-|CS*     |
74LS367 | |  |        |       | |        |          | | |        |
       Gnd|  +--------+       | +--------+        Gnd | +--------+
      1K  |    Gram       1K  |   Gram            1K  |  RAM banks
Vbat-WWW--&#39;       Vbat---WWW--&#39;           Vbat---WWW--&#39;  &amp; DSRs      

All memory chips are connector to the data bus, but their address busses
differ: the first 32K and the 8K take their addresses from a counter
(GROM emulation), whereas the second 32K chips takes its addresses
directly from the address bus (DSR space and cartridge banks). To OE\*
output enable pin of all three chips is controlled by an inverted DBIN
signal. Their WE\* pins are controlled by the WR\* line, after due
masking by CRU bit 2 with an OR gate. This allows to write protect the
memories by setting this bit high.

The P-Gram+ uses a 128K SRAM chip instead of a 32K for GROM emulation
and a 32K instead of the 8K. The two extra address pins are connected to
address lines A12 and A13, as shown in brackets in the above schematics.
Note that this means that only four GROM bases out of 16 are
implemented: \>98x0, \>98x4, \>98x8 and \>98xC. Because decoding is
incomplete, the others bases are redundant: for instance, GRAM at \>9800
can also be accessed at \>9810, \>9820 and \>9830.

The 128K SRAM chip has 32 pins, whereas 32K and 8K chips have only 28
pins. This allows to have a 32-pin socket installed, into which you can
plug either a 32K or a 128K SRAM. The extra address lines come on the
extra pins, and the pin that is used for Vcc on 28-pin chips now
corresponds to an active-high CS2 pin on 32-pin chips, which is just
fine.

Things are more complicated when replacing the
8K with a 32K. Because the extra address pin A13 (#26) in the 32K
corresponds to chip select pin CS2 on the 8K, there is a small switch
onboard that lets you select whether you have a P-Gram or a P-Gram+. In
its "plus" position, the switch connects pin 26 of the memory chip to
address line A13. In the "nonplussed" position, it connects pin 26 to
+5V via the control LED that's always on. When power is off, the CS2 pin
puts the chip is high-impedance state to minimize power consumption.

  +--------+     o---A13
  | CS2/A13|---o           LED
  |        |     o-----+---|&lt;|---+5V
  +--------+       1K  |
             Gnd--WWW--&#39;   

####  GRAM address counter

GROMs have an internal address counter that gets incremented after every
data access. A new address can be loaded in the couter as two successive
bytes. Since the G-memory space is 64K, we'll need a 16 bits counter.
The P-Gram card makes one such counter by cascading four 4-bit loadable
counter chips 74LS161A. The resulting counter is incremented when the
CK\* provided by the 74LS139 decoding the address, changes from low to
high. The output of the counters is fed directly to the address lines of
the SRAM chips.

      +-------+         +-------+         +-------+
D0----|P3   Q3|---GA8---|P3   Q3|---GA0---|S2  Y0*|
D1----|P2   Q2|---GA9---|P2   Q2|---GA1---|S1  Y1*|
D2----|P1   Q1|---GA10--|P1   Q1|---GA2---|S0  Y2*|
D3----|P0   Q0|---GA11--|P0   Q0|---GA3   |    Y3*|---------Sel3*
      |       |         |       |         |    Y4*|---| )
CK*---|CLK    |   CK*---|CLK    |   +5V---|G1  Y5*|---|  )
LD*---|LD*    |   LD*---|LD*    |   Gnd---|G2A*Y6*|---|  )--Sel2*
+5V---|RST*   |   +5V---|RST*   | Data*---|G2B*Y7*|---|_)
+5V---|ENP    |   +5V---|ENP    |         |       | 74LS21
  ,---|ENT RC0|----, ,--|ENT RC0|         |       |
  |   +-------+    | |  +-------+         +-------+
  &#39;--------------, | --------------,
      +-------+  | |    +-------+  |
+5V---|ENT RCO|--&#39; &#39;----|ENT RCO|--&#39;
D4----|P3   Q3|---GA12--|P3   Q3|---GA4
D5----|P2   Q2|---GA13--|P2   Q2|---GA5
D6----|P1   Q1|---GA14--|P1   Q1|---GA6
D7----|P0   Q0|---GA15--|P0   Q0|---GA7
      |       |         |       |
CK*---|CLK    |   CK*---|CLK    |
LD*---|LD*    |   LD*---|LD*    |
+5V---|RST*   |   +5V---|RST*   |
+5V---|ENP    |   +5V---|ENP    |
      +-------+         +-------+

Loading an address into a GROM is done by passing two bytes at port
\>9C02, the MSB first, then the LSB. GROMs have an internal flip-flop
that toggles after the first byte is written and won't toggle back until
data is accessed. The P-Gram card has a somewhat different behaviour:
the incoming byte becomes the LSB and the current LSB becomes the MSB.
This should not bear any consequences though, because addresses should
always be passed as two bytes. And it makes design so much easier: the
data bus is connected to the inputs of the first two 74LS161A chips,
whereas the next two takes their input from the output of the first two.

The card does not provide a way to read back the address, which is
complicated because GROMs actually return the address plus one.
Fortunately, the console GROMs will do it, whatever the address, so
there is no need to implement this feature in hardware.

The TI-99/4A already contains three GROMs, at addresses \>0000, \>2000
and \>4000. Although it is possible for a GRAM card to override these,
it may result in damaging the GROMs (although this never happened to
me). For this reason, the P-Gram card takes care not to answer to any
GRAM address under \>6000. This is achieved by decoding the top 3 bits
of the address counter with a 74LS138 decoder, which is enabled by data
access operations.

The first three outputs of the decoder, which correspond to the console
GROMs addresses, are not connected. The next one, GRAM \>6000, enable
the 8K SRAM, whereas the last 4 collectively enable the 32K SRAM.

####  Cartridge banks

Some cartridges have cpu memory that appears in the \>6000-7FFF area.
This 8K memory can be expanded by paging it into two banks. The
selection of a bank is performed by writing to the \>6000-7FFF area: to
\>6000 to select bank 0, to \>6002 for bank 1. Some GRAM cards allow for
more than two banks, but the P-Gram doesn't.

            +-------+           +-------+  
WE*----,    |     Q2|-----------|A13    |
Sel4*--=)&gt;--|CK     |  DsrEn*---|A14    |
     74LS32 |       |           |       |
      A14---|D      |           +-------+
     Bit3---|RST*   |    
     Bit4---|SET*   |    
            +-------+  

The memory itself is implemented as two 8K banks, which are part of the
second 32K SRAM chip (the rest of the chip is used for DSRs). Bank
switching is achieved by a 74LS74 flip-flop, which is clocked by write
operations to the \>6000-7FFF range when CRU bit 1 is high, and/or to
the \>4000-5FFF range when CRU bit 0 is high (i.e. the Sel4\* signal
described [above](#Address%20decoding)). The flip-flop memorizes A14
(i.e. \>xxx2) and sends its output to the second address pin of the
SRAM.

The flip-flop is reset when CRU bit 3 is low, thereby forcing access to
bank 0, or set when CRU bit 4 is low, thereby forcing access to bank 1.
This is necessary so that you can write data into a bank without
switching it. ( N.B. If both bit 3 and bit 4 are low, bank 1 is
selected).

####  DSR space

The second 32K SRAM also serves to hold DSRs at addresses \>4000-5FFF.
This space is paged as two 8K pages by the same mechanism as the
cartridge banks (except that toggling is achieved by writing to \>4xx0
or \>4xx2). Note that this means that toggling DSR pages automatically
toggles RAM banks and conversely.

The memory chip is enabled when CRU bit 0 is high and an access occurs
in the \>4000-5FFF range (by the DsrEn\* signal that is part of Sel4\*,
see [above](#Address%20decoding)). The DsrEn\* enabling line is also
applied to the first address line of the SRAM, which will thus be low
for DSRs and high for cartridge banks.

####  Real-time clock

The P-Gram card also comprises an optional real-time clock MM58167A,
which requires little additional circuitery. It looks like it was added
as an afterthough, as it makes little use of the existing PGram
circuitery. As a matter of fact, its wiring is highly reminiscent of the
[MBP card](mbp.md), which uses the same chip.

         +-------+         +-------+             +-------+
    A2---|S0     |    A9---|S0  Y1*|----Sel5*----|CS*  D0|---D7
    A1---|S1     |    A8---|S1     | DBIN-=|)o---|RD*  D1|---D6
    A0---|S2     |    A7---|S2     |       WE*---|WR*  D2|---D5
         |    Y4*|---------|G2A*   |       A14---|A0   D3|---D4
Memen*---|G2A*   |    A4---|G2A*   |       A13---|A1   D4|---D3
    A3---|G2B*   |    A5---|G1     |       A12---|A2   D5|---D2         Crystal
    A6---|G1     |         |       |       A11---|A3   D6|---D1         32.768 kHz
         |       |         |       |       A10---|A4   D7|---D0     200K      20 pF
         +-------+         +-------+  +5V--|&gt;|-+-|PO*  OI|--------+-WWW---+---||---Gnd
                                    Gnd---WWW--&#39; |     OO|-+--WWW-&#39;      ===
                                           1K    +-------+ &#39;--------------+---||---Gnd
                                                             20M            3-30 pF

The address decoding is performed by two 74LS138 decoders, which
together react to memory operations in the range \>8400-87FF. The
selection signal Sel5\* also enables the address bus, as seen
[above](#Bus%20buffers). Note that these addresses correspond to the
sound chip in the console, which means that you may occasionally
generate sounds when writing to the clock. As the sound chip cannot be
accessed for input, read operations only access the clock.

The clock itself is a MM58167A chip. It has an 8-bit data bus which is
connected to the PE-box data bus, taking into account that D0 is the
most significant bit for TI, but the least significant bit for the rest
of the world. The clock also has five address pins, connected to A10
through A14, which provide access to a total of 32 registers. Its WE\*
write-enabling pin is connected to the WR\* line and the RD\*
read-enabling pin to an inverted DBIN (that comes from the Gram part of
the card).

The PO\* pin is used to put the clock in standby mode when the power is
off. It is connected to the regular +5V power supply, via the second
LED. When power is off, this line goes low and place the chip in standby
mode.

Finally, the clock needs a 32.768 kHz crystal, with its associated
circuitery: two caps (one 20 pF one adjustable 3-30 pF or 6.8-45 pF) and
two resistors. The clock receives its power from the same battery-backed
power line that's used for the SRAMs. This is obviously necessary so it
can keep time while power is off.

The clock chip has the ability to generate a variety of interrupts, but
the corresponding pins are not connected in the P-Gram design, so this
feature cannot be used to generate alarms.

###  The MM58167A real-time clock

The MM58167A maintains the time from the milliseconds to the months, but
does not support years. An alarm can be set that will automatically
trigger an interrupt pin. The interrupt pin can also be triggered by a
variety of periodic events.

When power is off, the clock can be placed in standby mode and powered
by a +3 volts battery. In this mode all outputs are disabled, except for
a special dedicated interrupt pin.

The clock is accessible via a set of 32 registers (of which only 25 are
used). Their function is described in the [software
section](#Acessing%20clock) of this page.

[Pinout
](#Clock%20pinout)[Absolute maximum ratings
](#Clock%20max%20ratings)[Electrical characteristics
](#Clock%20electrical)[Timing diagrams](#Clock%20timing)

#### Pinout

          +----+--+----+ 
       CS* |1 o       24| Vcc
       RD* |2         23| PwrDown*
       WR* |3    R    22| D7
      RDY* |4    T    21| D6
        A0 |5    C    20| D5
        A1 |6    5    19| D4
        A2 |7    8    18| D3
        A3 |8    1    17| D2
        A4 |9    6    16| D1 
     OscIn |10   7    15| D0
    OscOut |11        14| StdbyInt*
       Vss |12        13| IntOut
           +------------+

**D0-D7** form the data bus. D0 is the least significant bit.

**A0-A4** form the address bus that lets you select one input for the
conversion. A0 is the least significant bit.

**CS\*** is the chip select signal.
**RD\*** triggers read operations.
**WR\*** triggers write operations.
**PwrDown\*** when low, puts the clock in standby mode (i.e. outputs
disabled, minimal current usage).

**RDY\*** when high, signals the clock is not ready to answer.
**IntOut** signals that an interrupt occured (active high).
**StdbyInt** signals that an interrupt occured while in standby mode
(active low, open collector).

**OscIn** Oscillator input. Requires a 32.768 kiloHerz signal.
**OscOut** Oscillator output.

**Vcc** Power supply, normally +5V, can be as low as +2V in standby
mode.
**Vss** Ground.

####  Absolute maximum ratings

    Supply voltage Vcc-Vss:      6.0 V
    Voltage on all pins:         Vss-0.3 to Vcc+0.3 V
    Storage temperature:         -65 to 150 `C
    Operating temperature:       0 to 70 `C

####  Electrical characteristics under recommended conditions

Standby
2.2
5.5
V
Vih = 2.0 V
5
mA
Low
0.0
0.6
V
High

Tri-state
Ioh = -400 uA
Ioh = -10 uA
Vss &lt;= Vout &lt;= Vcc
2.4
0.8Vcc
-1


+1
V
V
uA
High (i.e leakage)
Vout &lt;= Vcc
10
uA

####  Timing diagrams

##### Read cycle
    XXXX    Valid   address                                  XXXXXXXXX A0-A4 
    ___|___                                     |   >50 ns   |   _____
       |   \                                    |               /      CS*
       |    | >0 |                              |
       |        \                              /|                      RD*
       | >100 ns |                              |
       |                <1050 ns             |  | 0-250 ns|
    ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZX Valid data XZZZZZZZZZZZ D0-D7
                 |                      |<100|  |
                 |<150 |                | >0    |
                      \                /                               READY*
                       |    <800 ns          |

##### Write cycle
    XXXX    Valid   address                                  XXXXXXXXX A0-A4 
    ___|___                                     |  >50 ns    |   _____
       |   \                                    |               /      CS*
       |    | >0 |                              |
       |        \                              /|                      WR*
       | >100 ns |                              |
       |    |>100|                              | >110|
    ZZZZZZZX     Valid   data                         XZZZZZZZZZZZ D0-D7
                 |                              |
                 |<150 |                | >0    |
                      \    <800 ns     /                               READY*

------------------------------------------------------------------------

## Software

[Low-level access
](#Low-level%20GRAM)[DSR subprograms
](#Mid-level)[File formats
](#File%20format)[Accessing the clock](#Acessing%20clock)

### Low-level

The P-Gram card is essentially controlled by the CRU.

For normal operations, bit 1 should be set the enable the card, bit 2
should be set to write-protect the memory, and bits 4 and 5 should be
set to enable bank switching. This is done by LDCRing the value \>1B.

To load data into GRAM/RAM the write-protection bit (CRU bit 2) should
be reset. Also, the bank switching mechanism should be frozen, so that
banks do not switch each time you write new data into them. This is done
by reseting either CRU bit 3 (to write to bank 0) or CRU bit 4 (for bank
1). The values to load with LDCR are \>12 for bank 0 and \>0A for bank
1.

When emulating a cartridge like Mini-memory that has a RAM bank, you
should also enable writing and disable bank switching. Use \>12 as a
LDCR value.

To access the DSR space, bit 0 should be set. Also, one of the bank
switching bits (bits 3 or 4) should be 0 to select the desired page. For
page 0 LDCR the value \>11, for page 1 LCDR the value \>09. Note that
bit 2 should also be 0 if you mean to write to the DSR space.

       LI   R1,&gt;1E00   Normal mode
       LDCR R1,5       Card on, write-protect, switch banks`
       LI   R1,&gt;1200   Write-enable, freeze bank 0
       LDCR R1,5       `
       LI   R1,&gt;0A00   Write-enable, freeze bank 1
       LDCR R1,5`
       LI   R1,&gt;1100   DSR access, freeze bank 0
       LDCR R1,5 `
       LI   R1,&gt;0900   DSR access, freeze bank 1
       LDCR R1,5`
       LI   R1,&gt;0000   Card off
       LDCR R1,5

###  Mid-level

#### Assembly routines

The DSRs that come with the P-Gram card provide a number of assembly
subroutines to control the card. These should be called with DSRLNK,
DATA \>000A, after placing the subroutine name in the VDP memory and
pointing at it with \>8356.

*-----------------------------
CALL30 LI  R1,&gt;0130     Subroutine &gt;30 (with leading size byte)
       LI  R0,&gt;1000     Address in VDP memory
       BLWP @VSMW       Write size byte
       MOV  R0,@&gt;8356   Save the address for DSRLNK
       SWPB R1          Move to name
       INC  R0          Next address
       BLWP @VSBW       Write the name
       BLWP @DSRLNK     Link to the subroutine
       DATA &gt;000A       Code for &quot;subroutine&quot;        
**Sub \>30**: Select GRAM and RAM bank 1

**Sub \>31**: Select GRAM and RAM bank 2

**Sub \>32**: Select GRAM, write protect.

**Sub \>34**: Turn P-Gram card off.

**Sub \>35**: Perform the power-up checks.

**Sub \>36**: Invoke the GRAM loader.

**Sub \>37**: Invoke the memory editor.

####  Basic routines

From Basic (or X-Basic) the following subroutines can be called:

**CALL PG** Invoke the P-Gram menu.

**CALL PGON** Force the P-Gram card on.

**CALL PGOFF** Force the P-Gram card off.

**CALL PGZAP** Destroy any module header in GRAM/RAM (i.e. makes the
card appear empty). This is done by changing the \>AA tag that indicates
a standard header, to \>00. It is usefull if a corrupted module causes
problems. Once the problem is fixed, you can always change back the \>00
to \>AA with the memory editor.

**CALL PTIME** Invoke the clock access program.

There is also a DSR called **CLOCK** that lets you access the clock as a
file, the same way a CorComp card does. This pseudo-file contains three
records with each one string:

Record 0 is the day of the week: "0" = Sunday.
Record 1 is the date. E.g. "03/25/02"
Record 2 is the time in 24h format. E.g. "21:05:54"

###  File format

The file format used the the PG loader/saver is very similar to the one
popularized by the GramKracker, except for one thing: if byte \>xx03 of
any [GROM header](headers.md) is equal to \>A5, the first RAM bank will
be frozen and enabled for writing (this allows to emulate cartridges
with RAM banks). Any other value (normally \>00 since this byte is not
used in standard headers) cause the banks to be write protected and sets
switching mode.

The files are of "program" type, and start with a 6-byte header that
provides some information to the loader. The rest of the file contains a
memory image.

    Byte   Meaning
    >00     >00=last file. >FF=more to come.
    >01     Loading flag (see below)
    >02-03  Number of bytes to load
    >04-05  Loading address in GRAM or RAM

The loading flag can be:

    >04: GRAM at >6000
    >05: GRAM at >8000
    >06: GRAM at >A000
    >07: GRAM at >C000
    >08: GRAM at >E000
    >09: RAM bank 0
    >0A: RAM bank 1

###  Accessing the clock

The MM58167A real-time clock interface consists in 32 registers. They
can be accessed at addresses \>8640 through \>867E. Only even addresses
are considered, although both bytes are transfered: so just ignore the
LSB.

All values are in binary-coded decimal (aka BCD), i.e. number from \>00
to \>09 represent 0 to 9, then \>10 to \>19 represent 10 to 19, etc. I
know it's a drag, but that's the way it is. To convert decimal to BCD
and conversely, use the following formulas:

    BCD = DECIMAL + 6 * INT(DECIMAL/10)
    DECIMAL = BCD - 6* INT(BCD/16)

#### Extended Basic

When working from Extended Basic, you can use CALL LOAD and CALL PEEK to
access the clock. Clock addresses start at -31168, and go up from there.
For instance, to set and read back the time, you could do:

100 CALL LOAD(-31164,SECS,0,MINS,0,HRS,0)
110 CALL SOUND(1,110,30)
120 CALL PEEK(-31164,SECS,X,MINS,X,HRS,X)

Line 110 is necessary to shut off the sound generators that could be
affected by writing to the clock (since they share the same address). X
is used to dump the least significant bytes.

For more details, see the "Assembly" section. Just convert the
hexadecimal addresses to signed decimal.

#### Assembly

In assembly, the clock registers can be accessed at the following
addresses:

\>8640 1/1000 seconds (0-9)
\>8642 1/100 seconds (0-99)
\>8644 seconds (0-59)
\>8646 minutes (0-59)
\>8648 hours (0-23)
\>864A day of the week (1-7, Sunday=1)
\>864C day of the month (1-31)
\>864E month (1-12)

The year is not stored inside the clock chip. The P-Gram DSRs save it
(together with the month) in the DSR space and updates the year upon
power up, if needed.

To implement alarms, the same information can be saved in the clock RAM
at the following addresses:

\>8650 1/1000 seconds (\>Cx = always true)
\>8652 1/100 seconds (\>Cx = always true)
\>8654 seconds (\>Cx = always true)
\>8656 minutes (\>Cx = always true)
\>8658 hours (\>Cx = always true)
\>865A day of the week (\>xC= always true)
\>865C day of the month (\>Cx = always true)
\>865E month (\>Cx = always true)

The clock chip can be set to automatically compare these values with the
current time and to trigger an interrupt if they match. Setting the
first two bit of any RAM address causes any value to match the current
time (except for day-of-week that should be set as \>0C). This allows
for setting a daily alarm, an hourly alarm, etc.

The remaining registers are:

\>8660 Interrupt status register
\>8662 Interrupt control register
\>8664 Writing \>FF here resets the clock
\>8666 Writing \>FF here resets the RAM values
\>8668 Status bit: \>01 = time was updated. Reset when read.
\>866A Writing here resets the seconds, 100th and 1000th. Minutes are
incremented if seconds were 40 or higher.
\>866C Standby interrupt: \>01 to enable, \>00 to disable.
\>866E-7C These registers do not exist
\>867E Test mode, do not use.

The **interrupt control register** is used to determine when interrupt
will occur

    Bit  Interrupt when
    >80  RAM matches time
    >40  Every 10th of second
    >20  Every second
    >10  Every minute, at 00 seconds
    >08  Every hour, on the hour
    >04  Every day, at 00:00
    >02  Every week, on Saunday at 00:00
    >01  Every month, on the first at 00:00

The **interrupt status register** mirrors this structure. It indicates
which interrupt has fired, if any. Reading this register also clears the
interrupt condition. This kind of interrupt is signaled on pin \#13 of
the clock (active high), which is not connected in the P-Gram card. So
an actual interrupt will not occur, but you can always check the
interrupt status register periodically, to see if an alarm has fired.
This is easier than checking the time yourself.

There is another interrupt pin, \#14, which is active (low) even when
the clock is in standby mode, i.e. when power is off. This pin only
reacts to a match between the RAM and the clock. Write \>01 to \>866C to
enable it, write \>00 to disable it. Note that, since this pin is not
connected either, this interrupt is completely useless.

The **status bit** should be read before and after each access to the
current time. If it is \>01 the time was updated while you were reading
it, so the values may be incorrect (e.g. if it was midnight). In this
case, read the time again. The bit is only set if the time was rippling,
e.g. if incrementing the seconds changed the minutes, etc.

To start the clock more precisely (e.g. when the user presses a key),
you can write to \>866A, which resets the seconds to 0. In case the
seconds were higher than 39 the minutes will be incremented.

Writing \>FF to \>8664 and \>8666 reset the clock and the RAM,
respectively. It's not overly useful (quite dangerous in fact), so
beware of these addresses!
Revision 1. 4/24/02. OK to release.
Revision 2. 5/1/02. Added chapter on clock.

[Back to the TI-99/4A Tech Pages](titechpages.md)
