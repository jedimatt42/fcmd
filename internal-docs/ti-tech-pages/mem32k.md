# The 32K memory expansion card

This card provides 32K of RAM, mapping in two blocks: the low memery
expansion at \>2000-3FFF and the high memory expansion at \>A000-FFFF.
The memory consists in sixteen MK4116 DRAM chips, each of 16Kx1 bits.

[MK4116 pinout
](#pinout)[Additional circuitery
](#circuitery)[Timing diagrams
](#Timing%20diagrams)[Electrical charateristic](#Electrical)

[Do-it-yourself card](#doit)

##  MK4116 pinout

         +---+--+---+
     Vee |1  +--+ 16| GND
       D |2       15| CAS*
     WE* |3   MK  14| Q
    RAS* |4  4116 13| A6
      A0 |5       12| A3
      A2 |6       11| A4
      A1 |7       10| A5
     Vdd |8        9| Vcc
         +----------+

Power supply
**Vcc** +5V
**Vdd** +12V
**Vee** -5V
**Gnd** Ground

Control bus
**WE\*** Write enable
**RAS\*** Row address select: The chip is a matrix of 128x128 bits. To
access a bit, the row address is placed on A0-A6, then RAS\* goes low to
latch the row address. This also refreshes the whole row.
**CAS\*** Column address select. Once the row is selected, the column
address in placed on A0-A6 and CAS\* goes low, which selects a given bit
in the chip. If WE\* is low, the bit will be given the value of D. If
WE\* is high, the bit value will be output on Q.

Address bus
**A0-A6** Row address / column address. Multiplexed via the RAS\*
and CAS\* signals.

Data bus
**D** Data bit in.
**Q** Data bit out.

##  Additional circuitery

A complicated circuitery is in charge of refreshing the DRAM chips by
composing all the row addresses one after the other, when the card is
idling (i.e. when MEMEN\* is high). This circuit comprises:

    3  74LS74   dual-D flip-flop
    1  74LS109  dual JK flip-flop
    1  74LS161  4-bit counter
    1  74LS393  dual 4-bit counter
    1  74LS244  dual 4-bit buffer, 3-state

The selection logic is achieved by a proprietary chip.

The PE-Box address bus is buffered by two 74LS244 buffers, then lines A2
through A15 are multiplexed as 2 times 7 bits via 2 74LS257 quad 2-to-1
multiplexers. An extra 74LS244 buffers the control bus, and a 74LS245
buffers the data bus (its direction is controlled by the DBIN line of
the control bus).

The LED indicator is controlled by a 3964 transistor driven by a 74LS123
one-shot multivibrator.

Note that the card does not contain any ROM, nor does it have a CRU
interface.

##  Timing diagrams

These diagrams come from the Mitsubishi equivalent: M5K4116P,S-3 (200
ns).

#### Read cycle

           |                 >375 ns                   |
           \           200-1000 ns             />120 ns\     RAS*
            |             >200 ns          |   |       |
            | 25-65 ns  |       >135 ns        |       |
            |          \     >135 ns      /|  > -20 ns |  \_ CAS*
            |     >120 ns      |           |
      | >0  | >25 |  |a |>55 ns|           |
    XX    row     XXX  column  XXXXXXXXXXXXXXXXXXXXXXXXXXXXX A0-A6
            |    |>0 |                     |>0|
    XXXXXXXXXXXX/       |   <135 ns  |     |  \XXXXXXXXXXXXX R/W*
            |         <200 ns        |     |<50|
    ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZX  data    XZZZZZZZZZZZZ DOUT

    a) > -10 ns

#### Write cycle

           |                    >375 ns                       |
           \              200-1000 ns               / >120 ns \     RAS*
            |                >200 ns              | |         |
            | 25-65 ns  |         >135 ns           |         |
            |          \|     >135 ns            /|  > -20 ns |  \_ CAS*
            |           |                         | |
            |      >120 ns      |                 | |
      | >0  | >25 |   |a| >55   |                 | |
    XX     row    XXXX  column  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX A0-A6
            |      |                 >85 ns       | |
            |      |              >70 ns            |
            |      |<-20| >55 |
            |      \  >55 ns  /                                \   R/W*
            |     >120 ns     |
                      |b| >55 ns |
    XXXXXXXXXXXXXXXXXX   data    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX DIN
            |       >55 ns       |
    ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ DOUT

a\) \> -10 ns
b) \> -20 ns (data setup time before CAS\*)

