# The MBP card

The MBP card comprises a real-time clock and an 8-bit analog-to-digital
converter (ADC). The clock gives you the time and date, the ADC let you
measure the voltage (or current) of upto eight different signals. The
card is based on an original design by Gary Emmich.

Let me point out that I don't own such a card, so the present
description is based on the schematics and manual, rather than on
"hands-on" experience.

**Hardware  **
[Card circuitery
](#Card%20circuitery)[The clock chip
](#RTC58167)[The ADC chip](#ADC0809)

**Software  **
[Accessing the clock
](#Acessing%20clock)[Accessing the ADC](#Accessing%20ADC)

##  Hardware

[Power supply
](#Power)[Bus buffering
](#Bus%20buffers)[Front light
](#LEDs)[Address decoding
](#Address%20decoding)[Real-time clock
](#Clock)[AD converter](#ADC)

####  Power supply

There are two independent +5V voltage regulators, one for the clock and
one for the rest of the card. That's probably because one of them
provides current to the external devices that you want to hook to the AD
converter. In addition, the clock is backed up by a +3V lithium battery,
so that it keeps time even when power is off.

                    7805                          All chips
                +----------+      +5V                +-----+
+8V---+-----+---|Vin   Vout|---+----------...----+---|Vcc  |    
      |     |   |   Ref    |   |                 |   |     |
      | 100 =   +----------+   = 22          0.1 =   |     |
      | uF  |        |         | uF          uF  |   |     |
      |    Gnd      Gnd        Gnd        Gnd----+---|Vss  |
      |                                              +-----+
      |             7805                          Clock
      |         +----------+      1N914          +-----+
      &#39;-----+---|Vin   Vout|---+---|&gt;|---+-------|Vcc  |    
            |   |   Ref    |   |         |       |     |
        100 =   +----------+   = 22      |       |     |
        uF  |        |         | uF      |       |     |
           Gnd 1N914 V         Gnd       |       |Vss  |
                     |                   |       +-----+
                    Gnd                  |
                          |              |   1MOhm
                    Gnd---||-----|&gt;|-----+---WWW---Gnd
                          |   1N914
                        +3V battery


Regular power is supplied by a +5 volts regulator 7805. It has a 100 uF
cap on its input and a 22 uF cap on its output.

The clock power is supplied by an identical +5 volts regulator 7805,
through a 1N914 diode to prevent the battery current from flowing to the
rest of the card (and the PE-box) when power is off. This diode will
cause a small voltage drop, so the regulator has a similar diode in its
reference line to compensate for the drop.

A lithium battery (BR2325 or equivalent) provides a +3V backup voltage
to the clock. Another 1N914 diode prevent the regular +5 volts power
supply from fighting the battery when power is on.

Each chip on the card has its own 0.1 uF bypass capacitor, between its
power pin and the ground. This helps filtering power spikes.

####  Bus buffering and enabling

As specified by Texas Instruments, all PE-box signals are re-buffered on
card. There are two 74LS244 to buffer the address bus, one more to
buffer some control lines (MEMEN\*, WE\*, PHI3\*, and DBIN), and a
74LS245 to buffer the data bus. The direction of the 74LS245 is
controlled by DBIN, and it is enabled by the Sel\* selection signal (see
[below](#Address%20decoding)), together with the RDBENA\* line that
activates the remote data bus buffers in the connection card and cable.

Note that DBIN is double-buffered by a 74LS08, then inverted by a
74LS368 to provide a delayed, active-low read signal.

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
  PHI3*&gt;---|       |---Phi3*  
    WE*&gt;---|       |---WE*          
  MEMEN&gt;---|       |---Memen*               
           |       |            
           |       | +5V-,      74LS368      
   DBIN&gt;---|       |-----=|)-+---|&gt;o---Dbin*   
      Gnd--|EN*    |  74LS08 |    | 
           +-------+         |   Gnd 
            74LS245          |      
           +-------+         |      
           |    DIR|&lt;--------&#39;      
    D0&lt;---&gt;|       |-----D0       
     .     |       |      .     
     .     |       |      .    
     .     |       |      .   
     .     |       |      .              
     .     |       |      .      
     .     |       |      . 
    D7&lt;---&gt;|       |-----D7             
           |    OE*|&lt;--------+---------Sel*  
           +-------+         |               
                             |     1K           
RDBENA*&lt;--------------------o&lt;|---WWW---+5V
                         74LS367

####  Front light

The card contains a dedicated chip, a 555 timer, whose sole function is
to light up the front panel LED when the card is accessed. This is kind
of a luxury and could easily be ommited, or replaced with a simple
transistor.

       |    Output|-------|&gt;|---WWW---Gnd
Sel*---|Trigger   |
       | Threshold|--+---WWW---+5V
       |          |  |   1K
       | Discharge|--+---||----Gnd
       |          |     22 uF
       |  CtrlVolt|---||---Gnd
       +----------+  100 uF 

The 555 is wired as a one-shot monostable multivibrator. When the Sel\*
line is triggered, it sends an ~25 usec pulse to the LED.

####  Address decoding

The address decoding is performed by two 74LS138 decoders, which
together react to memory operations in the range \>8640-867F (real-time
clock) and \>8680-86BF (AD converter).

The two selection signals are further combined with an AND gate to
provide the Sel\* signal that enables the data bus and triggers the 555
to light up the LED.

         +-------+         +-------+   ,----ClockSel*   
    A2---|S0     |    A9---|S0     |   |
    A1---|S1     |    A8---|S1  Y1*|---+---, 74LS08
    A0---|S2     |    A7---|S2  Y2*|--+----=|)-------Sel*
         |    Y4*|---------|G2A*   |  |
Memen*---|G2A*   |    A4---|G2A*   |  &#39;-----ADCSel*
    A3---|G2B*   |    A5---|G1     | 
    A6---|G1     |         |       | 
         |       |         |       |
         +-------+         +-------+  

####  Real-time clock

The real-time clock is a MM58167 chip. It has an 8-bit data bus which is
connected to the PE-box data bus, taking into account that D0 is the
most significant bit for TI, but the least significant bit for the rest
of the world. The clock also has five address pins, connected to A10
through A14, which provide access to a total of 32 registers. Its WE\*
write-enabling pin is connected to the WR\* line and the RD\*
read-enabling pin to the inverted and delayed DBIN (see
[above](#Bus%20buffers)).

              +-------+
 ClockSel*----|CS*  D0|---D7
     Dbin*----|RD*  D1|---D6
        WE*---|WR*  D2|---D5
        A14---|A0   D3|---D4
        A13---|A1   D4|---D3
        A12---|A2   D5|---D2   Crystal
        A11---|A3   D6|---D1   32.768 kHz
        A10---|A4   D7|---D0            3-30 pF
              |     Oi|-----------+-----||---Gnd
              |       |          ===
              |     Oo|----WWW----+-----||---Gnd
              +-------+    200K         20 pF

The clock needs a 32.768 kHz crystal, with some associated circuitery:
two capacitors (one 20 pF one adjustable 3-30 pF) and a 200K resistor.

The clock chip has the ability to generate a variety of interrupts, but
the corresponding pins are not connected in the MBP design, so this
feature cannot be used to generate alarms.

####  The AD converter

The converter is an ADC0809, 8-bit analog-to-digital converter. It has 8
input channels that can be selected via three address lines. These
address lines are controlled by A12 through A14.

                            +-------+
                      A12---|A2  IN0|---&lt;
                      A13---|A1  IN1|---&lt;
                      A14---|A0  IN2|---&lt;
            A10----,        |    IN3|---&lt;
               ,---=|)------|OE  IN4|---&lt;
ACDSel*---|&gt;o--+            |    IN5|---&lt;
               &#39;---,        |    IN6|---&lt;
            A11----=|)---+--|ALE IN7|---&lt;
                         &#39;--|Start  |
        +-------+           |       |
Phi3*---|Clk* Q2|-----------|Clk    |
     ,--|Rst1   |           +-------+
Gnd--+--|Rst2   |
        +-------+
         74LS93

The active-low selection signal (see above) is inverted to become active
high. It is combined with address line A10 via a 74LS08 AND gate to
activate the output enable OE pin. This means that reading at addresses
\>86A0-86BF will repetedly access the seven ADC registers.

The inverted selection signal is also combined with A11 to trigger both
the address latch ALE pin (which selects an input line) and the Start
pin that causes the ADC to appraise voltage on the selected line. So
these will be triggered by access to the range \>8690-869F and
\>86B0-86BF.

Operations are clocked by the Phi3\* clock signal, divided by 4 with a
74LS93 counter. Since Phi3\* is 3 MHz, the ADC is effectively clocked at
750 KHz.

###  The MM58167A real-time clock

The MM58167A maintains the time from the milliseconds to the months, but
does not support years. An alarm can be set that will automatically
trigger an interrupt pin. This interrupt pin can also be triggered by a
variety of periodic events.

When power is off, the clock can be placed in standby mode and powered
by a +3 volts battery. In this mode all outputs are disabled, except for
a special, dedicated interrupt pin.

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
       RD* |2         23| PwrOff*
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
**PwrOff\*** when low, puts the clock in standby mode (i.e. outputs
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

###   The ADC0809 analog-to-digital converter

The ADC0809 analog-to-digital converter is based on an internal series
of 256 daisy-chained resistors. One end of the chain is tied to the
Vref- pin and the other end to the Vref+ pin. All resistors have the
same value, except the first one (1/2 the value) and the last one (1.5
times the value). If you do the math, you will see that this makes up a
voltage divider with 256 equal steps.

To appraise an external voltage, the ADC compares it to these internal
references. It does not perform 256 comparisons though, rather it starts
at the mid-point, then moves to the upper or lower quarter-mark, etc.
This way it never takes more than 8 steps before it finds the
appropriate value. The total conversion time should be around 100
microseconds. This is equivalent to a dozen or so assembly instructions.

The ADC0809 has eight analog inputs. You select one by placing its
number on the 3-bit address bus, then pulsing the ALE pin high. You then
start the conversion by pulsing the "Start" pin high. The end of the
conversion is signaled by the EOC pin. At this point, you can enable the
data bus by setting the OE pin high, and read the result.

Pulsing "Start" before the end of the current conversion cancels it and
begins a new conversion.

[Pinout
](#ADC%20pinout)[Absolute max ratings
](#ADC%20max%20rating)[Recommended operating conditions
](#ADC%20recommended)[Electrical parameters
](#ADC%20electrical)[Timing diagrams](#ADC%20timing)

#### Pinout

          +----+--+----+ 
      IN3 |1 o       28| IN2
      IN4 |2         27| IN1
      IN5 |3    A    26| IN0
      IN6 |4    D    25| A0
      IN7 |5    C    24| A1 
    Start |6         23| A2 
      EOC |7    0    22| ALE 
       D4 |8    8    21| D0 
       OE |9    0    20| D1 
      Clk |10   9    19| D2 
      Vcc |11        18| D3 
    Vref+ |12        17| D7 
      Gnd |13        16| Vref- 
       D6 |14        15| D5 
          +------------+

**D0-D7** form the data bus. D0 is the least significant bit.
**OE** is used to place the result of the conversion on the data bus
(active high).

**A0-A2** form the address bus that lets you select one input for the
conversion. A0 is the least significant bit.
**ALE** is used to latch the address. This signal is active high.

**Start** triggers the A--\>D conversion process. It should be a
positive pulse whose leading edge resets the successive approximation
register, whereas the falling edge begins the conversion process.
**EOC** signals the end of the conversion. It goes low when the
conversion begins, and high when it's done.
**Clk** clocks the conversion process. Typical frequency = 640 kHz.

**Vcc** is the power supply. It should be +5 volts.
**Gnd** is the ground.
**Vref+** and **Vref-** are the two ends of the resistor chain, i.e.
provide the voltage range whithin which the measured input must fall.
They should not exceed the values of Gnd and Vcc for proper
conversion.
**IN0**-**IN7** are the analog input pins. Their limitations are the
following: -0.1 Vref to +1.1 Vref, maximum 100 milliAmps. Exceeding
these values may destroy the chip.

####  Absolute maximum ratings

    Supply voltage:                -0.3 to 10 V
    All inputs except controls:    -0.3 to Vcc+0.3 V
    Control inputs:                -0.3 to +15 V
    Package dissipation:           875 mW
    Storage temperature:          -65 to 150 `C

####  Recommended operating conditions

|             |     |     |     |       |
|-------------|-----|-----|-----|-------|
| Parameter   | Min | Nom | Max | Unit  |
| Vcc         | 4.5 | 5.0 | 6.0 | Volts |
| Temperature | -40 | .   | +85 | \`C   |

####  Electrical characteristics under recommended conditions


-40 to +85 `C
-1
1
uA
-40 to +85 `C
-1.25
+1.25

####  Timing diagram

      _   _   _   _   _   _   _   _   _|1/F|  _   _   _   _   _   _   _  
    _| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| | Clock

            /<200\                                                       ALE
        |<50|<50 |
    XXXX Valid   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX A0-A2
            | b |
    XXXXXX         Stable                        XXXXXXXXXXXXXXXXXXXXXXX INx

              /<200\                                                     START
              |     | 90-116 us (typ 100 us)    |
              | note a |                        |
                      \                         /                        EOC
                                           |  c |
                                                   /         \           OE
                                                    |        |<250 |
                                                    | <250 |       |
    ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZX Valid  XZZZZ D0-D7

    a) 2 us + 8 clock cycles (of duration 1/F)
    b) Analog multiplexer delay: typ 1 us, max 2.5 us
    c) The data register is updated 1 clock cycle before EOC goes high

All values are in nanoseconds, unless marked us (microseconds). Typical
values are generally are half the indicated max value. NB The figure is
not drawn to scale.

------------------------------------------------------------------------

## Software

[Accessing the clock
](#Acessing%20clock)[Accessing the ADC](#Accessing%20ADC)

### Accessing the clock

The MM58167A real-time clock interface consists in 32 registers. They
can be accessed at addresses \>8640 through \>867E. Only even addresses
are considered, although both bytes are transfered: so just ignore the
least significant byte.

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

    >8640 1/1000 seconds (0-9)
    >8642 1/100 seconds (0-99)
    >8644 seconds (0-59)
    >8646 minutes (0-59)
    >8648 hours (0-23)
    >864A day of the week (1-7, Sunday=1)
    >864C day of the month (1-31)
    >864E month (1-12)

The year is not stored inside the clock chip. The P-Gram DSRs save it
(together with the month) in the DSR space and updates the year upon
power up, if needed.

To implement alarms, the same information can be saved in the clock RAM
at the following addresses:

    >8650 1/1000 seconds (>Cx = always true)
    >8652 1/100 seconds (>Cx = always true)
    >8654 seconds (>Cx = always true)
    >8656 minutes (>Cx = always true)
    >8658 hours (>Cx = always true)
    >865A day of the week (>xC= always true)
    >865C day of the month (>Cx = always true)
    >865E month (>Cx = always true)

The clock chip can be set to automatically compare these values with the
current time and to trigger an interrupt if they match. Setting the
first two bit of any RAM address causes any value to match the current
time (except for day-of-week that should be set as \>0C). This allows
for setting a daily alarm, an hourly alarm, etc.

The remaining registers are:

    >8660 Interrupt status register
    >8662 Interrupt control register
    >8664 Writing >FF here resets the clock
    >8666 Writing >FF here resets the RAM values
    >8668 Status bit: >01 = time was updated. Reset when read.
    >866A Writing here resets the seconds. Minutes are incremented if secs were >=40.
    >866C Standby interrupt: >01 to enable, >00 to disable.
    >866E-7C These registers do not exist
    >867E Test mode, do not use.
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

###  Accessing the ADC

To start the ADC conversion process, just read a dummy word from
addresses \>8690 through \>869E, corresponding to channels IN0 through
IN7 respectively. Writing to these addesses will also work since the
TI-99/4A always carries a dummy read before a write. (NB In Extended
Basic, address \>8690 correspond to -31088.)

Then wait for about 100 usec until the conversion process is finished.
Unfortunately, there is no way you can read the status of the EOC pin
that signals the end of the conversion, since this pin is not connected
on the MBP card.

Read the result from any address in the range \>86A0-86AE (decimal
-31072). As far as I know, the exact address does not matter.

If you would like to start a new conversion as you are reading the
results of the current one, you may try to read from \>86B0 through
\>86BE instead, although I don't guaranty that this will work.

Reading from \>8680-868E has no effect on the converter.

Note that these addresses correspond the sound generator in the console.
Since the generator cannot be read from, there is no conflict with the
ADC. That is, unless you elect to write to the ADC to start the
conversion...

#### Address summary

    >8680-869E: No effect
    >8690:      Start converting channel 0
    >8692:      Start converting channel 1
    >8694:      Start converting channel 2
    >8696:      Start converting channel 3
    >8698:      Start converting channel 4
    >869A:      Start converting channel 5
    >869C:      Start converting channel 6
    >869E:      Start converting channel 7
    >86A0-86BE: Read conversion result
    >86B0-86BE: Read result and start new conversion
Revision 1. 4/30/02. Preliminary.
Revision 2. 5/1/02. OK to release.
[Back to the TI-99/4A Tech Pages](titechpages.md)
