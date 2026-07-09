# Pinouts

These informations have been extracted from a very usefull handbook
published by TI in 1983, with many informations, timing diagrams,
console schematics, etc:

          TI-99/4A Console 
                and 
     Peripheral Expansion System 
           Technical Data 

**Console**
[Joystick port](#Joystick)
[Cassette port](#Cassette)
[Power ports](#Power)
[Monitor port](#Monitor)
[Cartridge port](#Cartridge)
[Side port](#Side)
[Keyboard connector](#Keyboard)

**Peripherals**
[PE-box slots](#PE-box)
[Floppy disk controller connector](#Floppy)
[PIO connector](#PIO)
[RS232 connector](#RS232)

**Chips** (links to other pages)
[TMS9900 CPU](http://www.nouspikel.com/ti99/tms9900.htm#Pinout)
[TMS9901 Programmable system 
interface](http://www.nouspikel.com/ti99/tms9901.htm#Pinout)
[TMS9902 UART](http://www.nouspikel.com/ti99/rs232c.htm#Pinout9902)
[TIM9904 Clock generator &
driver](http://www.nouspikel.com/ti99/tim9904.htm#Pinout)
[TMS9919 Sound
generator](http://www.nouspikel.com/ti99/tms9919.htm#Pinout)
[TMS9918A
Videoprocessor](http://www.nouspikel.com/ti99/tms9918a.htm#Pinout)
[FD1771 Floppy disk
controller](http://www.nouspikel.com/ti99/disks.htm#FD1771pinout)
[FD179x Floppy disk
controller](http://www.nouspikel.com/ti99/disks.htm#FDCpinout)
[TMS5220C Speech
synthesizer](http://www.nouspikel.com/ti99/speech.htm#Pinout)

##  Joystick port

                      ___________ 
                     ( 1 2 3 4 5 ) 
                      \ 6 7 8 9 / 
    # I/O Use          \_______/ 
    - --- ------- 
    1     not used 
    2  >  Test joystick 2 
    3  <  Up 
    4  <  Fire button pressed 
    5  <  Left 
    6     not used 
    7  >  Test joystick 1 
    8  <  Down 
    9  <  Right

##   Cassette port

                     ___________ 
                    ( 1 2 3 4 5 ) 
                     \ 6 7 8 9 / 
    # I/O Use         \_______/ 
    - --- ------- 
    1  >  Cass 1 motor control (CRU bit 22) 
    2  >  Ditto
    3  >  Output to tape 2 (CRU bit 25) 
    4  >  Audio gate (CRU bit 24) 
    5  >  Output to tape 2 (neg) 
    6  >  Cass 2 motor control (CRU bit 23) 
    7  >  Ditto
    8  <  Input from tape 1 or 2 (CRU bit 27) 
    9  <  Ditto

N.B. Cassette 1 can't be written to. Signals are buffered by
opto-isolators.

##   Power port

### Inside console (regulated)

                              
                   < 1 2 3 4 |                               
    #  Volts
    -  ----- 
    1   -5 
    2  +12 
    3  GND 
    4   +5

###  External plug

        Transformer      Plug
    >_______   _______> >-----1 (red) 
            )|( 
      2     )|( 
      2     )|( 
      0     )|(-------> >-----2 (white) 
      V     )|( 
      ~     )|( 
    >_______)|(_______> >-----4 (black)

                        >-----3 (nc)

NB: for US users replace 220V with 110V
The AC values on pins 1, 2 and 4 appear to vary according to the model
of transformer. Therefore I did not indicate them here.

##   Monitor port

              
    #  Use     
    -  ----- 
    1  12V vid 
    2  R-Y (color burst clock) 
    3  Sound output 
    4  Y 
    5  B-Y (external video input?) 
    U  GND
I was told that US console have a different connector, for NTSC
standard. I don't know what it looks like, but the pinout is supposed to
be:

    #  Use 
    -  ----- 
    1  +12V
    2  Ground
    3  Sound
    4  Composite video
    5  Signal ground

##   Cartridge port

     ______________________________________________________________ 
    |                                       2                  36  |
    |                                       [====================] | 
    |_______________________________________1__________________35__| 
    |___|___|___|___|___|___|___|___|___|_|                        | 
    |_____|___|___|___|___|___|___|___|___|                        | 
    |___|___|___|___|___|___|___|___|_____|                        | 
    |_____|___|___|___|___|___|___|___|___|                        | 
    |___|___|__________________|___|______|________________________|

       TOP    RIGHT   BOTTOM

        Vss  36||35  GND
      ROMS*  34||33  Vss    ROMS* is low if addr in >6000-7FFF
        WE*  32||31  GR   High = GROM ready 
         A4  30||29  -5V     
         A5  28||27  GRC  GROMCLK (or CPUCLK) from the VDP 9918A 
         A6  26||25  DBIN 
         A3  24||23  A14
         A7  22||21  GS*  Low if addr in >9800-9FFF 
         A8  20||19  +5V
         A9  18||17  D0
        A10  16||15  D1 
        A11  14||13  D2   
        A12  12||11  D3
        A13  10||9   D4
     A15/OUT  8||7   D5
       CRUIN  6||5   D6
     CRUCLK*  4||3   D7 
         GND  2||1   RESET (Active high)
       
              LEFT

     #  Name   I/O  Use
    --  -----  ---  --------------------------------------- 
     1  RESET   <   Resets the system (active high) 
     2  GND         Signal groud 
     3  D7      <>  Data bus, bit 7 (least significant) 
     4  CRUCLK* >   Inversion of TMS9900 CRUCLOCK pin 
     5  D6      <>  
     6  CRUIN   <   CRU input to TMS9900 
     7  D5      <>  
     8  A15     >   Address bus, bit 15 / also CRU output bit 
     9  D4      <>  
    10  A13     >   
    11  D3      <>   
    12  A12     >    
    13  D2      <>   
    14  A11     >    
    15  D1      <>   
    16  A10     >    
    17  D0      <>  Data bus, bit 0 (most significant) 
    18  A9      >   
    19  VCC         +5 Volts power supply 
    20  A8      >   
    21  GS*     >   Grom select. Active low is addr in >9800-9FFF. Internal 2.2K pull-up to +5V.
    22  A7      >   
    23  A14     >   Address bus, bit 14. Select mode: low=data / high=addr. 2.2K pull-up to +5V. 
    24  A3      >   
    25  DBIN    >   Active high = read memory. Internal 3.3K pull-up to +5V 
    26  A6      >   
    27  GRC     >   GROM clock: 447.7 kHz signal from the VPD (or 3.5 MHz, jumper-selected). 1K pull-up to +5V.
    28  A5      > 
    29  VDD         -5 Volts power supply 
    30  A4      > 
    31  GR      <   Active high = GROM ready. 4.7K pull-up. Masked by GS* in the console before it reaches the CPU.
    32  WE*     >   Active low = write enable (derived from TMS9900 WE*) 
    33  VSS         Pseudo-ground (-0.72 V see GROM page for details)
    34  ROMG*   >   Active low if addr in >6000-7FFF 
    35  GND 
    36  GND

##   Side port

                                              Peripheral connector     
                                          /                          | 
                                         /                           | 
            ____________________________/      2             44      | 
           /                                  [================]     | 
          /                                    1             43      | 
          |__________________________________________________________|

     
         TOP   REAR   BOTTOM

     AUDIOIN  44||43  -5V      
          D3  42||41  IAQ 
          D1  40||39  D2    
          D5  38||37  D0     
          D6  36||35  D4   
          D7  34||33  CRUIN
      MEMEN*  32||31  A0 
          A1  30||29  A6
        MBE*   8||27  GND   MBE* low if >4000-5FFF 
         WE*  26||25  GND     
       PHI3*  24||23  GND   
     CRUCLK*  22||21  GND   
          A2  20||19  A15/CRUOUT   
          A9  18||17  A7 
         A14  16||15  A13    
          A8  14||13  LOAD* Low => BLWP @>FFFC
       READY  12||11  A12   
          A3  10||9   DBIN   
         A11   8||7   A4     
         A10   6||5   A5     
     EXTINT*   4||3   RESET*
         SBE   2||1   +5V   SBE high if >9000-94xx

               FRONT

    #  Name   I/O  Use 
     -  ----   ---  ----------- 
     1  VCC         +5 Volts power supply 
     2  SBE     >   High if addr in >9000-94xx (sound port) 
     3  RESET*  >   System reset (active low) 
     4  EXTINT* <   External interrupt (active low) 
     5  A5      >   Address bus, bit 5 
     6  A10     >      
     7  A4      >       
     8  A11     >       
     9  DBIN    >   Active high = read memory 
    10  A3      >      
    11  A12     >     
    12  READY   <   Active high = memory is ready 
    13  LOAD*   <   Unmaskable interrupt (=> BLWP @>FFFC) 
    14  A8      >      
    15  A13     >       
    16  A14     >      
    17  A7      >       
    18  A9      >    
    19  A15     >   Address bus, lsb. Also CRU output bit. 
    20  A2      >     
    21  GND            
    22  CRUCLK* >   Inversion of TMS9900 CRUCLOCK pin 
    23  GND          
    24  PHI3*   >   Inversion of phase 3 clock 
    25  GND          
    26  WE*     >   Write Enable (derived from TMS9900 WE* pin) 
    27  GND           
    28  MBE*    >   Active low if addr in >4000-5FFF (card ROMs) 
    29  A6      >     
    30  A1      >     
    31  A0      >   Address bus, bit 0 (most significant) 
    32  MEMEN*  >   Memory access enable (active low) 
    33  CRUIN   <   CRU input bit to TMS9900 
    34  D7      <>  Data bus, bit 7 (least significant) 
    35  D4      <>     
    36  D6      <>     
    37  D0      <>  Data bus, bit 0 (most significant) 
    38  D5      <>       
    39  D2      <>      
    40  D1      <>      
    41  IAQ     >   Interrupt acknowledged by TMS9900 
    42  D3      <> 
    43  VDD         -5 Volts power supply 
    44  AUDIOIN <   To sound generator AUDIO IN pin

     

##  Keyboard connector

This connector is inside the console. Red wire is \#15, pin \#1 is
marked on the PCB.

    pin|  12    13    14    15     9     8     6 
    ---+------------------------------------------- 
     5 |   =     .     ,     M     N     / 
     4 | space   L     K     J     H     ; 
     1 | enter   O     I     U     Y     P 
     2 |         9     8     7     6     0 
     7 | fctn    2     3     4     5     1   A-lock 
     3 | shift   S     D     F     G     A 
    10 | ctrl    W     E     R     T     Q 
    11 |         X     C     V     B     Z

Note: pressing a key closes the contact between corresponding row +
column. Since there are no diodes to prevent current going backwards,
pressing 3 keys at a time often results in appearance of a "phantom" key
at the 4th corner of the square formed by these keys (e.g 8+7+3=phantom
4. The current goes pin#15 --\> key 7 --\> key 8 --\> key 3 --\> pin#7,
as if key 4 were pressed).

##   PE-Box internal slots

       LEFT   REAR   RIGHT

         +8V  1||2   +8V
         GND  3||4   READY
         GND  5||6   RESET*
         GND  7||8   (nc)
         (nc) 9||10  AUDIOIN
     RDBENA* 11||12  (high)
      (high) 13||14  (nc)
      (high) 15||16  (high)
      INTA*  17||18  (nc)
         D7  19||20  GND  
         D5  21||22  D6
         D3  23||24  D4
         D1  25||26  D2
        GND  27||28  D0
        A14  29||30  A15/CRUOUT
        A12  31||32  A13
        A10  33||34  A11
         A8  35||36  A9
         A6  37||38  A7   
         A4  39||40  A5
         A2  41||42  A3
         A0  43||44  A1
      (high) 45||46  (high) 
        GND  47||48  (high)
        GND  49||50  CLKOUT*
    CRUCLK*  51||52  DBIN
        GND  53||54  WE*
      CRUIN  55||56  MEMEN*
       -16V  57||58  -16V
       +16V  59||60  +16V

              FRONT

     #  Name   I/O   Use 
    --  -----  ---   -------------------------------------------- 
     1               +5V 3-T regulator voltage supply (about +8V) 
     2               Ditto          
     3  GND            
     4  READYA       System ready (10K pull-up to +5V) 
     5  GND           
     6  RESET*  >     System reset (active low) 
     7  GND             
     8  SCLK    nc   System clock (not connected) 
     9  LCP*    nc   CPU indicator 1=TI99 0=2nd generation (not connected) 
    10  AUDIO   <    Input audio (=AUDIOIN) 
    11  RDBENA* <    Active low: enable flex cable data bus drivers (1K pull-up) 
    12  PCBEN   H    PCB enable for burn-in (always High) 
    13  HOLD*   H    Active low CPU hold request (always High) 
    14  IAQHA   nc   IAQ [or] HOLDA (logical or) 
    15  SENILA* H    Interrupt level A sense enable (always High) 
    16  SENILB* H    Interrupt level B sense enable (always High) 
    17  INTA*   <    Active low interrupt level A (=EXTINT*) 
    18  LOAD*   nc   Unmaskable interrupt (not connected) 
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
    45  AMB     H    Extra address bit. Always High.
    46  AMA     H    Extra address bit. Always High.
    47  GND          
    48  AMC     H    Extra address bit. Always High.
    49  GND            
    50  CLKOUT* >    Inversion of phase 3 clock (=PHI3*) 
    51  CRUCLK* >    Inversion of TMS9900 CRUCLOCK pin 
    52  DBIN    >    Active high = read memory
    53  GND           
    54  WE*     >    Write Enable (derived from TMS9900 WE* pin)
    55  CRUIN   <    CRU input bit to TMS9900
    56  MEMEN*  >    Memory access enable (active low) 
    57               -12 Volts 3-T regulator supply voltage (about -16V) 
    58               Ditto 
    59               +12 Volts 3-T regulator supply voltage (about +16V) 
    60               Ditto

Notes:

Signals buffered by 74LS244 in the connection card: A0-A15, DBIN,
MEMEN\*, WE\*, CLRCLK\*, RESET\*, CLKOUT\*.

Unbuffered signals: CRUIN, INTA\*, AUDIOIN, READY.

The data bus is buffered by two 74LS245 (one at each end of the cable),
activated by the card with by RDBENA, the direction is set by DBIN.

All signals must be re-buffered on each card.

Always High lines (AMA, AMB, AMC, SENILA\*, SENILB\*, PCBEN, HOLD\*) are
pulled up to +5 Volts by 47 Ohms resistors.

The light on the connection card is turned on by DBIN (High = on).
 

 
 
 

## Floppy drive controller connector

        FRONT    UP    REAR  
              +-----+ <-- Male 34-pin connector for internal drive
          nc  |34 o |========  <-- Card edge connector for external drives
        SIDE  |32 o |========              
       RDATA  |30 o |========        
    WPROTECT  |28 o |========          
      TRACK0  |26 o |========            
       WGATE  |24 o |========            
       WDATA  |22 o |========             
        STEP  |20 o  ========              
         DIR  |18 o  ========    
      STROBE  |16 o  ========  
        SEL3  |14 o |========  
        SEL2  |12 o |======--  __ notch
        SEL1  |10 o |======--  
       INDEX  |8  o |========  
        SEL4  |6  o |========     
          nc  |4  o |========      
          nc  |2  o |========            
              +-----+ 
              BOTTOM

    #  I/O  Name            Function
    -- ---- --------------- ---------
     2         ?             (used on PCs, for 1.44 MB drives)
     4         -             not used
     6       SEL4            Select DSK4 (not on original TI FDC)
     8  <    INDEX           Index pulse (hole in floppy detected)
    10  >    SEL1            Select DSK1
    12  >    SEL2            Select DSK2
    14  >    SEL3            Select DSK3
    16  >    STROBE          Motor on
    18  >    DIR             Direction to step at (0=out, 1=in)
    20  >    STEP            Step by one track
    22  >    WDATA           Data output
    24  >    WGATE           Enable data output
    26  <    TRACK0          Track 0 reached
    28  <    WPROTECT        Write protection detected
    30  <    RDATA           Data input
    32  >    SIDE            Side selection
    34        -              not used

    All odd numbered pins are connected to ground

##  PIO connector

    Male 16-pin connector (bottom view)

            LEFT   FRONT    RIGHT
                  +-----+
             GND  |16 15|  1K +5V 
        SPAREOUT  |14 13|  SPAREIN        
      +5V 10 Ohm  |12 11|  GND             
     HANDSHAKEIN  |10 9    D0     
              D1  | 8 7    D2 
              D3  | 6 5 |  D4 
              D5  | 4 3 |  D6 
              D7  | 2 1 |  HANDSHAKEOUT    
                  +-----+
                   REAR

    #  I/O  Name            Function
    -- ---- --------------- ---------
     1  >   HANDSHAKEOUT    User defined (syncronisation signal)
     2  <>  D7              Data byte (least significant bit)
     3  <>  D6 
     4  <>  D5 
     5  <>  D4 
     6  <>  D3 
     7  <>  D2 
     8  <>  D1 
     9  <>  D0              (most significant bit) 
    10  <   HANDSHAKEIN     User defined (syncronisation signal) 
    11      GND             Ground
    12  >   +5V via 10 Ohm  User defined
    13  <   SPAREIN         User defined
    14  >   SPAREOUT        User defined
    15  >   +5V via 1 KOhm  User defined
    16      GND             Ground

##   RS232C connector

     # I/O  Name   Use
    -- ---- ------ ----------------
     1      GND    Ground
     2  <   RD-1   Data input
     3  >   TX-1   Data output
     4  nc 
     5  >   CTS-1  Clear to send
     6  >   DSR    Data set ready. Common pin, always high (+12V via 1.8 KOhm) 
     7      GND    Ground 
     8  >   DCD-1  Data carrier detected
     9  nc
    10  nc 
    11  nc         (may replace 11: jumper selected)
    12  >   DCD-2  Data carrier detected  
    13  >   CTS-2  Clear to send 
    14  <   RD-2   Data input
    15  nc
    16  >   TX-2   Data output
    17  nc
    18  nc         (may replace 19: jumper selected)
    19  >   DTR-2  Data terminal ready (to DSR* and CTS* pins) 
    20  <   DTR-1  Data terminal ready (to DSR* and CTS* pins)
    21  nc
    22  nc
    23  nc
    24  nc

Notes

Voltages are in the range -12V to +12V.

The -1 pins correspond to RS232/1 (or RS232/3 if two cards are
installed), the -2 pins to RS232/2 (or RS232/4).

Some cards have jumpers to direct the DTR lines to either pin 19 or 18
and either pin 20 or 11.
Revision 1. 3/24/99 OK to release

Revision 2. 5/2/00 Added FDC connector

Revision 3. 5./25/01 Updated GROM port
Revision 4. 5/14/02 Correction: SBE is active high
Revision 5. 11/20/02 Added chip links

[Back to the TI-99/4A Tech Pages](titechpages.md)
