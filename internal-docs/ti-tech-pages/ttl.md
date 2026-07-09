# TTL chips

[74LS00](ttl.htm#7400) Quad 2-input NAND gates
[74LS04](#7404) Hex inveter
[74LS05](#7405) Hex inverters, open collector
[74LS08](#7408) Quad 2-input AND gates
[74LS21](#7421) Dual 4-input AND gates
[74LS32](#7432) Quad 2-input OR gates
[74LS74](#7474) Dual D flip-flop
[74LS85](#7485) 4-bit comparator
[74LS121](#74121) One-shot
[74LS123](#74123) Dual retriggerable one-shot
[74LS125](#74125) Quad buffers, 3-state
[74LS133](#74133) 13-inputs NAND gate
[74LS138](#74138) 1-of-8 decoder
[74LS139](#74139) Dual 1-of-4 decoder
[74LS154](#74154) 1-of-16 decoder
[74LS156](#74156) 2-of-8 decoder, open collector
[74LS175](#74175) 4-bit D flip-flop with complementary outputs and
reset.
[74LS194](#74194) 4-bit universal shift register
[74LS244](#74244) Dual 4-bit buffers, 3-state
[74LS245](#74245) 8-bit bus transceiver, 3-state
[74LS251](#74251) 8-to-1 multiplexer, 3-state
[74LS259](#74259) 1-of-8 addressable latch
[74LS273](#74273) 8-bit D flip-flop, 3-state
[74LS373](#74373) 8-bit transparent latch, 3-state
[74F543](#74543) 8-bit transceiver with latches, 3-states.
[74LS688](#74688) 8-bit identity comparator

**Non TTL chips  **
[78T05](#78T05) Voltage regulator +5V, 1 Amp
[78L05](#78L05) Voltage regulator +5V, 100 mAmp
[78L12](#78L05) Voltage regulator +12V, 100 mAmp
[79L05](#79L05) Voltage regulator -5V, 100 mAmp

For information on the pinouts of TTL, CMOS and many other chips, see
"The Giant Internet IC Masturbator" at
 or "The Chip Directory"
at 

##  74LS00

    Quad 2-input NAND gates.

        +---+--+---+             +---+---*---+           __
     1A |1  +--+ 14| VCC         | A | B |/Y |      /Y = AB
     1B |2       13| 4B          +===+===*===+
    /1Y |3       12| 4A          | 0 | 0 | 1 |
     2A |4  7400 11| /4Y         | 0 | 1 | 1 |
     2B |5       10| 3B          | 1 | 0 | 1 |
    /2Y |6        9| 3A          | 1 | 1 | 0 |
    GND |7        8| /3Y         +---+---*---+
        +----------+

## 74LS04

Hex inverters.

        +---+--+---+          +---*---+ _
     A1 |1  +--+ 14| Vcc      | A |Y* | Y* = A
    Y1* |2       13| A6       +===*===+
     A2 |3       12| Y6*      | 0 | Z |
    Y2* |4   74  11| A5       | 1 | 0 |
     A3 |5   04  10| Y5*      +---*---+
    Y3* |6        9| A4
    GND |7        8| Y4*
        +----------+

When a A input pin is high, the corresponding Y output pin is low. When
A is low, the corresponding Y is high impedence.

##  74LS05

Hex open-collector inverters.

        +---+--+---+          +---*---+ _
     A1 |1  +--+ 14| Vcc      | A |Y* | Y* = A
    Y1* |2       13| A6       +===*===+
     A2 |3       12| Y6*      | 0 | Z |
    Y2* |4   74  11| A5       | 1 | 0 |
     A3 |5   05  10| Y5*      +---*---+
    Y3* |6        9| A4
    GND |7        8| Y4*
        +----------+

When a A input pin is high, the corresponding Y output pin is connected
to GND (ground). When A is low, the corresponding Y is not connected
(high impedence, i.e neither low nor high).

##  74LS08

Quad 2-input AND gates.

        +---+--+---+                +---+---*---+
     A1 |1  +--+ 14| VCC            | A | B | Y |   Y = A and B
     B1 |2       13| B4             +===+===*===+
     Y1 |3       12| A4             | 0 | 0 | 0 |
     A2 |4  74   11| Y4             | 0 | 1 | 0 |
     B2 |5  08   10| B3             | 1 | 0 | 0 |
     Y2 |6        9| A3             | 1 | 1 | 1 |
    GND |7        8| Y3             +---+---*---+
        +----------+

An output Yn is high if (and only if) both its inputs An and Bn are
high.

##  74LS21

Dual 4-input AND gates.

        +---+--+---+                   +---+---+---+---*---+
     A1 |1  +--+ 14| VCC               | A | B | C | D | Y |  Y = A and B and C and D
     B1 |2       13| D2                +===+===+===+===*===+
        |3       12| C2                | 0 | X | X | X | 0 |
     C1 |4   74  11|                   | 1 | 0 | X | X | 0 |
     D1 |5   21  10| B2                | 1 | 1 | 0 | X | 0 |
     Y1 |6        9| A2                | 1 | 1 | 1 | 0 | 0 |
    GND |7        8| Y2                | 1 | 1 | 1 | 1 | 1 |
        +----------+                   +---+---+---+---*---+

An output Yn is only high when all its inputs An, Bn, Cn and Dn are
high.

##  74LS32

Quad 2-input OR gates.

        +---+--+---+                    +---+---*---+
     A1 |1  +--+ 14| VCC                | A | B | Y | Y = A or B
     B1 |2       13| B4                 +===+===*===+
     Y1 |3       12| A4                 | 0 | 0 | 0 |
     A2 |4   74  11| Y4                 | 0 | 1 | 1 |
     B2 |5   32  10| B3                 | 1 | 0 | 1 |
     Y2 |6        9| A3                 | 1 | 1 | 1 |
    GND |7        8| Y3                 +---+---*---+
        +----------+

An output Yn is high if at least one of its inputs An or Bn is high.

##   74LS74

Dual D flip-flop with set and reset.

          +---+--+---+           +---+---+----+----*---+---+
    1CLR* |1  +--+ 14| Vcc       | D |CLK|PR* |CLR*| Q |Q* |
       1D |2       13| 2CLR*     +===+===+====+====*===+===+
     1CLK |3       12| 2D        | X | X |  0 |  0 | 1 | 1 |
     1PR* |4  7474 11| 2CLK      | X | X |  0 |  1 | 1 | 0 |
       1Q |5       10| 2PR*      | X | X |  1 |  0 | 0 | 1 |
      1Q* |6        9| 2Q        | 0 | / |  1 |  1 | 0 | 1 |
      GND |7        8| 2Q*       | 1 | / |  1 |  1 | 1 | 1 |
          +----------+           | X |!/ |  1 |  1 | - | - |
                                 +---+---+----+----*---+---+

When a rising edge occurs on the CLK input, the content of the D input
is latched onto the Q output. The inverted state is presented on Q\*.
The situation remains unchanged until the next rising edge on CLK.

But this only occurs if both PR\* and CLR\* are high. If PR\* is low, Q
is high and Q\* is low. If CLR\* is low, Q is low and Q\* is high. If
both PR\* and CLR\* are low, Q and Q\* are high.

##  74LS85

4-bit noninverting magnitude comparator with cascade inputs.

         +---+--+---+
      B3 |1  +--+ 16| VCC
    IA<B |2       15| A3
    IA=B |3       14| B2
    IA>B |4       13| A2
    OA>B |5   74  12| A1
    OA=B |6   85  11| B1
    OA<B |7       10| A0
     GND |8        9| B0
         +----------+

The 4-bit value on A1-A4 is compared to that on B1-B4. The output OA=B
is high if they are equal. The output OA\<B is high if A1-A4 is lower
than B1-B4. The output OA\>B is high if A1-A4 is higher than B1-B4.

The result is affected by the cascading inputs IA=B, IA\B
that are used to connect another comparator dealing with more
significant bits. If IA\B is
high, the result will be A\>B. It's only when IA=B is high that the
values of A1-A4 and B1-B4 become relevant.

##  74LS121

Monostable multivibrator with Schmitt-trigger inputs.
Programmable output pulse width from 40 ns to 20 seconds.

         +---+--+---+
      Q* |1  +--+ 14| VCC
         |2       13|
      A* |3   74  12|
      B* |4  121  11| RC
       C |5       10| C
       Q |6        9| Rint
     GND |7        8|
         +----------+

Initially the Q output is low and Q\* is high. For a pulse to occur A\*
and B\* must be low and C must be high. When this condition is met, a
square pulse is triggered on the ouputs (i.e. Q becomes temporarily
high, Q\* temporarily low). For another pulse to occur one of the inputs
should change and then come back to the triggering condition.

The length of the pulse is determined by external components: a resistor
connecting RC and Vcc, and a capacitor connecting C and RC. I'm not sure
what Rint is used for, it may be an alternative to the external resistor
used by connecting it to RC?

##  74123

Dual retriggerable monostable multivibrators with overriding reset.

           +---+--+---+
       1A* |1  +--+ 16| Vcc
        1B |2       15| 1RC
     1Clr* |3       14| 1C
       1Q* |4   74  13| 1Q
        2Q |5  123  12| 2Q*
        2C |6       11| 2Clr*
       2RC |7       10| 2B
       GND |8        9| 2A*
           +----------+

Initially the Q outputs are low and the Q\* are high. When A\* gets low
and/or B gets high, a square pulse is triggered on the ouputs (i.e. Q
becomes temporarily high, Q\* temporarily low). The length of the pulse
is determined by external components: a resistor connecting RC and Vcc,
and a capacitor connecting C and RC.

The pulse length can be increase indefinitely if falling edges keep
occuring on A\* during the pulse (or rising edges on B). The pulse can
be interrupted (or prevented) by a low level on CLR\*.

##  74LS125

Quad 3-state noninverting buffer with active low enables.

         +---+--+---+                +---+---*---+
    OE1* |1  +--+ 14| VCC            | A |OE*| Y |
      A1 |2       13| OE4*           +===+===*===+
      Y1 |3   74  12| A4             | 0 | 0 | 0 |
    OE2* |4  125  11| Y4             | 1 | 0 | 1 |
      A2 |5       10| OE3*           | X | 1 | Z |
      Y2 |6        9| A3             +---+---*---+
     GND |7        8| Y3
         +----------+

When an OEn\* (output enable) input is low, the content of its
associated input An is copied onto the output Yn.
When OEn\* is high, the corresponding Yn is inhigh impedace state (i.e.
isolated, neither high nor low).

##  74LS133

13-input NAND gate.

        +---+--+---+                 _____________
      A |1  +--+ 16| VCC        Y* = ABCDEFGHIJKLM
      B |2       15| M
      C |3       14| L
      D |4   74  13| K
      E |5  133  12| J
      F |6       11| I
      G |7       10| H
    GND |8        9| Y*
        +----------+

The output Y\* is low if all inputs A through M are high. Otherwise, Y\*
is high.

##  74LS138

1-of-8 inverting decoder/demultiplexer.

         +---+--+---+                +---+----+----+---+---+---*---+---+---+---+
      S0 |1  +--+ 16| Vcc            | G1|G2A*|G2B*| S2|S1 | S0|Y0*|Y1*|...|Y7*|
      S1 |2       15| Y0*            +===+====+====+===+===+===*===+===+===+===+
      S2 |3       14| Y1*            | 0 | X  | X  | X | X | X | 1 | 1 | 1 | 1 |
    G2A* |4  74   13| Y2*            | 1 | 1  | X  | X | X | X | 1 | 1 | 1 | 1 |
    G2B* |5  138  12| Y3*            | 1 | 0  | 1  | X | X | X | 1 | 1 | 1 | 1 |
      G1 |6       11| Y4*            | 1 | 0  | 0  | 0 | 0 | 0 | 0 | 1 | 1 | 1 |
     Y7* |7       10| Y5*            | 1 | 0  | 0  | 0 | 0 | 1 | 1 | 0 | 1 | 1 |
     GND |8        9| Y6*            | 1 | 0  | 0  | . | . | . | 1 | 1 | . | 1 |
         +----------+                | 1 | 0  | 0  | 1 | 1 | 1 | 1 | 1 | 1 | 0 |
                                     +---+----+----+---+---+---*---+---+---+---+

G1, G2A and G2B are enabling inputs: for one Y output to be active (i.e.
low) G1 must be high, and both G2A and G2B must be low. Which output is
then low depends on the value of the 3 selection inputs S0, S1 and S2.

##  74LS139

Dual 1-of-4 inverting decoder/demultiplexer.

         +---+--+---+            +---+---+---*---+---+---+---+
     1G* |1  +--+ 16| VCC        | G*| S1| S0|Y0*|Y1*|Y2*|Y3*|
     1S0 |2       15| 2G*        +===+===+===*===+===+===+===+
     1S1 |3       14| 2S0        | 1 | X | X | 1 | 1 | 1 | 1 |
    1Y0* |4   74  13| 2S1        | 0 | 0 | 0 | 0 | 1 | 1 | 1 |
    1Y1* |5  139  12| 2Y0*       | 0 | 0 | 1 | 1 | 0 | 1 | 1 |
    1Y2* |6       11| 2Y1*       | 0 | 1 | 0 | 1 | 1 | 0 | 1 |
    1Y3* |7       10| 2Y2*       | 0 | 1 | 1 | 1 | 1 | 1 | 0 |
     GND |8        9| 2Y3*       +---+---+---*---+---+---+---+
         +----------+

The chip contains two independent 2-to-4 decoders. 1G\* enables the
first one when low. This makes one of the 1Y outputs go low, which one
it is depends on 1S0 and 1S1. 2\*, 2S0 and 2S2 play the same role for
the 2Y outputs.

##  74LS154

1-of-16 inverting decoder/demultiplexer.

         +---+--+---+
     Y0* |1  +--+ 24| VCC
     Y1* |2       23| S0
     Y2* |3       22| S1
     Y3* |4       21| S2
     Y4* |5       20| S3
     Y5* |6   74  19| EN2*
     Y6* |7  154  18| EN1*
     Y7* |8       17| Y15*
     Y8* |9       16| Y14*
     Y9* |10      15| Y13*
    Y10* |11      14| Y12*
     GND |12      13| Y11*
         +----------+

When either EN1\* or EN2\* (or both) is high, all outputs Y0\*-Y15\* are
high.
When both EN1\* and EN2\* are low, one of the outputs is low. The inputs
S0-S4 determine which one: 1000 is Y0\*, 0100 is Y1\*... 1111 is Y7\*.

##  74LS156

2-of-8 open-collector inverting decoder/demultiplexer with separate
enables.

          +---+--+---+
     1GA* |1  +--+ 16| VCC
     1GB* |2       15| 2GA*
       S1 |3       14| 2GB*
     1Y3* |4   74  13| S0
     1Y2* |5  156  12| 2Y3*
     1Y1* |6       11| 2Y2*
     1Y0* |7       10| 2Y1*
      GND |8        9| 2Y0*
          +----------+

When both 1GA\* and 1GB\* are low, one of the outputs 1Y0\* to 1Y3\* is
low. S0 and S1 determine which it is: 00 = 1Y0\*, 10=1Y1\*, 01=1Y2\*,
and 11=1Y3\*. The other outputs are high. Similarly, 2GA\* and 2GB\*
enable one of the outputs 2Y0\* to 2Y3\*, when both of them are low.
Otherwise, all outputs are high.

##  74175

4-bit D flip-flop with complementary outputs and reset.

         +---+--+---+            +----+---+---*---+---+
    RST* |1  +--+ 16| VCC        |RST*|CLK| D | Q |Q* |
      Q1 |2       15| Q4         +====+===+===*===+===+
     Q1* |3       14| Q4*        |  0 | X | X | 0 | 1 |
      D1 |4   74  13| D4         |  1 | / | 0 | 0 | 1 |
      D2 |5  175  12| D3         |  1 | / | 1 | 1 | 0 |
     *Q2 |6       11| Q3*        |  1 |!/ | X | - | - |
      Q2 |7       10| Q3         +----+---+---*---+---+
     GND |8        9| CLK
         +----------+

The device latches the values of D1 through D4 when the CLK pin goes
from low to high. The data the remains stable on Q1 through Q4, and its
inverse on Q1\* through Q4\*. Bringing the RST\* pin low will reset all
Qn outputs to '0' and all Qn\* outputs to '1'.

##  74LS194

4-bit bidirectional universal shift register with asynchronous reset.

         +---+--+---+            +---+---*---------------+
    RST* |1  +--+ 16| VCC        | S1| S0| Function      |
      SR |2       15| QA         +===+===*===============+
       A |3       14| QB         | 0 | 0 | Hold          |
       B |4   74  13| QC         | 0 | 1 | Shift right   |
       C |5  194  12| QD         | 1 | 0 | Shift left    |
       D |6       11| Ck         | 1 | 1 | Parallel load |
      SL |7       10| S1         +---+---*---------------+
     GND |8        9| S0
         +----------+

When RST\* goes low, all Qn outputs go low immediately and stay so as
long as Rst\* is low. Other operations only happen the moment Ck goes
from low to high. The operation depends on the status of S0 and S1:

If both are low, nothing happens.
If both are high, the contents of the inputs A through D are copied to
the outputs QA through QD.
If S1 is low and S0 is high, the register shifts to the right: QD=QC,
QC=QB, QB=QA, QA=SrD.
If S1 is high and S0 is low, the register shifts to the left: QA=QB,
QB=QC, QC=QD, QD=SrL.

##  74LS244

Dual 4-bit 3-state noninverting buffer/line driver.

         +---+--+---+
    OE1* |1  +--+ 20| Vcc
      A0 |2       19| OE2*
      Y7 |3       18| Y0
      A1 |4       17| A7
      Y6 |5   74  16| Y1
      A2 |6  244  15| A6
      Y5 |7       14| Y2
      A3 |8       13| A5
      Y4 |9       12| Y3
     GND |10      11| A4
         +----------+

The values of the inputs A0 through A7 are copied to the outputs Y0
through Y7 respectively. When OE1\* is low, Y0 through Y3 are in "high
impedence" state, i.e. isolated, neither low nor high. When OE2\* is
low, Y4 through Y7 are in high impedence.

##  74LS245

8-bit 3-state noninverting bus transceiver.

        +---+--+---+           +---+---*---+---+
    DIR |1  +--+ 20| Vcc       |EN*|DIR| A | B |
     A1 |2       19| EN*       +===+===*===+===+
     A2 |3       18| B1        | 1 | X | Z | Z |
     A3 |4       17| B2        | 0 | 0 | B | Z |
     A4 |5   74  16| B3        | 0 | 1 | Z | A |
     A5 |6  245  15| B4        +---+---*---+---+
     A6 |7       14| B5
     A7 |8       13| B6        Z = high impedence
     A8 |9       12| B7
    GND |10      11| B8
        +----------+

When DIR is high, the A pins are inputs and their values are copied on
the corresponding B pins. When DIR is low, it's the opposite: the A pins
are outputs and their value depends on that of the corresponding input B
pin. When EN\* (enable) is low, all A and B pins are in "high impedence"
state, i.e. isolated.

##  74LS251

8-to-1 line 3-state data selector/multiplexer with complementary
outputs.

        +---+--+---+
     A3 |1  +--+ 16| VCC
     A2 |2       15| A4
     A1 |3       14| A5
     A0 |4   74  13| A6
      Y |5  251  12| A7
     Y* |6       11| S0
    EN* |7       10| S1
    GND |8        9| S2
        +----------+

When EN\* is low, the content of one of the inputs A0-A7 is copied onto
Y, and its inverted value on Y\*. The input selection is achieved by
S0-S2: 000 is A0, 100 is A1... 111 is A7.
When EN\* is high, all the outputs are in high impedance state (i.e.
isolated).

##  74LS259

1-of-8 addressable latch with reset.

        +---+--+---+            +---+----*--------------------+
     S0 |1  +--+ 16| VCC        |EN*|RST*| Function           |
     S1 |2       15| RST*       +===+====*====================+
     S2 |3       14| EN*        | 0 | 0  | 1-of-8 demultiplex |
     Q0 |4    74 13| D          | 0 | 1  | addressable latch  |
     Q1 |5   259 12| Q7         | 1 | 0  | reset              |
     Q2 |6       11| Q6         | 1 | 1  | hold               |
     Q3 |7       10| Q5         +---+----*--------------------+
    GND |8        9| Q4
        +----------+

When EN\* is low, the value of the input D is copied onto one of the
outputs Q0-Q7. The output selection is achieved by the inputs S0-S2: 000
is Q0, 100 is Q1... 111 is Q7. When EN\* is high, the status of Q0-Q7
remains unchanged not matter what happens to D.

Bringing RST\* low resets all output to 0.

##  74LS273

8-bit 3-state D flip-flop with reset.

         +---+--+---+              +----+---+---*---+
    RST* |1  +--+ 20| Vcc          |RST*|CLK| D | Q |
      Q0 |2       19| Q4           +====+===+===*===+
      D0 |3       18| D4           | 0  | X | X | 0 |
      D1 |4       17| D5           | 1  | / | 0 | 0 |
      Q1 |5   74  16| Q5           | 1  | / | 1 | 1 |
      Q2 |6  273  15| Q6           +----+---+---*---+
      D2 |7       14| D6
      D3 |8       13| D7
      Q3 |9       12| Q7
     GND |10      11| CLK
         +----------+

When CLK (clock) goes from low to high, the values present on D0 through
D7 are latched and output on Q0 through Q7 respectively. The Q outputs
will remain unchanged until the next time CLK goes from low to high, no
matter what happens to the D inputs. The active low RST\* pin resets the
chip and forces all outputs to low.

##  74LS373

8-bit 3-state transparent latch.

        +---+--+---+                    +---+---+---*---+
    OE* |1  +--+ 20| VCC                |OE*| LE| D | Q |
     Q1 |2       19| Q8                 +===+===+===*===+
     D1 |3       18| D8                 | 1 | X | X | Z |
     D2 |4       17| D7                 | 0 | 0 | X | - |
     Q2 |5   74  16| Q7                 | 0 | 1 | 0 | 0 |
     Q3 |6  373  15| Q6                 | 0 | 1 | 1 | 1 |
     D3 |7       14| D6                 +---+---+---*---+
     D4 |8       13| D5
     Q4 |9       12| Q5
    GND |10      11| LE
        +----------+

When LE (latch enable) is high, the values present on D0 through D7
appear on the outputs Q0 through Q7 respectively. When LE is loe, the Q
outputs remain unchanged no matter what happens to the D inputs.

If OE\* (output enable) is high all the outputs Q0-Q7 are in high
impedance state (i.e. isolated).

##  74F543

8-bit 3-state noninverting registered transceiver.

          +---+--+---+
    LEBA* |1  +--+ 24| VCC
    OEBA* |2       23| CEBA*
       A0 |3       22| B0
       A1 |4       21| B1
       A2 |5       20| B2
       A3 |6   74  19| B3
       A4 |7  543  18| B4
       A5 |8       17| B5
       A6 |9       16| B6
       A7 |10      15| B7
    CEAB* |11      14| LEAB*
      GND |12      13| OEAB*
          +----------+

This transceiver has the ability to transfer a byte from A0-A7 to B0-B7,
or the opposite. In each direction, the input byte can be latched for
later use.

When low, CEAB\* enables the chip in the A to B direction. Under these
conditions, OEAB\* enables data output from the B port. If LEAB\* is
low, this data is the byte present on the A port. If LEAB\* is high the
B port outputs the data that was present on A the last time LEAB\* was
low (i.e. it remembers it). When either CEAB\* or OEAB\* is high (or
both), the B outputs are in high-impedance state.

CEBA\*, OEBA\* and LEBA\* play the same role in the B to A direction.

##  74LS688

8-bit inverting identity comparator with enable.

        +---+--+---+
     G* |1  +--+ 20| VCC
     A0 |2       19| A=B*
     B0 |3       18| B7
     A1 |4       17| A7
     B1 |5   74  16| B6
     A2 |6  688  15| A6
     B2 |7       14| B5
     A3 |8       13| A5
     B3 |9       12| B4
    GND |10      11| A4
        +----------+

When G\* is low, the chip is enable and the sole output A=B\* will be
low if each Ax input is identical to the corresponding Bx input. In
other words, the chip tells you if the 8-bit number placed on A0-A7 is
the same as the one placed on B0-B7.

##  78T05

5 volts, 1 Amp positive power supply regulator Vin must be between 8 and
40 volts.

TO 220 package

    +---+ <--Metal tab
    +===+ Branded face to you
    | ##|
    +++++ 1: Vin
     |||  2: GND
     |||  3: Vout
    1 2 3

##   78L05, 78L08, 78L12

Fixed voltage 100mA positive power supply regulator. Vin must exceed
Vout by at least 3V, but may not exceed 40V.

TO 92 package

    +---+ Flat face to you
    | ##|
    +++++ 1: Vout
     |||  2: GND
     |||  3: Vin
    1 2 3

##  79L05, 79L08, 79L12

Fixed voltage 100mA negative power supply regulator. Vin must exceed
Vout by at least -3V, but may not exceed -40V.

TO 92 package

    +---+ Flat face to you
    | ##|
    +++++ 1: GND
     |||  2: Vin
     |||  3: Vout
    1 2 3

[Back to the TI-99/4A Tech Pages](titechpages.md)
