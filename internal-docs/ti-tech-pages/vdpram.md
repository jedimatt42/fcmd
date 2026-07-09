# The Videoprocessor RAM

[Pinout
](#Pinout)[Operation
](#Operation)[Timing diagrams](#Timing%20diagrams)
[Electrical characteristics](#Electrical)

[Contents](#Contents)

##  Introduction

The VDP RAM is implemented with eith TMS4116, 16384 x 1 bit NMOS RAM
chips. These are dynamic RAMs, which means that they must be constantly
refreshed, less they would forget data. Fortunately, the VDP takes care
of this so no extra circuitery is needed.

Eight chips add up to 16 Kbytes. Apart for the 256 bytes of (expensive)
SRAM known as the scratch-pad, at address \>8300-83FF, the VDP RAM is
the only RAM available in the TI-99/4A console.

I don't have data sheet for the Texas Instruments chips, the info
provided here is taken from the Mitsubishi equivalent chip: M5K4116P.

##  Pinout

         +----+--+----+
     Vbb |1 o       20| Gnd
     DIN |2    T    19| CAS*
    R/W* |3    M    18| DOUT
    RAS* |4    S    17| A6
      A0 |5    4    16| A3
      A2 |6    1    15| A4
      A1 |7    1    14| A5
     Vdd |8    6    13| Vcc
         +------------+

Power supply
**Vcc**: +5V
**Vdd**: +12V
**Vbb:** -5V
**Gnd**: ground

Address + data buses
**A0-A6**: Address bus (multiplexed: 2 x 6 bits)
**DIN**: Data bit input
**DOUT**: Data bit output

Control bus
**RAS\***: Row address select
**CAS\***: Column address select
**R/W\***: H=read, L=write

## Operation

A 14-bit address must be latched in two operations, through the A0-A6
lines. The first 7 bits (row number) are latched when RAS\* is brought
low, the last 7 bits (column number) with CAS\* is brought low (the
CAS\* is internally inhibited until the chip is ready to set the column
address).

The data present on DIN is written when both R/W\* and CAS\* are low.
Whichever of the two becomes low last determines the writing operation.

For read operations, the data bit will be presented on DOUT when CAS\*
goes low after the address has been set. It will remain unchanged until
CAS\* goes high, irrespective of RAS\*. When CAS\* is high, DOUT is in
high-impedance state.

This allow for a chip-select mechanism that can use either RAS\* or
CAS\*. Decoding RAS\* reduces the power dissipation as CAS\* will be
ignored in the non-selected chips.

The 4116 must be refreshed by presenting all 128 row addresses on A0-A6
and pulsing RAS\* low, every 2 msec. A normal memory cycle will also
perform refreshing of this particular row, but it uses more power than a
RAS\*-only cycle. This is because most of the power is dissipated when
addresses are latched.

At power-up, it is recommended that Vbb is applied first (and removed
last) and that it never goes above Gnd while Vdd is powered. Some eight
dummy cycles are necessary after power is on before the chip is ready to
operate.

##  Timing diagrams

I don't have the datasheet for the TI chips. These diagrams come from
the Mitsubishi equivalent: M5K4116P,S-3 (200 ns).

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

I don't have the datasheet for the TI chips. These are the
characteristics of the Mitsubishi equivalent: M5K4116P.

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



##  Contents

Being RAM, the VDP memory can contain any kind of data, depending on the
program currently running. Of course, the VDP will need some space, but
the rest is available to the user, through the VDP. This has a major
incovenient: is it slow since the VDP only answers memory requests when
it's not too busy drawing the screen. But that was a cheap way to
implement 16K of RAM into a home computer: a dedicated CPU RAM would
have required a refresh circutery, or the use of expensive SRAMs.

[VDP
](#VDP%20map)[Console ISR
](#ISR%20map)[Floppy disk controller
](#FDC%20map)[TI-Basic
](#Basic%20map)[Extended Basic
](#XB%20map)[Editor/Assembler
](#Ed/Asm%20map)[TI-Writer
](#Writer%20map)[Mini-memory](#Minimem%20map)

### Video-processor

The TMS9918a video-processor expects to find data tables in the VDP RAM,
that describes what to displays on screen. These tables are built by the
CPU, which accesses the VDP RAM through the VDP itself. The position of
each table is specified in a dedicated register that is part of the VDP.
Since these registers can only contain numbers from 0 to 255, the tables
have to be aligned on a multiple of their length (e.g. on a \>800
boundary : \>0000, \>0800, \>1000, etc).

#### Screen image table

This table contains the code of the caracter to be displayed for each
screen position. In all video modes except text mode, the screen is 24
lines x 32 columns, so the table is \>0300 bytes in length. It must be
aligned on a \>0400-byte boundary.

#### Character pattern table

This table contains the bit pattern to be displayed for each character
code. It must be aligned on a \>800-byte boundary. It is \>1800 bytes
long in bitmap mode, \>800 bytes in all other modes.

#### Color table

This table defines the color to be used to display a given character. It
must be aligned on a \>40-byte boundary. Its size varies from \>20 bytes
in standard mode to \>1800 bytes in bitmap mode. It is not used in
multicolor mode, nor in text mode.

#### Sprite attributes table

This table defines the location, pattern code, and color of upto 32
sprites. Sprites are special characters that can be overlayed on the
screen. They have a transparent background and are not limited to
row/column positions: instead they use a coordinates system of 192 rows
x 256 columns. The sprite attribute table is \>80 bytes in lenght and
must be aligned on a \>80-byte boundary. It is not used in text mode.

#### Sprite pattern table

This table define the pattern to be used for each sprite code. It is
\>800 bytes in length and must be aligned on a \>800-byte boundary. It
is not used in text mode.

See my page on the [TMS9918a VDP](tms9918a.md) for more details on the
structure of these tables. See below for the location of the respective
tables with a some important TI cartridges.

###  Console interrupt routine

The interupt service routine (ISR) located in the console ROMs can use
the VDP memory for two purposes: automatic sprite motion and automatic
sound playing. This only occurs for VDP-triggered interrupts, i.e. 60
times per second (50 in Europe). Of course, it implies that VDP
interrupts have been allowed (by setting a bit in VDP register 1) and
that interrupts in general are enabled by the program in control (with a
LIMI 2 instruction).

#### Sprite auto-motion

The ISR expects the sprite attributes table to be located at address
\>0300. It also requires a sprite motion table to be placed at address
\>0780. This table is \>80 bytes in length (at max, actually 4 bytes per
sprite but not all sprites need to move).

Some key bytes in the scratch-pad RAM control this feature:

\>837A: Number of the highest sprite in auto-motion
\>83C4: Bit 0 (weight \>80) and 1 (weight \>40) disable auto-motion.

#### Sound playing

The ISR can play a list of sounds stored either in the VDP RAM or in
GROM/GRAM memory.

Some key bytes in the scratch-pad RAM control this feature:

\>83CC-83CD: Address of the sound list.
\>83FD Bit 15 (weight \>01) is set if the sound list is in VDP RAM.
\>83CE: Bytes remaining to play
\>83C4: Bit 0 (weight \>80) and 2 (weight \>20) disable sound playing.

###  TI floppy disk controller card

The floppy disk controller (FCD) card reserves some room in the VDP
memory at power-up time, for disk buffering purposes. The amount of
memory reserved can later on be expanded or diminished by calling
subprogram FILES from Basic, or subprogram \>16 from assembly.

Normally, the reserved area will be located at the top of the VDP RAM,
but provision is made to use several controllers, which each will have a
reserved area. Therefore, each area begins with a 4-byte header that
allows to link on the next one. The bottom area is pointed at by bytes
\>8370-8271 in the scratch-pad memory:

    >AA      Validation code to identify a reserve area
    >11      CRU of the controller card that owns it (here >1100)
    >3FFF    End of the reserved area (here: end of the VDP RAM)

Then the FDC reserves 518 bytes for each file that can be open
simultaneously. By default this is three, but it can be bumped up to 9
in Basic and to 16 in assembly. Therefore, the location of the header
can vary. Assuming no other card has reserved memory on top of the VDP
RAM, the FDC-reserved area can begin at:


The FDC also reserves a 252-byte stack that its DSRs and subprograms use
to push return adresses and save register values.

Finally, there is a 256-byte buffer that generally holds sector 0 of the
disk directory, and then some buffered variables (most notably, the name
and drive number of the last file accessed).

Here is a exemple of the area reserved by the FDC after a normal
power-up (number of files = 3):


5
Buffer area header
6
File control block
for first file
first file
to signal a modified FDR).
for first file
for second file
second file
for second file
for third file
third file
for third file
4
Disk drive info
copied from sector 0
11
Filename compare

###  TI-Basic

VDP tables
Screen image : \>0000-02FF
Char pattern : \>0000-07FF
Color table: \>0300-031F
Sprite attributes: \>0300-0301 (a single byte \>D0 disables all
sprites).
Sprite patterns: \>0000 (not used)

Yep, that's right: the character pattern table overlaps with the screen
image and the color table! This means that we cannot use characters
0-110. But then again we can, so there must be a trick here. There is
indeed: before putting a character code in the screen image, TI-Basic
adds 96 (hex \>60) to its ascii code. This means that we can use
characters 28 to 159: they will be automatically translated into
characters 124 to 255 for display purposes, so that the only the end of
the pattern table is ever used (\>03E0-07FF). This oddity was
implemented to save space in the VDP RAM, since this is where the Basic
program and all its variables will be stored.

TI-Basic reserves several memory buffers for its own purpose, in the VDP
RAM. For most of these buffers, the address is variable and is stored in
some key locations in the scratch-pad RAM (two bytes each).

Crunch buffer (to translate a line into "tokens"): \>0370-03C0
VDP roll-out area (to save temporary data): \>03C0-03DF

Value stack: bottom in \>8324, top in \>836E.
PAB linked list: first link in \>833C.
String space: bottom in \>831A, top in \>8318
Symbol table: bottom in \>833E
Line number table: bottom in \>8330, top in \>8332
Statements: top in \>8370

#### VDP RAM map with TI-Basic

Here is a summary of the VDP RAM usage in TI-Basic. For more
information, see my [TI-Basic page](basic.md).

    >0000 +-------------------------+ <--- VR2=F0, VR4=F9, VR6=0
          | Screen image table      |
          | Also char patterns table|
          | (not used, due to bias) |
    >0300 +-------------------------+ <--- VR3=0E, VR5=86
          | Color table (unused)    |
    >030F | Color table (used)      |
    >0320 +-------------------------+
          | Crunch buffer           |
    >0370 +-------------------------+
          | Char patterns >0E-18    |
    >03C0 +-------------------------+
          | VDP roll-out area       |
    >03E0 +-------------------------+
          | Char patterns >1C-9F    |
          | (chars >A0-FF not used) |
          | (due to bias of >60   ) |
    >0800 +-------------------------+
          | Value stack             | <--- @>8324
          |                         | <--- @>836E
          | PAB list                | <--- @>833C
          |                         |
          | String space            | <--- @>831A
          |                         | <--- @>8318
          | Symbol table            | <--- @>833E
          |                         |
          | Line number table       | <--- @>8330
          |                         | <--- @>8332
          | Statement list          |
    >37D8 +-------------------------+ <--- @>8370
          | File buffers  (3 files) |
    >3FFF +-------------------------+ VR0=0, VR1=E0, VR7=F7
###  Extended Basic cartridge

VDP tables
Screen image : \>0000-02FF
Char pattern : \>0000-07FF
Color table: \>0800-081F
Sprite attributes: \>0300-036F
Sprite patterns: \>0000-07FF

Sprite motion table: \>0780-07FF

Just as in Basic, the character pattern table overlaps other tables.
Extended Basic automatically adds \>60 to the ascii code of any
character to be displayed on screen, effectively limiting the use of the
pattern table to \>03F0-07FF. Note however that the sprite motion table
used by the ISR also overlaps with the pattern table and prevents us
from using the last 16 characters. All in all, only characters 30 to 143
can be used in Extended Basic.

Also note that the sprite pattern table completely overlaps the
character pattern table: so sprites can only have a pattern that was
assigned to a character. We are so used to this that most of us don't
realise it does not have to be so: the Editor/Assembler cartridge, for
instance, allows sprite patterns that differ from character patterns.

Extended Basic also uses the VDP memory to save many variables, its
stack, several buffers and part of the program. If a memory expansion
card is installed, most of the Extended Basic program will be stored in
the high memory expansion (\>A000-FFFF), which frees a lot of room in
VDP RAM. The top of the reserved area in the high memory expansion is
pointed at by word \>8384, the bottom by word \>8386 in the scratch-pad.
If the expansion is present, byte \>8389 will contain \>E7 while the
program is running.

Interpreter variables (XB internal use): \>0370-03BF
Roll-out buffer (to save the scratch-pad): \>03C0-03EF
Crunch buffer (to translate a line into "tokens"): \>0820-08BE
Edit/Recall buffer (holds the command line): \>08C0-0957

Value stack: bottom in \>8324, top in \>836E.
PAB linked list: first link in \>833C.
String space: bottom in \>831A, top in \>8318
Symbol table: bottom in \>833E. In memory expansion, if present (numeric
values only).
Line number table: bottom in \>8330, top in \>8332. In memory expansion:
bottom also in \>8386.
Statements: top in \>8370. In memory expansion: top in \>8384.

#### VDP RAM map with Extended Basic

    >0000 +-------------------------+ <--- VR2=00, VR4=00, VR6=00
          | Screen image table      |
          | Also char + sprite      |
          | (not used) patterns     |
    >0300 +-------------------------+ <--- VR5=06
          | Sprite attributes table |
    >0370 +-------------------------+
          | XB system area          |
    >03F0 +-------------------------+
          | Char patterns >2E-8F    |
          | Also sprite patterns    |
    >0780 +-------------------------+
          | Sprite motion table     |
    >0800 +-------------------------+ <--- VR3=20
          | Color table             |
    >0820 +-------------------------+
          | Crunch buffer           |
    >08C0 +-------------------------+
          | Edit/recall buffer      |
    >0958 +-------------------------+ <--- VR4=01
          | Value stack             | <--- @>8324
          |                         | <--- @>836E
          | PAB list                | <--- @>833C
          |                         |
          | String space            | <--- @>831A
          |                         | <--- @>8318
          | Symbol table            | <--- @>833E (if no mem exp)
          |                         |
          | Line number table       | <--- @>8330 (if no mem exp)
          |                         | <--- @>8332
          | Statement list          |
    >37D8 +-------------------------+ <--- @>8370
          | File buffers  (3 files) |
    >3FFF +-------------------------+ VR0=0, VR1=E0, VR7=07

###  Editor-Assembler cartridge

VDP tables
Screen image : \>0000-02FF
Char pattern : \>0800-0FFF
Color table: \>0380-039F
Sprite attributes: \>0300-037F
Sprite patterns: \>0000-07FF

Here also, there is some overlap: the sprite pattern table overlaps
three other tables. Concretely, this means that we are limited to
sprites with a pattern code of \>80-F0 (their patterns will be at
\>0400-077F).

The cartridge also uses the VDP area to store PABs for file access, and
to load the file themselves.

PAB for Editor, Assembler, edited/printed file, assembly source file,
loaded file: \>1000-1018
PAB for assembly object file, printer: \>1100-1118
PAB for assembly list file: \>1200-1218
PAB for file copied during assembly (?): \>1300
Assembly source file, edited/printed file: \>1080-10CF
Assembly code file data, printer data: \>1180-11CF
Assembly list file data: \>1280-12CF
Program files (Editor, Assembler, user-defined): \>1380-157F

Of course, all subprogram that are callable from TI-Basic expect the
Basic VDP RAM organisation.

#### VDP RAM map with Editor/Assembler

    >0000 +-------------------------+ <--- VR2=00, VR6=00
          | Screen image table      |
          | (also sprite patterns)  |
          |                         |
    >0300 +-------------------------+ <--- VR5=06
          | Sprite attributes table |
    >0380 +-------------------------+ <--- VR3=0E
          | Color table             |
    >03A0 +-------------------------+
          |        (free)           |
    >03C0 +-------------------------+
          | VDP roll-out area       |
    >0400 +-------------------------+
          |                         |
          | Sprite patterns >80-F0  |
          |                         |
    >0780 +-------------------------+
          | Sprite motion table     |
    >0800 +-------------------------+ <--- VR4=01
          |                         |
          | Character pattern table |
          |                         |
    >1000 +-------------------------+
          | PAB for most files      |
    >1080 +-------------------------+
          | Data are for above PAB  |
    >10D0 +-------------------------+
          |        (free)           |
    >1100 +-------------------------+
          | PAB for object files    |
    >1180 +-------------------------+
          | Data are for above PAB  |
    >12D0 +-------------------------+
          |        (free)           |
    >1300 +-------------------------+
          | PAB for print files     |
    >1380 +-------------------------+
          | Data area for above PAB |
    >15D0 | and for "program" files |
          |                         |
    >3580 +-------------------------+
          |        (free)           |
    >35D2 +-------------------------+ <--- @>8370
          | File buffers (4 files)  |
    >37D8 +-------------------------+ <--- @>8370
          | File buffers (3 files)  |
    >3FFF +-------------------------+ VR0=0, VR1=E0, VR7=F5

###  TI-Writer cartridge

VDP tables
Screen image : \>0000-02FF
Char pattern : \>0000-07FF
Color table: \>0300-031F
Sprite attributes: none
Sprite patterns: none

The cartridge calls subprogram \>16 in the FDC card to change the number
of files (and hence the size of the area reserved at the end of the VDP
RAM): 1 files with the editor, 4 files with the formater, and 3 files
with the utility.

It uses a single PAB at \>0680, with a data area at \>1000. The formater
sets its own PABs, but I don't know where.

#### VDP RAM map with TI-Writer

    >0000 +-------------------------+ <--- VR2=00, VR6=00
          | Screen image table      |
          |                         |
          |                         |
    >0300 +-------------------------+ <--- VR5=06
          | Sprite attributes table |
    >0380 +-------------------------+ <--- VR3=0E
          | Color table             |
    >03A0 +-------------------------+
          |                         |
          | Sprite patterns >74-F0  |
          |                         |
    >0780 +-------------------------+
          | Sprite motion table     |
    >0800 +-------------------------+ <--- VR4=01
          |                         |
          | Character pattern table |
          |                         |
    >1000 +-------------------------+
          |                         |
          | Data area for PAB       |
          |                         |
    >3000 +-------------------------+
          |        (free)           |
    >35D2 +-------------------------+ <--- @>8370
          | File buffers (4 files)  |
    >37D8 +-------------------------+ <--- @>8370
          | File buffers (3 files)  |
    >3BE4 +-------------------------+ <--- @>8370
          | File buffers (1 file)   |
    >3FFF +-------------------------+ VR0=0, VR1=E0, VR7=F5

###  Mini-memory cartridge

VDP tables
Screen image : \>0000-02FF
Char pattern : \>0800-0FFF
Color table: \>0380-039F
Sprite attributes: \>0300-0380
Sprite patterns: \>0800-0FFF

Just like the Editor/assembler cartridge. Here also, the sprite pattern
table overlaps three other tables, which limitates the use of sprite
patterns to \>74-FF (or even \>74-F0 if we plan to use sprite
auto-motion). Of course, the Mini-memory itself does not use sprites,
but this is the VDP structure that we inherit when we start a program
with Mini-memory.

The subprograms and DSRs called from Basic expect of course the Basic
VDP structure. Most notably, their PABs are created in the string space,
by asking Basic for the adequate space.

#### VDP RAM map with Mini-memory

    >0000 +-------------------------+ <--- VR2=00, VR6=00
          | Screen image table      |
          | (also sprite patterns)  |
          |                         |
    >0300 +-------------------------+ <--- VR5=06
          | Sprite attributes table |
    >0380 +-------------------------+ <--- VR3=0E
          | Color table             |
    >03A0 +-------------------------+
          |                         |
          | Sprite patterns >74-F0  |
          |                         |
    >0780 +-------------------------+
          | Sprite motion table     |
    >0800 +-------------------------+ <--- VR4=01
          |                         |
          | Character pattern table |
          |                         |
    >1000 +-------------------------+
          | PAB for most files      |
    >1080 +-------------------------+
          | Data are for above PAB  |
    >1100 +-------------------------+
          | Data for CS1 in Eazybug |
    >1140 +-------------------------+
          |                         |
          |        (free)           |
          |                         |
    >37D8 +-------------------------+ <--- @>8370
          | File buffers (3 files)  |
    >3FFF +-------------------------+ VR0=0, VR1=E0, VR7=F5

[Back to the TI-99/4A Tech Pages](titechpages.md)
