# The peripheral expansion box

This very bulky piece of hardware is meant to support upto eight
peripheral cards that will enhance the capabilities of the TI-99/4A
console. It has its own power supply, which provides each card with the
necessary power for a +5V, a +12V and a -12V (possibly -5V, with a large
heat sink...) voltage regulator.

It is connected to the console side port via a 38-connection cable that
ends up in a peripheral card. And here goes one of the eight available
slot. For some reason, the cable does not carry all lines available on
the console side port (even though some of them are present in the
PE-box bus).

The PE-box is a ventilated unit, and is designed to cool the peripheral
cards when the lid is close. The manual specifies that proper
ventilation cannot be insured if the lid is left open.

The front pannel has little glass window in front of each card slot. A
card can thus use an onboard LED to signal the user that it is in use.
It may be usefull for troubleshooting purposes, but I suspect it was
mainly meant to look cool... Here is a [picture](pebox1.JPG) (not mine).

The PE-box also has room for a disk drive unit and provides it with
power. This drive of course requires a controller card. The original
card by TI could also control two more disk drives, to be intalled
externally (with their own power supply). Later on, some third parties
offered slim units that would allow to fit two disk drives in the space
originally meant for one.

[Power supply](#Power%20supply)
[Internal bus](#Internal%20bus)

**The connection card  **
[Signals buffering](#Signal%20buffering)
[Bus multiplexing](#Bus%20multiplexing)
[Fixing the firehose cable](#Fixing%20firehose)

**A typical peripheral card  **
[General design
](#Typical%20card)[ROM types](#ROM%20types)
[In-circuit programming
](#In%20circuit)[ROM paging](#ROM%20paging)

##  Power supply

The PE-box provides unregulated power, meant to be used by voltage
regulators intalled on each card. Typically, these would be a 78x05
(+5V) , a 78x12 (+12V) or a 79x12 (-12V). According to the specs, these
regulators need an input voltage at least 3V in excess of the required
output voltage. Therefore, the PE-box supplies them with +16V, -16V and
+8V. Note that if you need -5V, you could use a 79x05 and feed it with
the -16V supply. This would just mean that you need a really big heat
sink, since this regulator is going to dissipate a lot of power...

##   Internal bus

The PE-box connect each card to a xxx-lines internal bus. Some of the
lines on the bus are meant for production purposes and are held high in
the PE-box. Supposedly, the card manufacturing machines used them to
burn EPROMs or test the card. Here is a [picture](pebox2.JPG) of the
inside (not mine).

                                    59     Left         1 
                              Front ===================== Rear
                                    60    Right         2

     #  Name   I/O   Use 
    --  -----  ---   -------------------------------------------- 
     1               Power supply for a +5V 3-T voltage regulator (about +8V) 
     2               Ditto          
     3  GND            
     4  READYA  <     System ready (goes to READY, 10K pull-up to +5V) A low level puts the cpu on hold.
     5  GND           
     6  RESET*  >     System reset (active low) 
     7  GND             
     8  SCLK    nc   System clock (not connected in interface card) 
     9  LCP*    nc   CPU indicator 1=TI99 0=2nd generation (not connected in interface card) 
    10  AUDIO   <    Input audio (to AUDIOIN in console) 
    11  RDBENA* <    Active low: enable flex cable data bus drivers (1K pull-up) 
    12  PCBEN   H    PCB enable for burn-in (always High) 
    13  HOLD*   H    Active low CPU hold request (always High) 
    14  IAQHA   nc   IAQ [or] HOLDA (logical or) 
    15  SENILA* H    Interrupt level A sense enable (always High) 
    16  SENILB* H    Interrupt level B sense enable (always High) 
    17  INTA*   <    Interrupt level A (active low, goes to EXTINT*) 
    18  LOAD*   nc   Unmaskable interrupt (not carried by interface cable/card) 
    19  D7      <>   Data bus, bit 7 (least significant) 
    20  GND            
    21  D5      <>    
    22  D6      <>     
    23  D3      <>      
    24  D4      <>      
    25  D1      <>       
    26  D2      <>        
    27  GND              
    28  D0      <>   Data bus, bit 0 (most significant) 
    29  A14     >      
    30  A15     >    Address bus, lsb. Also CRU output bit. 
    31  A12     >      
    32  A13     >       
    33  A10     >       
    34  A11     >       
    35  A8      >       
    36  A9      >       
    37  A6      >       
    38  A7      >        
    39  A4      >       
    40  A5      >       
    41  A2      >       
    42  A3      >       
    43  A0      >    Address but, bit 0 (most significant)
    44  A1      >     
    45  AMB     H    Extra address bit (always High)
    46  AMA     H    Extra address bit (always High)
    47  GND          
    48  AMC     H    Extra address bit (always High)
    49  GND            
    50  CLKOUT* >    Inverted PHI3 clock, from TIM9904 clock generator
    51  CRUCLK* >    Inverted CRU clock, from TMS9900 CRUCLK pin
    52  DBIN    >    Active high = read memory. Drives the data bus buffers.
    53  GND           
    54  WE*     >    Write Enable pulse (derived from TMS9900 WE* pin)
    55  CRUIN   <    CRU input bit to TMS9900
    56  MEMEN*  >    Memory access enable (active low) 
    57               Power supply for a -12 Volts 3-T voltage regulator (about -16V) 
    58               Ditto 
    59               Power suppy for a +12 Volts 3-T voltage regulator (about +16V) 
    60               Ditto

Here is the same info, arranged as on the connector:

              REAR
         +8V  1||2   +8V
         GND  3||4   READY
         GND  5||6   RESET*
         GND  7||8   (nc)
         (nc) 9||10  AUDIO
     RDBENA* 11||12  (high)
      (high) 13||14  (nc)
      (high) 15||16  (high)
      INTA*  17||18  (nc)
         D7  19||20  GND
         D5  21||22  D6
         D3  23||24  D4
         D1  25||26  D2
        GND  27||28  D0
        A14  29||30  A15/CRUOUT
        A12  31||32  A13
        A10  33||34  A11
         A8  35||36  A9
         A6  37||38  A7
         A4  39||40  A5
         A2  41||42  A3
         A0  43||44  A1
      (high) 45||46  (high)
        GND  47||48  (high)
        GND  49||50  CLKOUT*
    CRUCLK*  51||52  DBIN
        GND  53||54  WE*
      CRUIN  55||56  MEMEN*
       -16V  57||58  -16V
       +16V  59||60  +16V
             FRONT

##   The connection card

The main purpose of the connection card is to buffer signals to or from
the console, before to send them out on the PE-box bus. The buffers are
74LS244 TTLs, except for the data bus that requires a bidirectional
transceiver 74LS245. The console end of the cable also contains a
74LS245 chip to buffer the data bus once more. Nevertheless, the TI
specs insist that each card must rebuffer all signal from or to the
PE-box bus.

The LED in the front of the connection card is turned on by DBIN (high =
on). Thus, it will shine for reading operations, and shut off for
writing operations. The end result is that it twinkles when the bus is
active.

Here is a picture of the [card](pebox3.jpg), and one of the
[connector](pebox4.jpg) on the console side of the cable.

#### Signal buffering

  Console          |      :   Side connector         Cable       Connection card        PE-box bus
                   R 4.7K :                                                             ................
          100 Ohms |      :Pin #                                                        :
   Load*-----WWW---+------[13                        Wire #                        Pin #:
   CRUIN-----WWW----------[33]--------------------------1----------------------------[55]---CRUIN
 Extint*-----WWW----------[ 4]-------------------------41----------------------------[17]---IntA*
 AudioIn------------------[44]-------------------------39----------------------------[10]---AudioIn
Ready---------------------[12]-------------------------40---------------+------------[ 4]---Ready
Hold/IAQ-----WWW----------[41                                           &#39;--WWW---+5V    :
    SBE*------------------[2                                               10K          :      
            &#39;367          :                                                 &#39;244        :
           +----+         :     47 Ohms                                    +----+       :
    A13----|    |---------[15]---WWW-------------------23------------------|17 3|----[32]---A13
     A9----|    |---------[18]---WWW-------------------19------------------|8 12|----[36]---A9
     A8----|    |---------[14]---WWW-------------------18------------------|6 14|----[35]---A8
     A7----|    |---------[17]---WWW-------------------17------------------|4 16|----[38]---A7
     A6----|    |---------[29]---WWW-------------------16------------------|2 18|----[37]---A6
    A14----|    |---------[16]---WWW-------------------24------,      ,----|15 5|----[31]---A12
    Gnd----|EN* |         :                                    |     |,----|13 7|----[34]---A11
           +----+         :                                    |     ||,---|11 9|----[33]---A10
            &#39;367          :                                    |     |||   +----+       :
           +----+         :     47 Ohms                        |     |||                :
    A12----|    |---------[11]---WWW-------------------22------|-----&#39;||                :
    A11----|    |---------[ 8]---WWW-------------------21------|------&#39;|    &#39;244        :
    A10----|    |---------[ 6]---WWW-------------------20------|-------&#39;   +----+       :   
     A5----|    |---------[ 5]---WWW-------------------15------|-----------|17 3|----[40]---A5
     A4----|    |---------[ 7]---WWW-------------------14------|-----------|15 5|----[39]---A4 
     A3----|    |---------[10]---WWW-------------------13------|-----------|13 7|----[42]---A3
    Gnd----|EN* |         :                                    | ,---------|11 9|----[56]---Memen*
           +----+         :                                    | |,--------|4 16|----[41]---A2    
Memen*--------------------[32]---WWW--------------------2------|-&#39;|,-------|6 14|----[44]---A1
            &#39;244          :                                    |  ||,------|8 12|----[43]---A0
           +----+         :     47 Ohms                        |  |||,-----|2 18|-+--[52]---DBIN
     A2----|    |---------[20]---WWW-------------------12------|--&#39;|||     +----+ |     :
     A1----|    |---------[30]---WWW-------------------11------|---&#39;||      &#39;244  |     :
     A0----|    |---------[31]---WWW-------------------10------|----&#39;|     +----+ |     :
    A15----|    |---WWW---[19]-------------------------26------|-----|-----|4 16|-|--[30]---A15
  Phi3*----|    |---WWW---[24]--------------------------6------|-----|-----|13 7|-|--[50]---Phi3*
    WE*----|    |---WWW---[26]--------------------------4------|-----|-----|6 14|-|--[54]---WE*   
CRUCLK*----|    |---WWW---[22]--------------------------5------|-----|-----|8 12|-|--[51]---CRUCLK*
   MBE*----|    |---------[28                                  &#39;-----|-----|2 18|-|--[29]---A14
    Gnd----|EN* | 52 Ohms :                                          |,----|3 17|-|-+[11]--RDBENA*
           +----+         :     47 Ohms                              ||,---|5 15|-|-+   :
   Reset-------------+----[ 3]---WWW-------------------36------------|||---|11 9|-|-|-[6]---Reset
          +5V---WWW--&#39;    :                                          |||   +----+ | R 1K:
                4.7K      :                                          ||| +5V      | |   :
                          :                                          ||| &#39;-|&gt;|-\| | +5V :
                          :             47 Ohms                      |||   100  |-+     :
  DBIN--------------------[ 9]-+----------WWW-----------6------------&#39;|| ,-WWW-/| |    8]---SClk
                          :    |  &#39;245                                ||Gnd       |    9]---SCP*
                          :    | +----+                               ||   +----+ |   14]---IAQHA
                          :    | | OE*|----------------43-----WWW-----&#39;&#39;---|OE* | |   18]---LOAD*
                          :    &#39;-|DIR |  47 Ohms               47 Ohms     | DIR|-&#39;     :
    D0--------------------[37]---|18 2|---WWW----------28------------WWW---|18 2|----[28]---D0   
    D1--------------------[40]---|17 3|---WWW----------29------------WWW---|17 3|----[25]---D1   
    D2--------------------[39]---|16 4|---WWW----------30------------WWW---|16 4|----[26]---D2    
    D3--------------------[42]---|15 5|---WWW----------31------------WWW---|15 5|----[23]---D3   
    D4--------------------[35]---|14 6|---WWW----------33------------WWW---|14 6|----[24]---D4   
    D5--------------------[38]---|13 7|---WWW----------34------------WWW---|13 7|----[21]---D5   
    D6--------------------[36]---|12 8|---WWW----------35------------WWW---|12 8|----[22]---D6   
    D7--------------------[34]---|13 9|---WWW----------36------------WWW---|13 9|----[19]---D7  
                          :      | Vcc|-,                                  +----+       :
                          :      +----+ |                                         ,--[ 3]---Gnd
   -5V--------------------[43           |                                         +--[ 5]---Gnd
   +5V--------------------[1            |                                         +--[ 7]---Gnd
   Gnd--------------------[21]--,       |                                         +--[20]---Gnd
   Gnd--------------------[23]--+================3=7=9=25=27=32=37================+--[27]---Gnd
   Gnd--------------------[25]--+       |                                         +--[47]---Gnd
   Gnd--------------------[27]--&#39;       |                                         +--[48]---Gnd
                          :             |                                         &#39;--[53]---Gnd
..........................:             |                                             59]---Unreg +16
                                        |                                             60]---Unreg +16  
                                        |                                             57]---Unreg -16
                                        |                                             58]---Unreg -16
                                        |       ,-----42-44-------------+---------+--[ 1]---Unreg +8V
                                ,-------&#39;       |                       |         &#39;--[ 2]---Unreg +8V  
                                |     78M05     |                       |  ,---WWW---[12]---PCBEN*  
                                |  +--------+   |                       |  +---WWW---[13]---HOLD*      
                              ,-+--|Vout Vin|-+-+                       |  +---WWW---[15]---SENILA
                              | |  |  Comm  | | |                       |  +---WWW---[16]---SENILB     
                        22 uF = =  +---+----+ = = 22 uF                 |  +---WWW---[45]---AMA
                       0.1 uF | |      |      | | 0.1 uF                |  +---WWW---[46]---AMB
                              Gnd     Gnd     Gnd                       |  +---WWW---[48]---AMC
                                                                        |  |  47 Ohms  :...............
                                                                        | +5V    78M05
                                                                        |     +--------+          to
                                                                        +-+---|Vin Vout|-+-+--+5V all
                                                                        | |   |  Comm  | | |      chips
                                                                  22 uF = =   +---+----+ = = 22 uF      
                                                                 0.1 uF | |       |      | | 0.1 uF 
                                                                        Gnd      Gnd     Gnd    
                                                           
Most of the lines are buffered (or generated) by TTLs inside the
TI-99/4A console. In addition, the data bus D0-D7 is buffered in the
connector itself. The connector "black box" contains a 74LS245 buffer
and a 78M05 voltage regulator powered by the PE-box. It also contains 47
Ohm serial resistors placed within many lines (D0-D7, A0-A14, Reset,
DBIN and Memen\*).

Most signals are buffered again in the connection card: the 16 address
lines A0-A15, DBIN, Memen\*, WE\*, CLRCLK\*, RESET\*, CLKOUT\*. As
mentionned above, D0-D7 are buffered again by a second 74LS245 onboard
the card, after another set of 47 Ohm serial resistors. The direction of
both 74LS245 buffers is set automatically by the DBIN line. However, the
peripheral card is responsible for activating these buffers when it
needs them. This is done by lowering the dedicated RDBENA\* line which
is connected to the OE\* pin of each 74LS245 (but not to the console
side port).

You will have noted that all input signals are left unbuffered: CRUIN,
INTA\*, AUDIOIN, READY. Maybe so that the peripheral card has the option
to leave them in high-impedence state? Of these, only READY has a 10 K
pull-up resistor in the connection card.

The "for production line only" pins AMA, AMB, AMC, SENILA\*, SENILB\*,
PCBEN, HOLD\* are pulled up to +5 Volts by 47 Ohms resistors. These
would be really hard to pull down! Anyway, most third party cards ignore
them. The TI disk controller card on the other hand makes use of
several, posssibly for test procedures on the production line.

The PE-Box lines SClk, SCP\*, IAQHA, and LOAD\* are not even connected
to the connection card.

Similarly, some lines present on the console side port are not part of
the cable. These "lost" lines are:
**SBE\*** Output line which is low to signal an access to the speech
synthesizer (address \>9000-97FF). We can dispense with that one.
**MBE\*** Output line which is low to signal an access to the card ROMs
(address in \>4000-5FFF). This one would spare us the need for a 74LS138
decoder!
**IAQ** Interrupt acknowledged by the CPU. Not very usefull anyhow
except possibly for a debugger card.
**LOAD\*** Input line used to send an unmaskable interrupt to the CPU.
This could have been really usefull, too bad!

####  Bus multiplexing

You probably have realised that the PE-box is byte-oriented: it has 16
address lines and 8 data lines (as opposed to 15 address pins and 16
data pins on the TMS9900 CPU). I always wondered why TI decided to
cripple their system in such a way. Probably because most peripherals in
the early eighties were byte-oriented. Or maybe it was just to save a
few lines in the connection cable? Or else, just to copy IBM and make
the same mistake they did with the PC XT??

Anyhow, what it means is that the data bus has to be multiplexed. This
is achieved by a small logic circuit inside the TI console: any memory
access to the range \>2000-7FFF and \>A000-FFFF is multiplexed. The
least significant byte (odd address, D8-D15) is passed first, and this
is signaled by a high level on the additional address line A15. For
input operations, this byte is latched into the console by a 74LS373
D-type latch. Then the multiplexer puts the CPU on hold (with the READY
line) and places the most significant byte on the data bus (even
address, D0-D7), which is signaled by a low level on A15. The operation
will only be completed after 4 clock cycles on Phi3\*, by releasing the
block on the TMS9900.

Concretely, this means several drawbacks:

It is impossible to access only one byte: each read/write instruction
actually consists in two read/write cycles, one for the odd byte and one
for the even byte.

While dealing with the second byte, the CPU is put on hold. Thus all
peripheral access is of the 4 wait-state type.

As the TMS9900 has a 16-bit data bus, it never deals with single bytes.
Even an instruction like MOVB actually writes a 16-bit word. To avoid
erasing the lower byte, the CPU reads the word first, changes the most
significant byte and rewrites the whole word. For peripheral card it
means that every write operations (4 wait cycles) is preceded by a read
operation to the same pair of addresses (4 more wait cycles).

If you wanted to use a peripheral with a 16-bit data bus, you could use
the circuit below to de-multiplex the PE-box data bus. It is similar to
the multiplexing circuit in the console, with a few modifications
(tested by yours truly: works fine).

                                     Even byte       +------+ 
                                ,-------------------&gt;|B0  A0|&lt;----------&gt;D15
                              ,-|-------------------&gt;|B1  A1|&lt;----------&gt;D14
                            ,-|-|-------------------&gt;|B2  A2|&lt;----------&gt;D13
                          ,-|-|-|-------------------&gt;|B3  A3|&lt;----------&gt;D12
                        ,-|-|-|-|-------------------&gt;|B4  A4|&lt;----------&gt;D11
                      ,-|-|-|-|-|-------------------&gt;|B5  A5|&lt;----------&gt;D10
                    ,-|-|-|-|-|-|-------------------&gt;|B6  A6|&lt;----------&gt;D9
                  ,-|-|-|-|-|-|-|-------------------&gt;|B7  A7|&lt;----------&gt;D8
                  | | | | | | | | A15&gt;-------,     ,-|G* DIR|----&lt;DBIN
                  | | | | | | | | CardSel*&gt;--=)&gt;---&#39; +------+  
                  | | | | | | | |           74LS32
                  | | | | | | | |                                    Peripheral
 PE-Box           | | | | | | | |            74LS373                 16-bit bus
 8-bit bus        | | | | | | | |            +------+                       
  D0&lt;-------------|-|-|-|-|-|-|-+-----------&gt;|D0  Q0|--+----------------&gt;D7
  D1&lt;-------------|-|-|-|-|-|-+-|-----------&gt;|D1  Q1|--|-+--------------&gt;D6
  D2&lt;-------------|-|-|-|-|-+-|-|-----------&gt;|D2  Q2|--|-|-+------------&gt;D5
  D3&lt;-------------|-|-|-|-+-|-|-|-----------&gt;|D3  Q3|--|-|-|-+----------&gt;D4
  D4&lt;-------------|-|-|-+-|-|-|-|-----------&gt;|D4  Q4|--|-|-|-|-+--------&gt;D3
  D5&lt;-------------|-|-+-|-|-|-|-|-----------&gt;|D5  Q5|--|-|-|-|-|-+------&gt;D2
  D6&lt;-------------|-+-|-|-|-|-|-|-----------&gt;|D6  Q6|--|-|-|-|-|-|-+----&gt;D1
  D7&lt;-------------+-|-|-|-|-|-|-|-----------&gt;|D7  Q7|--|-|-|-|-|-|-|-+--&gt;D0 
                  | | | | | | | | Odd byte   |OE* LE|  | | | | | | | |
                  | | | | | | | |            +------+  | | | | | | | |
                  | | | | | | | |              |   |   | | | | | | | |
                  | | | | | | | |   74LS244    |   |   | | | | | | | |
                  | | | | | | | |   +------+   |   |   | | | | | | | |
                  | | | | | | | &#39;---|Y0  A0|&lt;--|---|---&#39; | | | | | | |
                  | | | | | | &#39;-----|Y1  A1|&lt;--|---|-----&#39; | | | | | |
                  | | | | | &#39;-------|Y2  A2|&lt;--|---|-------&#39; | | | | |
                  | | | | &#39;---------|Y3  A3|&lt;--|---|---------&#39; | | | |
                  | | | &#39;-----------|Y4  A4|&lt;--|---|-----------&#39; | | |
                  | | &#39;-------------|Y5  A5|&lt;--|---|-------------&#39; | |
                  | &#39;---------------|Y6  A6|&lt;--|---|---------------&#39; |
                  &#39;-----------------|Y7  A7|&lt;--|---|-----------------&#39;
                                    |G1*G2*|   |   |
                                    +------+   |   |        74LS04
                                      |  |     |   |     ,---o&lt;|---WE*
                           74LS139    &#39;--+     |   &#39;--(|=----------A15
                           +------+      |     |    74LS08    
               A15&gt;-------&gt;|S0 Y3*|------&#39;     | 
               DBIN&gt;------&gt;|S1 Y0*|------------&#39;             
                           |   Y1*|-nc
              CardSel*&gt;---&gt;|G* Y2*|-nc
                           +------+  

CardSel\* is an active low selection signal generated by your interface
card (see below).
The '245 is activated by CardSel\*, only when A15 is low, i.e. for the
even (most significant) byte. Its direction is set by DBIN.
The '244 is active when both A15 and DBIN are high, i.e. when reading an
odd (least significant) byte.
The '373 is only addressed by writing operations. It latches the odd
byte when A15 is high and WE\* low, and outputs it to the 16-bit bus
when A15 and DBIN are low , thereby reconstituting the 16-bit bus as the
TI-99/4A sends the even byte.
NB The 8-bit bus is numbered the way TI does, i.e. D0 is the most
significant bit. The 16-bit bus is numbered like everyone else does,
i.e. D15 is the most significant bit.

### Fixing the "firehose" cable

The most delicate part of the PE-box is probably the connection cable
(aka "firehose"): it's easy to bend and/or bang it, and it may
eventually become defective. Not to worry though: it can easily be
replaced.

#### Symptoms

Depending on which line was broken, the symptoms may vay quite a bit.
For instance, I had one case when the console would boot by itself, but
crash when the connection cable was plugged in (even with the PB-Box
off, even with the card out of the PE-Box).

If it boots, you can check the data and address busses with the Easy-Bug
function of the Mini-memory, or CALL LOAD and CALL PEEK in Extended
Basic: write \>FF (i.e. 255) and \>00 at addresses \>3333 (13107) and
\>CCCC (-13108). Then check if the data is ok, and if it was written at
the right place.

If the memory expansion works but you cannot select any other peripheral
card, then it's probably the CRUCLK line that broke.

A key symptom may be that the problem comes and goes as you fumble with
the cable, bend it, stretch it, etc. This points toward a broken wire
inside the cable.

#### Differential diagnostic

Poor contact on the side port (or inside the PE-Box). Oxidation is our
\#1 ennemy with 20-years old hardware.

Broken wire inside the connection cable.

Dead chip on the connection card, or the side connector.

#### Repair

First, make sure the problem is not bad connnections in the side port.
Get a contact cleaner spray and clean the console side port and the
firehose connector. While you are at it, remove the connection card from
the PE-box and clean its contacts, as well as its slot in the PE-box
(heck, clean ALL slots in the PE-box)..

If this does not solve the problem, a wire may be broken, or a chip may
be defective. Let's first check the cable.

Remove the connection card from the PE-Box (remove the one screw that
keeps it in).

Open its metal case: remove the two handles by force, then remove the 4
screws. The label can stay and serve as a hinge.

Unscrew the plastic brace across the end of the cable.

Remove the metal braces that are under so as to expose the cable
terminals.

At the console-end of the cable, remove the 4 screws that keep the black
connector closed, then take the small board out of its metal casing.

Refer to these pictures to identify the components on the
[card](pebox3.jpg), and on the [connection board](pebox4.jpg). Now you
are ready to test the cable (we couldn't do it without dismantelling the
cable, because of the resistors and buffer chips). Flip both boards
upside down, so you can access the pins from under. Using a continuity
tester, make sure current goes from each pin on the console end of the
cable, to its corresponding pin on the card side. If you already know
which line is bad, refer to the [above schematic](#Signal%20buffering)
to locate its wire and test it directly. Remember that the problem may
come and go as you move the cable.

If everything looks ok, then is may be that a given buffer chip is dead.
These are very common and cheap TTLs, so you should not have any problem
buying a spare. Then just unsolder the original (using a soldering
pump), and solder back the new chip.

Replacing the cable is not much harder:

First obtain about 5 feet of 44-wire flat cable.

Remove the brace on the cable, at the console-end.

On each board, mark the same side of the cable, so you know how to put
back the new one. The cable should have colored stripes, which makes it
easy to know which side is which.

Disconnect the cable at each end. This is a bit tricky: it requires
lifting the little platic cover that's over the end of the cable. You
cannot remove it completely though, because it's partly wrapped around
the cable: you will need to slide it off the end of the cable. Then just
peel the cable off the pins.

Tape your new cable to one end of the old one, using strong electrical
tape.

Pull the other end of the flat cable out of the firehose, bringing the
new cable along. Make sure you do not remove the steel cable inside
(it's here to absorb any tension applied to the firehose). Also, take
care not to peel away the shielding layer inside the firehose. Tip: this
is easier to do for two people, one at each end of the firehose.

Now fasten the cable at each end, refering to your marks for proper
orientation. Slide the connector lid over the tip of the cable, the
press the cable over the connectors. You may need pliers, or a wise, to
close the lid tight. The goal is to cause the cutting ends of the pins
to bite through the plastic, into each copper wire.

Repeat your point-to-point tests, to make sure that the new cable makes
proper contacts everywhere. If so, you can put everything back together:

Fasten back the plastic brace across the console end of the cable,
trapping the steel cable in. Then install the metal and plastic boxes.
Tip: one side of the metal box has holes, into which the plastic box
inserts small bulges. The other side should be braced agains the back of
the first one. The goal is that both metal lips are flushed with the
card-edge connector.

Place the metal braces back on the connection card: one has two pins
that go through the board. The other has slots to accomodate these pins.
Push it forward, so that it locks the pins.

Fasten the plastic brace across the cable, the steel cable, and the top
metal brace.

Put the card back in its metal case, if desired (the case is not really
necessary). Install the card inside the PE-Box and make sure everything
works as intended.

##  A typical peripheral card

In case you want to design your own peripheral cards, there are a few
common features that should be present on each and every card. The
schematic below represents a typical card, but you don't have to adopt
all my design choices (especially since I did not test all of them
yet!). If you need help about a chip, have a look at my [TTL
chips](ttl.md) page.

Power supply.
Typically, you would use a 78L05 connected to the +8V power line.If you
need more current than the 100 mAmps delivered by the "L" version, you
could use a 7805 (1 Amp), a 78T05 (3 Amp) or a 78H05 (5 Amp). By all
means, use a heat sink.
If you need +12V, you may similarly use a 78L12, a 7812, a 78T12 or a
78H12.
Negative voltages can be derived from the -16V supply, via 79x12 or
79x05 regulators.

Bus buffering
The address lines require two 74LS244 buffers. The extra CPU output
lines require a third one.
The data bus is buffered by a 74LS245, with the DIR pin driven by DBIN
(high = read cycle) and the EN\* pin driven by RDBENA\*.
The extra TMS9900 input lines (if you are using them) can be buffered by
74LS125. This way, you can keep them in high impedance state when your
card is not accessed.

CRU logic
As described in the [page](cru.md) on CRU, you'll need a 74LS259 for
CRU ouput from the TMS9900, and a 74LS251 for CRU input operations. It
is generally good practice to echo each bit from the '259 to the '251,
unless you have a need for specialised input bits. In addition, some
selection logic is needed: a 74LS85 does a pretty neat job, as it allows
the user to select the CRU address of the card with a DIP switch
(tested: works fine). In this schematic, I also included a master switch
(accessible from the back of the PE-pox) so that the user can shut off
the card if necessary.

ROM selection logic
A 74LS138 decoder can be used to detect memory accesses in the range of
the card ROM, \>4000-5FFF. The G2A\* active low selection input is
driven by MEMEN\*, so that the card only reacts to memory operations. I
also included a master switch (which may or may not be the same as the
one for the CRU) connected to G2B\*: when the switch is open the card is
never selected. Finally, the G1 enabling input of the decoder is
controlled by CRU bit 0, so that the card will only answer when it is
enabled via the CRU. You may want to use that bit to enable/disable
other important functions on your card: in this exemple all it does is
to enable the data bus.

Of course, nowadays many people would use a GAL (programmable logic) to
implement the selection logic, both for ROM and CRU...

Card light
The LED is generally driven by CRU bit 0, but that's up to you.

           +----+                                           SRAM (8K)
     A0&gt;---|    |                                        +-------------+  
      .    |    |                    &#39;138         A4&gt;---&gt;|A11          |
      .    |    |                   +----+         .     |           D7|&lt;----&gt;D0
      .    |    |             A0&gt;---|S2  |         .     | .         . |       . 
      .    |    |             A1&gt;---|S1  |         .     | .         . |       . 
      .    |    |             A2&gt;---|S0  |       A15&gt;---&gt;|A0         D0|&lt;----&gt;D7 
     A7&gt;---|    |                   | Y2*|----+---------&gt;|CE*          |        
      Gnd--|EN* |                   |    |    |          |             |       
           +----+        MEMEN*&gt;---&gt;|G2A*|    |          |             |     
            &#39;244    +5V---WWW--+---&gt;|G2B*|    |  WR*&gt;---&gt;|WE*          |    
           +----+         10K  | ,-&gt;|G1  |    |          |             |
     A8&gt;---|    |     Gnd--o o-+ |  +----+    |          +-------------+ 
      .    |    |                |            &#39;-----&gt;CardSel*          
      .    |    |                |                               LED              
      .    |    |                &#39;-------------+-----------------|&lt;|------+5V
      .    |    |                  +-----+     |      +-----+                     
      .    |    |        RESET*---&gt;|RST  |     |      |     |
     A15&gt;--|    |          A12&gt;---&gt;|S2 Q0|-----+-----&gt;|D0   | 
      Gnd--|EN* |          A13&gt;---&gt;|S1 Q1|-----------&gt;|D1   | 
           +----+          A14&gt;---&gt;|S0 Q2|-----------&gt;|D2   | 
            &#39;244                   |   Q3|-----------&gt;|D3   | 
           +----+          A15&gt;---&gt;|D  Q4|-----------&gt;|D4   | 
           |    |            &#39;32   |   Q5|-----------&gt;|D5   | 
 MEMEN*&gt;---|    |  CRUCLK*&gt;--=)&gt;--&gt;|EN Q6|-----------&gt;|D6   |  
    WE*&gt;---|    |           |      |   Q7|-----------&gt;|D7 EN|--,      
CRUCLK*&gt;---|    |           |      +-----+            +-----+  |   
CLKOUT*&gt;---|    |           &#39;----------------------,           | 
 RESET*&gt;---|    |                   +-----+  &#39;04   |           | 
   DBIN&gt;---|    |---,               |  A=B|--|&gt;o---+-----------&#39;        
      Gnd--|EN* |   |        A3&gt;---&gt;|A=B  |          10K         
           +----+   |        A4&gt;---&gt;|A1 B1|------+---WWW---+-+5V 
            &#39;245    |        A5&gt;---&gt;|A2 B2|-----+|---WWW---+    
           +----+   |        A6&gt;---&gt;|A3 B3|----+||---WWW---+  
           | DIR|&lt;--&#39;        A7&gt;---&gt;|A4 B4|---+|||---WWW---&#39;  
    D0&lt;---&gt;|    |        10K  Gnd--&gt;|A&lt;B  |   ||||             
     .     |    |    +5V-WWW---+---&gt;|A&gt;B  |   oooo  DIP        
     .     |    |              |    +-----+   oooo  switch 
     .     |    |    Gnd--o o--&#39;     &#39;85      ||||
     .     |    |                             ++++-Gnd
     .     |    |                                                         
     .     |    |                                                         
    D7&lt;---&gt;|    |                                                         
           | OE*|&lt;--,                                                         
           +----+   |   ,----------&lt; CardSel*                           
                    |   |                                                       
RDBENA*&lt;------------+---&lt;|--Gnd
                       &#39;125  
                                  78L05                          All chips
                               +----------+      +5V             +-----+
+8V--------------------+-------|Vin   Vout|---+-----... --+------|Vcc  |    
                       |       |   Gnd    |   |           |      |     |
                       = 22 uF +----------+   = 0.1 uF    =0.1uF |     |
                       |            |         |           |      |     |
                      Gnd           Gnd       Gnd         &#39;------|Vss  |
                                                                 +-----+

####  ROM types

When it comes to choosing a memory chip to install the card DSRs in, you
have several choices:

ROM are Read-Only Memories. They are manufactured with a given content,
that cannot be altered. Obviously, you can't buy these!

PROMs are programmable Read-Only Memories. They come filled with binary
"1", i.e. each byte contains \>FF. You can program a bit to "0", but
this operation is irreversible: zeros cannot be converted to ones. The
programming operation generally requires a special programming device.

EPROM are erasable PROMs with an expensive quartz window that allows you
to erase them with UV light, in case you want to modify their contents.
Most EPROMs also come in OTP version (one-time programmable), without
the quartz window. The idea is to use the erasable device for your
prototype and then the OTP for mass production.

EEPROM are EPROM that can be erased electrically, one byte at a time
(which takes a long time as erasing operations are very slow).

FLASH are EEPROMs that can be erased sector-wise, e.g. you can erase 1K
at a time. This is much faster, hence their name, but the drawback is
that you cannot modify individual bytes.

SRAM (static RAM) are way easier to use that DRAMs (dynamic RAM)
although more expensive. And they can be written to without any
problems, contrarily to EEPROMs or FLASHes. Of course, they loose all
data when the power goes off, but you can prevent this by supplying them
with a small lithium battery. This is what was done in the
[Mini-memory](mm_mod.htm#Hardware) cartridge: the upper 4K are
persistent RAM.

Price is of course a consideration, but nowadays memory becomes cheaper
and cheaper, at least at the scale of the TI-99/4A. Therefore, our
choice is basically down to EEPROMs or SRAM, possibly FLASH. These can
easily be mail-ordered from several places on the Web (Digikey, Jameco,
Mouser, etc). For your information, here is a comparative table I came
up with, after only an hour surfing the net. You could fill in the
blanks, with a little more hunting...


Intel $3.95
Intel $3.95
STM $2.95
AT28C16
Atmel $3.55
STM (+battery) $7.30
TMM2016APP
$1.09
National $4.05
Microchip $3.10
Samsung $2.95
AT28C64A-20
Atmel $5.05
Hitachi $2.29
M48Z1810PCI
STM (+battery) $11.26
Thompson $2.95
AMD $4.49
Atmel $6.95
Thompson
NEC $1.61
HM62256LB-12
Hitachi $2.09
AMD $3.85
Thompson $5.50
AMD $4.68
Atmel $225.20 !
AMD $5.0
NEC $2.52
HM628128LB-85
Hitachi $11.95
AMD $4.95
AMD $15.00
$7.41
AMD $5.95
Atmel $978.60 !
AMD $12.88
NEC $13.42
AMD $19.63
AMD $20.50

Notes:

The price does not only depend on the size, but also on the speed of the
memory. Since the TI-99/4A is extremely slow (400 ns), we can go ahead
and buy the slowest and cheapest chip.

Some retailers take a larger benefit margin than others (see Arrow's
crazy prices for the AT28C020 and AT28C40). So you may be able to find a
better deal than what is indicated in this table.

Pay attention to the package when you order: is it DIP (dual in line) or
surface mount (tiny chips very hard to handle)?

#### In-circuit programming

Rather than using an expensive EEPROM programmer, it would be nice to
install an EEPROM (or a FLASH) so that it can be programmed while inside
the PE-box. Traditionnally, writing and erasing is done by applying a
programming voltage (+12V, +13V, +25V, etc) on a given pin. This could
be implemented on a peripheral card with a +12V voltage regulator, whose
output is applied to the programming pin of the EEPROM via a PNP
transistor, controlled by a CRU bit for instance.

But there is a better solution: you can now find FLASH and EEPROM
memories that have a built-in voltage converter. That is, they only need
the regular +5V applied to Vcc. FLASH generally have a protection
against spurious writings: to program or to erase them, you must first
send an "unlocking sequence" (kind of a password). This is a slight
annoyance for us, since the TI-99/4A won't write a single byte in 8-bit
memory. We saw above that it will perform 4 access operations: first
reading a pair of addresses, then writing to them. This will mess up the
unlocking sequence and prevent us from writing to the chip. Unless we
add extra hardware to accept only the relevant write cycle and filter
the other three: such a circuit could be enabled by a CRU bit, so as not
to disturb regular ROM reading operations (see below).

Unlike FLASH, then +5V EEPROM generally don't require an unlocking
sequence, but writing a byte will stall the chip for a moment, while the
internal logic is writting it. So again we will need some kind of
circuitery to access odd and even bytes independently.

A simple solution would be to have two memory chips: one for odd bytes
and one for even bytes. The chips can be piggy-backed, with all their
corresponding pins connected together except for the CE\* pins. These
are driven by the CardSel line combined by OR gates with either A15
(even bytes) or the inverse of A15 (odd bytes).

WE*&gt;-------------------,                 +-----------+
            &#39;04        | &#39;32             |           |-+
Bit1&gt;---+---|&gt;o--------==)&gt;--------------|WE*     An | |&lt;---Bit n
        |                                |         . | |    .
        |                                |         . | |    .
DBIN&gt;---&#39;==|)--,                         |         . | |    .
          &#39;08  |  &#39;32                    |        A12| |&lt;---Bit 2
CardSel*&gt;------&#39;==)&gt;---+---,             |        A11| |&lt;---A3
                       |   |             |         . | |    .
                       |   |             |         . | |    .
                       |   |  &#39;32        |         . | |    .
          ,----------------&#39;==)&gt;---------|CE*      A0| |&lt;---A14
A15&gt;------+            |                 +-----------+ |
          &#39;----|&gt;o-----&#39;==)&gt;--------------|CE*         |
               &#39;04        &#39;32             +------------+       

In this circuit I also filtered the WE\* line by combining it with the
inverse of CRU bit 1. This provides a write-protection mechanism: you
can only write to the memory if CRU bit 1 is set to 1. Note that all CRU
bits are 0 at power-up time, so the memory will be write-protected by
default. For the opposite behaviour, remove the 74LS04 inverter.

CRU bit 1 also serves to prevent read operations, when set to 1. This is
only required for chips that have an unlocking sequence: it prevents the
read operation that comes along with each write in the TI-99/4A system,
from messing up the unlocking sequence. If you are using EEPROMs that
have no unlocking sequence, CardSel\* can be directly combined with A15
and not-A15.

Finally, additional CRU bits could be used to page the memory, so as to
implement more than 8 Kbytes.

The controlling software could be something like this:

* Address in R0, word in R1`
SELPAG LI   R12,&gt;1x00        CRU address of our card
       SBO  0                Enable access
       SBO  1                Enable writing, disable reading
       LI   R2,&gt;AA55         Unlocking code
       MOVB R2,@&gt;4AAA        This is address &gt;0555 in the chips
       SWPB R2
       MOVB R2,@&gt;555A        Now it&#39;s unlocked
       LI   R2,&gt;A000         Command &quot;program byte&quot;
       MOVB R2,@&gt;4AAA        Send command
       MOVB R0,*R1           Write the byte
       SBZ  1                Enable reading, disable writing
       SBZ  0                Disable card
       B    *R11

This example assumes that the unlocking sequence is the following:

1.  write \>AA to address \>0555
2.  write \>55 to address \>0AAA
3.  send a command code (\>90 to write)
4.  write the byte.

Remember that there are two chips, (one for odd bytes, one for even
bytes) so the addresses need to be multiplied by two and become \>0AAA
and \>1554 respectively. To which we add \>4000, which is the address of
the card.

If you don't like the idea of having two FLASH chips to implement only
8K of memory, here is an alternative: a circuit that lets you access
only even bytes (or only odd bytes) at will.

           &#39;32       &#39;08    &#39;32         +-----------+
DBIN&gt;------==)&gt;-----==|)----==)&gt;-------&gt;|CE*      An|&lt;---CRU bit n
           |        |       |           |        ...|    ...
A15&gt;-------&#39;        |       |           |        A13|&lt;---CRU bit 3
                    |       |           |           |
CRU bit 1&gt;----------&#39;       |           |        A12|&lt;---A3
                            |           |        ...|   ...
CardSel* &gt;------------------&#39;           |         A1|&lt;---A14
                                        |           |
                                        |         A0|&lt;---&lt;(==----&lt;A15
                                        |           |       |
                                        +-----------+       &#39;----&lt;CRU bit 2

When either DBIN is high (reading) or A15 is high (access to odd
addresses) the output of the 74LS32 OR gate is high and will prevent
access to the FLASH.

The circuit is enabled by CRU bit 1, with the help of a 74LS08 AND gate:
the bit must be one for the output to reflect the OR gate. Otherwise it
is always low. (If you don't want to use an AND gate, you can replace it
with an OR having inverters on both inputs and on the output).

The second OR gate filters the active low card selection signal applied
to the FLASH: if the output of the AND gate is high, so will be CE\* and
the FLASH won't react to this memory cycle.

Since this circuit only enables access to even bytes, we need a way to
select odd addresses in the FLASH: CRU bit 2 is used to force up the A0
flash input, no matter what A15 is.

The FLASH programming routine should be modified accordingly:

* Assumes a source pointer in R1, and the destination address in R2`
FLASHW LI   R12,&gt;1x00       CRU address of your card
       SBO  0               Enable card
       SBO  1               Enable filtering circuit
LP1    LI   R0,&gt;AA55        Unlocking bytes
       MOVB R0,@&gt;4555       That&#39;s address &gt;0555 on the FLASH
       SWPB R0
       SBO  2               Make next address odd
       MOVB R0,@&gt;4AAA       That&#39;s address &gt;0AAA on the FLASH
       LI   R0,&gt;9000        Command byte for &quot;write&quot;
       MOVB R0,@&gt;4AAA       Send it
       COC  @H0001,R2       Check if destination address is odd or even
       JEQ  SK1             Odd
       SBZ  2               Even: stop forcing last address bit high
SK1    MOVB *R1+,*R2+       Write byte in FLASH
       SBZ  2               Enable A15 again
       B    @WAIT           Wait a while (optionally, check FLASH status)

#### ROM paging

What if you need more than 8K of ROM in your card? Well, nothing
prevents you from installing a larger EEPROM/FLASH: these are quite
cheap nowadays. But you'll just need some sort of switching mechanism to
page the memory into the \>4000-5FFF bank. One way to do it is to drive
the extra address lines with CRU bitsas demonstrated in he circuit
above.

Another way is to use a 74LS273 latch to store a page number. This chip
can latch 8 bits, so you'll have a maximum of 256 pages. This comes to
256 \* 8K = 2 Megs of ROM. This should be enough...

                                   +------+           +----------+
                            A14---&gt;|D0  Q0|----------&gt;|A12       |
                             .     |      |           | .        |
                             .     |      |           | .        |
                             .     |      |           | .        |
                            A7----&gt;|D7  Q7|----------&gt;|A19       |
                                   |      |    A4----&gt;|A11       |
CardSel*------------------,        |      |    ...    |...       |
WE*--------------,       ,===)&gt;---&gt;|CLK   |    A15---&gt;|A0        |
CRU bit 3--|&gt;o---&#39;==)&gt;---&#39;         |      |           |          |
                        RESET*----&gt;|RST*  |           |          |
                                   +------+           +----------+

To avoid the problems due to data bus multiplexing, this design uses of
a commonly used TI trick: latching a value in the 74LS273 is done by
writing to an address in the range \>4000-5FFF. Bits A7-A14 of the
address bus are latched by the pulse on the WE\* line, and make up the
new A12-A19 for the memory chip (we cannot latch A15 since it is
automatically toggled by the multiplexer). Of course, we don't have to
use all eight bits: even if we latch eight, only those that we need will
be connected to the memory chip.

CRU bit 3 is used to enable latching at will. This is only necessary if
you plan to write to this memory , e.g to program a FLASH/EEPROM, or if
you are using static RAM. If you are using a pure ROM, you can assume
that it will never be written to and dispense with CRU bit 3: every
write operation is considered as a page selection.

The RESET\* line insures that page 0 will always be selected at power-up
time. That's required if you want to place a [standard
header](headers.md) in your card ROM.

The control software would look like this:

* Page number is expected in R0 (range 0-256)`
SELPAG LI   R12,&gt;1x00        CRU address of our card
       SBO  0                Enable access
       SBO  3                Enable latching (if installed)
       SLA  R0,1             To use A7-A14, instead of A8-A15
       MOVB *R0,*R0          What we write doesn&#39;t matter, the address does
       SBZ  3                Disable latching
       B    *R11
Revision 1. 3/24/99 OK to release

Revision 2. 3/30/99. Polishing

Revision 3. 8/16/99. Corrected stupid mistakes in the discussion bus
multiplexing. Added de-mpx schematic.
Revision 4. 5/3/00. Simplified in-circuit programming (using two chips).
Added price table.
Revision 5. 5/14/00. Corrected a timing problem in the demultiplexer.

Revision 6. 8/16/00. Added links to pictures.
Revision 7. 1/21/02. Added schematics of the cable/card.
Revision 8. 1/27/02. Added pictures of connection card.
[Back to the TI-99/4A Tech Pages](titechpages.md)