#### Refresh cycle

           |            >375 ns                              |
           \    200-1000 ns    /        120 ns - 2 ms        \     RAS*
            |
      | >0  | >25 |
    XX     row    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX A0-A6

    ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ DOUT

##  Electrical characteristics

These are the characteristics of the Mitsubishi equivalent: M5K4116P.

### Absolute maximum ratings

    Supply voltage: Vcc, Vdd, Gnd  -0.5 to +20 V over Vbb
                    Vcc, Vdd         -1 to +15 V over Gnd
                    Vbb                  max 0 V over Gnd
    Input voltage:                  -0.5 to 20 V over Vbb
    Output voltage:                 -0.5 to 20 V over Vbb
    Output current:                         50 mA
    Power dissipation:                700-1000 mW
    Free air temperature:             0 to 70 `C
    Storage temperature:           -40 to 125 `C

###   Recommended operating conditions

|                                               |      |     |      |      |
|-----------------------------------------------|------|-----|------|------|
| Parameter                                     | Min  | Nom | Max  | Unit |
| Supply voltage Vdd                            | 10.8 | 12  | 13.2 | V    |
| Supply voltage Vcc                            | 4.5  | 5   | 5.5  | V    |
| Supply voltage Gnd                            | 0    | 0   | 0    | V    |
| Supply voltage Vbb                            | -4.5 | -5  | -5.7 | V    |
| High level input voltage: RAS\*, CAS\*, R/W\* | 2.7  | .   | 7    | V    |
| High level input voltage: A0-A6, Din          | 2.4  | .   | 7    | V    |
| Low level voltage                             | -1   | .   | 0.8  | V    |

###  Characteristics under recommended conditions

Ta = 0-70 \`C, Vdd = 12V+/- 10%, Vcc = 5V +/- 10%, Gnd = 0V, Vbb = -5.7
to -4.5 V



------------------------------------------------------------------------
Do-it-yourself card

If you wanted to emulate such a card, here is a suggested circuit (not
tested yet).

           +----+                              
     A0&gt;---|    |                                       
      .    |    |                        74LS138        
      .    |    |                      +--------+                 
      .    |    |                A0----|S2      |                 
      .    |    |                A1----|S1   Y1*|---,               
      .    |    |                A2----|S0      |   |   74LS21      
     A7&gt;---|    |                      |        |   |    ___      
      Gnd--|EN* |                +5V---|G1      |   &#39;---|   \      
           +----+  ,-------------------|G2A* Y5*|-------|    )_ __  
            &#39;244   |  +5V---WWW--+-----|G2B* Y6*|-------|    )    | 
           +----+  |        10K  |     |     Y7*|-------|___/     |     
     A8&gt;---|    |  |    Gnd--o o-&#39;     +--------+                 | 
      .    |    |  |                                              | 
      .    |    |  |                                              | 
      .    |    |  |        ___                                   | 
      .    |    |  |  A0---|   \           55256 (32K)            |
      .    |    |  |  A2---|    )___     +-------------+          | 
      .    |    |  | +5V---|    )   &#39;----|A14          |          |
     A15&gt;--|    |  | +5V---|___/  A1-----|A13       CE*|----------+
      Gnd--|EN* |  |               .     | .           |          | 
           +----+  |               .     | .           |          |
            &#39;244   |               .     | .         D7|------D0  |
           +----+  |               .     |           . |      .   | 
           |    |  |             A15-----|A0         . |      .   | 
 MEMEN*&gt;---|    |--&#39;                     |           . |      .   |
    WE*&gt;---|    |------------------------|R/W*       . |      .   | 
CRUCLK*&gt;---|    |                  Gnd---|OE*        D0|------D7  | 
CLKOUT*&gt;---|    |                        +-------------+          |   
 RESET*&gt;---|    |                                                 | 
   DBIN&gt;---|    |---,                                             |
      Gnd--|EN* |   |        +5V                                  |     
           +----+   |          |                                  |   
            &#39;245    |     LED  V                                  |  
           +----+   |          |      74LS125                     | 
           | DIR|---&#39;          &#39;--------&lt;|------------------------+     
    D0&lt;---&gt;|    |                       |                         |    
     .     |    |                      Gnd                        |  
     .     |    |                                                 |       
     .     |    |                                                 |  
     .     |    |                                                 |  
     .     |    |                                                 |   
     .     |    |                                                 |   
    D7&lt;---&gt;|    |                                                 |
           | OE*|---------+---------------------------------------&#39;
           +----+         |                                      
                          |                                   
RDBENA*&lt;------------------&lt;|----Gnd
                       74LS125  
                                  78L05                          All chips
                               +----------+      +5V             +-----+
+8V--------------------+-------|Vin   Vout|---+-----... --+------|Vcc  |  
                       |       |   Gnd    |   |           |      |     |
                       = 22 uF +----------+   = 0.1 uF    =0.1uF |     |
                       |            |         |           |      |     |
                      Gnd           Gnd       Gnd         +------|Vss  |
                                                                 +-----+

####  Memory

As recommended by Texas Instruments, the PE-box bus incoming lines are
buffered on the card by 74LS244 buffers.

The memory chip is a 32Kx8 SRAM chip. Using SRAM allows us to avoid the
annoying requirement for a refreshing circuit. In addition, we could
install a small battery on the card to keep the content of the memory
intact even when the power is down... And nowadays 32K of SRAM costs
nothing.

The R/W\* pin is driven by the WR\* line that provides a write pulse
during write operations and remains high during read operations.

The data pins D0 through D7 are connected to the data bus via a 74LS245
8-bit bus transceiver, which is enabled by the card selection signal and
whose direction is controlled by DBIN. (NB Since Texas Instruments
numbers the pins the opposite way to everyone else, D0 corresponds to D7
and conversely. This is not strictly required in this case, though).

The 15 address pins A0-A13 are connected to the address bus lines A1 to
A15. Line A14 receives a A0 \[AND\] A2 signal, so that the various
extended memory areas map at different addresses in the chip (Thanks to
Dave Nieters). The AND gate is implemented with the second half of the
74LS21 chip used for address decoding: the two unused inputs should be
connected to +5 volts. This way, we have the following mapping:

    Address  A0 A1 A2 | A14 A13 SRAM
    >2000    0  0  1  |  0   0  >0000
    >A000    1  0  1  |  1   0  >4000
    >C000    1  1  0  |  0   1  >2000
    >E000    1  1  1  |  1   1  >6000

####  Selection logic

The address mapping is performed by a 74LS138 decoder hooked to address
lines A0-A2 and enabled by MEMEN\*. Each output of the decoder
corresponds to a \>2000 bytes block in memory. The ones we want to react
to are Y1\* (\>2000-3FFF), Y5\* (\>A000-BFFF), Y6\* (\>C000-DFFF) and
Y7\* (\>E000-FFFF). They are combined through a 4-inputs AND gate to
produce an active low selection signal. This signal selects the SRAM and
enables our data bus transceiver (74LS245). It also enables the data bus
buffers at each end of the connection cable by bringing RDBENA\* low via
a 74LS125 3-state buffer. The '125 is required because we don't want to
hold this line high when the card in not in use, since other cards may
need to bring it low.

A LED indicator lights up when the card is selected. Optionally, a
transistor could be used to make it shine brighter. Mount the LED so
that it is visible in the slot on the front of the PE-box.

An optional switch has been added to the 74LS138, so that the card can
be disabled if necessary (a good thing to have during the test phase).
The memory answers when the switch is closed, and is disabled when the
switch is open.

####  Power supply

The card is powered by a standard 78L05 voltage regulator that delivers
100 mAmps at +5 Volts. It has a 22 uF capacitor on the power line, and a
0.1 uF cap on the output. Note that each chip should also have its own
by-pass capacitor (0.1 uF is a good value) connected as close to the Vcc
pin as possible (ideally, the current should meet the cap before
reaching the chip).

####  List of components

    1  Hitachi 55256 32x8 SRAM
    1  74LS238 decoder
    1  74LS21 dual 4-input AND gate
    3  74LS244 dual 4-bit buffers
    1  74LS245 8-bit transceiver, 3-state
    1  74LS125 quad buffer, 3-state
    1  78L05 voltage regulator
    1  Yellow LED
    1  SPST switch (optional)
    1  Capacitor 22 microfarads
    9  Capacitors 0.1 microfarad
    1  Resistors 10 kilohms

The most expensive part will be the board itself. See my [PE-Box
page](pebox.md) for the connections with the PE-box bus.
Preliminary version 6/16/99. Not for release.
Revision 1. 6/17/99. Ok to release (circuit not tested).
Revision 2. 5/3/00. Added timing diagrams and electrical
characteristics.
Revision 3. 11/1/01. Changed the circuit controlling A14 (AND gate
instead of XOR).

[Back to the TI-99/4A Tech Pages](titechpages.md)
