# The german 128K Gram Karte

This card and a sister 512K card, were produced around 1986 by:

    Elektronik-Service
    Peter Kleinschmidt
    Linning 37
    D-4044 Kaarst Germany
    Phone: 02101/603208

The software in the card ROM was written by Heiner Martin (the guy who
wrote the "TI-99/4A Intern" book).

[Overview](#overview) 
[Glossary](#Glossary)

**Card structure**
[CRU map](#CRUmap)
[GRAM access](#Gram%20access)
[RAM access](#Ram%20access) 
[Bank switching](#Bank%20switching)
[RAM vs GRAM correspondance](#RAM-GRAM)
[DIP switches settings](#DIP%20switches)

**The card ROM**
[Power-up routine](#Card%20ROM)
Subprograms
[ EDITMEM](#Suprograms)
 [MODULE](#call%20module)
[ GRAM](#call%20gram)
[Programs and TI main menu](#main%20menu)

[Loader file formats](#File%20formats)

##  Overview

The Gram Karte is a GROM emulator: it allows to copy cartridges on
floppies and to load them in the card where they will execute normally.

It is controlled via the CRU and its CRU address can be set to any value
with DIP switches. Usually, it is \>1700.

The card has an 8K ROM, mapping at \>4000-5FFF, with the several DSR,
subroutines and programs. But most importantly, it has 128K of RAM.

The first 64K of RAM are used to emulate GROMs at access port \>9800
(the port is also selected with a DIP switch). Note that the emulation
covers the whole range of GROM addresses, from \>0000 to \>FFFF, which
means there are some differences with cartridges:

First, when address \>x7FF is accessed, the internal memory pointer
moves to \>x800 and does not loop back to \>x000 as with TI GROMs (the
same happens at address \>xFFF: the pointer moves to the next "GROM").

Second: the card conflicts with the console GROMs at addresses
\>0000-17FF, \>2000-37FF and \>4000-57FF. The user manual states that
the card output drivers are more powerfull and should be able to
override the GROMs. There is a possiblity that this would damage the
GROMs, although the authors did not experienced such a problem... A CRU
bit is used to toggle the override on/off, another bit toggles the whole
memory range on/off.

The second 64K can be used in two ways.

First as an additional GRAM memory, accessed via port \>9820 (i.e \>0020
apart from the DIP switch-selected port).

Second, as RAM memory mapping in the cartridge ROM space, at addresses
\>6000-7FFF in CPU memory. A CRU bit is provided to write-protect this
area, in order to better emulate cartridges ROMs. A convention for
cartridges, is that the memory block \>7000-7FFF can be toggled between
two different pages, by writing to bytes \>6000 (for page 0) or \>6002
(for page 1). The Gram Karte respects this convention and expands it to
16 pages, selected by writing to corresponding bytes in the range
\>6000-601E. A CRU bit allows to toggle switching on/off.
Note that the second 64K can be accessed both as RAM and as GRAM, if it
is necessary. The correspondence between RAM pages and GRAM is designed
in such a way that is it possible to have a small module loaded in GRAM
at \>9820, while still having enough memory for RAM.

##   Glossary

The following is a list of terms I'll be using throughout this document.
In most cases, these are "official" definitions, but some (like the
distinction between cartridges and modules, or pages and banks) are my
own and are only meant to facilitate the discussion.

If you are not familiar with the concepts of GROM, GRAM and cartridges
bank, have a look at the [page](architec.md) that describes the console
architecture.

**GROM**

Graphic Read Only Memory. A special kind of ROM, that can be read
serially, one byte at a time. It has an internal address pointer,
incremented after each access. The standard GROM size is \>1800 bytes
(6K), but there are some rare OK GROMs (\>2000 bytes).

**GRAM**

This is RAM memory with a external electronic circuit that emulates the
function of a GROM, but also allows for writing in it.
**GROM port**

A group of 4 addresses in CPU memory, through which a GROM (or GRAM) can
be accessed. The 4 addresses are organised as follow:
\>98x0 Read data: bytes can be read one by one from this address.
\>98x2 Read address: the GROM internal memory pointer can be retreived
from this address, as two byte (MSB comes first). This is the address of
the next byte to be read, *plus one* (to find current address, DECrement
the data word).
\>9Cx0 Write data: bytes can be writen to a GRAM (but not to a GROM) by
passing them one by one at this address.
\>9Cx2 Write address: the internal memory pointer can be set to any
value (0 to \>FFFF), by passing it as two bytes (MSB first) at this
address.
**GROM base**

The lowest address in a GROM port (the one used to read data).
Theoretically, there can be bases from \>9800 to \>9BFC by increments of
\>0004. However, the routines in the console ROMs will only access the
first 16 bases: \>9800, \>9804, \>9808,... \>981C. Since each port gives
access to 64K of GROM/GRAM, 16 bases correspond to 1 megabyte!
**Console GROMs**

There are three 6K GROMs installed in the TI-99/4A console. They contain
the power-up program, several math subroutines, and the Basic
interpreter. Their addresses are \>0000-17FF, \>2000-37FF and
\>4000-57FF. They can be reached from any GROM port.
**Cartridge GROMs**

GROMs with addresses \>6000 and above, that can be installed in a
cartridge. They can be reached from any GROM port unless the cartridge
contains a port selection circuit.
**Cartdridge ROM area**

The CPU memory area between \>6000 and \>7FFF is available for use by
cartridges. Most cartridges have a 8K ROM here, except the Mini-Memory
that has a non-volatile RAM in the second half (\>7000-7FFF). If a
cartridge needs more cpu memory, the 2nd half of the space can be paged.
**Cartridge**

Aka "Solid state software", cartridge is plugged in the dedicated port
in the front of the console. It can contain upto 5 GROMs with addresses
\>6000-7FFF, \>8000-9FFF, \>A000-BFFF, \>C000-DFFF and \>E000-FFFF.
These GROMs generally answer to every GROM base. A cartridge can also
contain ROM or RAM to appear in the \>6000-7FFF range in cpu memory.
**GRAM card**

A device that emulates one or more cartridges, by providing GRAM and
RAM. To emulate more than one cartridge, a gram card must discriminate
between the different GROM bases.
**Module**

A program installed either in a cartridge or in a GRAM card. A module
should contain a standard header, with a list of its contents: programs,
DSR, subprograms, or power-up routines.
**Bank**

A range of memory addresses that can be switched (by the software)
between several memory pages. In cartridges, there is a ROM bank at
addresses \>6000-7FFF that can display one of two 4K ROM pages. The Gram
Karte uses the same bank to display one out of 16 RAM pages.
**Page**

A block of memory that can be made to appear in a bank. The actual
physical address of this memory is irrelevant to the user, as it is
dealt with by the electronics.
**Bank switching** (aka page selection)

The operation that consists in selecting the page that must appear in a
bank. For cartridges, this is done by wrinting to addresses \>6000 (for
page 0) and \>6002 (for page 1). Since this space is supposed to contain
ROM, there is no other reason to write to it than to switch banks. The
Gram Karte expands the convention to 16 pages and uses 16 selection
addresses from \>6000 to \>601E.

##  Card architecture

### CRU map

The Gram Karte has four DIP switches that allows to install it at any
CRU address in the range \>1000-1F00. Just compose the second nibble of
the CRU address in binary format on the switches S1 (msb) to S4 (lsb):
ON=0 and OFF=1 (e.g. \>1700 = ON OFF OFF OFF). Be carefull not to
conflict with another existing card...
 


1: Default bank appears (DIP selected 1-4)

####   GRAM access

If GRAM memory is enabled, it can be accessed the usual way, through the
GRAM access ports:

\>9800 \>9820: Read a byte from GRAM/GROM, then increments the memory
pointer.
\>9802 \>9822: Read the address pointer, as two bytes. It points at the
next byte to be read/written *plus one*. The same pointer works for both
ports.
\>9C00 \>9C20: Write a byte to GRAM, then increments the memory
pointer.
\>9C02 \>9C22: Set the memory pointer, passed as two bytes (MSB first).

The value of the GRAM bases can be modified with 8 DIP-switches, but the
two ports are always \>0020 apart. All first 16 bases can be selected.
This allows to have multiple Gram Karte installed in the PE-box, without
conflict (supposedly, the 512K card implements 4 pairs of ports, so two
such cards would cover the whole range of addressable GRAMs).
|         |        |        |
|---------|--------|--------|
| Port 1  | Port 2 | Switch |
| \>9800  | \>9820 | 1      |
| \>9804  | \>9824 | 5      |
| \>9808  | \>9828 | 2      |
| \>980C  | \>982C | 6      |
| \>9810  | \>9830 | 3      |
| \>9814  | \>9834 | 7      |
| \>9818  | \>9838 | 4      |
| \>981C  | \>983C | 8      |

NB: Only one switch at a time can be ON, all others must be OFF.
You should always have one card using base \>9800, since it's the only
one that will have RAM banks.

By default, GRAM memory is readable in the range \>6000-FFFF (in both
ports). Three CRU bits can modify this situation:

CRU bit 5: when 1, enables the lower address range \>0000-5FFF for
reading in both ports, thus overriding the console GROMs.

CRU bit 3: when 1, disables the whole range \>0000-FFFF for reading.
This is anti-intuitive, but necessary since all bits are reset as 0 at
power-up and the GRAM must be on by default. NB It is NOT possible to
write-protect GRAM memory: even if it is not readable, the memory can
still be written to!

CRU bit 2: when 1, disables GRAM access via port \>9820. This is
critical when using that memory as RAM: a standard header in a page may
cause havoc if it appears in GRAM.

 

#### RAM access

The RAM at \>6000-7FFF can be accessed normally, provided it is enabled.
Two CRU bits control its enabling:

CRU bit 1: when 0, enables access to the RAM.

CRU bit 4: when 1, enables the RAM for writing. Thus the RAM is
write-protected by default, which is needed since it is supposed to
emulate ROM (only the mini-memory module has RAM in here).

####  Bank switching

This is a more complicated issue. First of all, the memory block
\>6000-6FFF is not switchable: page 14 always appears in it. The only
switchable block is \>7000-7FFF. It is 4K in lenght, therefore  64K of
memory will provide 16 pages that can appear in this bank. Selecting a
page is performed by writing to the \>6000-7FFF area. It is advisable to
write protect it before switching, so as not to modify its content (MOVB
a byte to itself is not guarantied to work as even the MOVB operation
rewrites the low byte, and this may switch banks before the high byte is
written).

Writing to \>6000 (and \>6020, \>6040,...\>7FE0) selects bank 0.
Writing to \>6002 (\>6022, etc) selects bank 1.
Writing to \>6004 selects bank 2, etc.

Two CRU bits control switching operations:

CRU bit 7: when 0, enables switching (thus switching is on by default).
When 1, writing to \>6000-7FFF will not switch the bank.

CRU bit 6: when 0, the selected page appears at \>7000-7FFF. When 1, a
default page appears. The default page is selected among 4 possible
pages via two DIP switches on the card:

DIP switches
Bank

**In summary**, this is how to switch banks:

* It assumes R0 contains a page number (0-15)`
 SELPAG LI    R12,&gt;1700        CRU address of Gram Karte (DIP switch selected)
        SBZ   1                Enable &gt;7000-7FFF bank
        SBZ   4                Write protect it
        SBZ   7                Bank switching on
        SBZ   6                Selected page appears in bank
        SLA   0,1              As switching addresses grow by 2
        AI    R0,&gt;6000         Coin switching address
        MOVB  *R0,*R0          Switch     
        SBO   7                Switching off (optional)
        SBO   4                Enable writing (optional)
        B     *R11

 

###  Correspondence between RAM pages and GRAM memory at base \>9820 

|        |                   |                  |
|--------|-------------------|------------------|
| Page   | Switching address | Location in GRAM |
| 0 \$   | \>6000            | \>E000           |
| 1 \$   | \>6002            | \>F000           |
| 2 \$   | \>6004            | \>C000           |
| 3 \$   | \>6006            | \>D000           |
| 4      | \>6008            | \>A000           |
| 5      | \>600A            | \>B000           |
| 6      | \>600C            | \>8000           |
| 7      | \>600E            | \>9000           |
| 8      | \>6010            | \>6000           |
| 9      | \>6012            | \>7000           |
| 10     | \>6014            | \>4000           |
| 11     | \>6016            | \>5000           |
| 12     | \>6018            | \>2000           |
| 13     | \>601A            | \>3000           |
| 14  \* | \>601C            | \>0000           |
| 15     | \>601E            | \>1000           |

\* This is the page that appears at \>6000-6FFF (non-switchable bank).
\$ The default page appearing when CRU bit 6=1 is selected among those
four, with two DIP-switches.

###   DIP switches

The 128K Gram Karte has 3 blocks of 8 DIP switches on board.

 +--------------------------------------------+
 |                                            |
 |                                            |           DIP 1
 |                                            |          +-------------------+
 |                                            |          | +-+-+-+-+-+-+-+-+ |
 |                                            +---+      | |1|2|3|4|x|x|7|8| |
 |                                             ===|      |o+-+-+-+-+-+-+-+-+ |
 |                                             ===| C    +-------------------+
 |                                             ===| o
 |                                             ===| n    DIP 2
 |                                             ===| n    +-------------------+
 |                                             ===| e    | +-+-+-+-+-+-+-+-+ |
 |                                             ===| c    | |1|2|3|4|5|6|7|8| |
 |                  ____ DIP1                  ===| t    |o+-+-+-+-+-+-+-+-+ |
 |                 |.___|                      ===| o    +-------------------+
 |                                             ===| r
 |                                            +---+      DIP 3
 |  ____ DIP3                                 |          +-------------------+
 | |.___|                 ____  DIP2          |          | +-+-+-+-+-+-+-+-+ |
 |                       |.___|               |          | | |2| | | | | | | |
 |                                            |          |o+-+-+-+-+-+-+-+-+ |
 |                              light         |          +-------------------+
 +--------------------------------U-----------+
                  Front

DIP 1: switches 1 to 4 select the CRU address (see [above](#CRUmap)),
switches 5 and 6 are not used, switches 7 and 8 select the default bank
(see [table](#Table%20default%20bank) above).

DIP 2 selects the GROM base (see [table](#Table%20bases) above)

DIP 3 selects the number of wait states that depends on the memory
installed. It is preset upon shipping (switch 2 closed, others open) and
should not be modified.

##    The card ROM

The card ROM appears at addresses \>4000-5FFF when CRU bit 0 is set
to 1. Version 1 contains a power-up routine and 6 subprograms. The card
can also install standard headers in GRAMs with programs in them. You
can view a disassembled listing of the whole ROM in my
[download](download.htm#disass) page.

Note: if you are not familiar with the structure of a standard header,
have a look [here](headers.md).

###  The power-up routine

The power-up routine check for the presence of a module, either as a
solid-state cartridge or installed in the Gram Karte. If none is found,
it clears the whole card memory. The user also has the option of erasing
the card by pressing Fctn-4, if no cartridge is plugged-in. Once the
card is erased, the routine places a default header at Gram address
\>6000 in both access ports. If the card answers to port \>9800, the
power-up routine also enables RAM in the range \>6000-7FFF, but only if
no ROM is detected here.

Here is an outline of the power-up routine:

- Let's first check whether a GROM cartridge is plugged in, which would
  prevent us from doing anything with GRAM:
  - Disable the card's GRAM and RAM: CRU bits 1, 2 and 3 =1. Also set
    bit 6=0 (Switched pages map in RAM).
  - Test the first byte of each GROM from \>6000 to \>E000, at port
    \>9800. If a non-zero value is found in any of them, repeat the
    procedure with the next port (\>9804, \>9808, etc)
  - If port \>981C is reached a cartrige is present (a cartridge answers
    to every port): Return immediately.
- If \>00 was found in the first byte of all 6 GROMs in a given port, no
  cartridge can be present. There may be other Gram Karte active (they
  are on by default), but we will always find at least one "empty" port:
  the one that corresponds to the current Gram Karte, since we turned it
  off. It may also be a port with no GRAM memory installed...
- Let's first look if the current port corresponds to our Gram Karte,
  and if it contains valid data:
  - Enable the card GRAM (CRU bit 3=0).
  - Check the first byte of each GRAM from \>6000 to \>E000 in the
    current port. If one of these bytes is \>AA, there is a standard
    header installed (i.e a "soft" module was found).
    - Test the \ key (Fctn-4). If it is down, reset the card
      (see below).
    - If \ is not pressed, see if a ROM cartridge is installed:
      check if addresses \>6000-6030 in cpu memory all contain \>0000.
      - If not, a ROM cartridge is installed. Return immediately.
      - If yes, enable RAM at \>6000-7FFF (CRU bit 1=0), but only if the
        current port is \>9800. This way, multiple Gram Karte won't
        fight for the RAM space: only the one with Gram port \>9800
        turns on RAM at power-up time.
      - Return.
  - If no standard header was found using the current port, check
    whether there is indeed GRAM here: write \>FF then \>00 at GRAM
    addreses \>6000-6030 and see if it reads back correctly.
  - If not, there is a no GRAM in this port: try the next port (\>0004
    away).
- If GRAM was found in the current port but no valid module in it, we
  must reset the card.
- But first, see if a ROM cartridge is present: check if addresses
  \>6000-6030 in cpu memory all contain \>0000.
  - If not and if the current port is \>9800, enable RAM in \>6000-7FFF
    (CRU bit 1=0)
  - Set a flag if that RAM contains a standard header (\>AA found at
    \>6000 in RAM).
- Reset the card:
  - Copy a custom program in the GRAM \>6000-7400 (to call the Loader
    program), from ROM \>4C00-5FFF.
  - Clear the remaining GRAM from \>7400 to \>FFFF.
  - Place the current CRU and GRAM port values at various places in the
    program copied in GRAM, so that each copy deals with its own port by
    default.
  - Do that again, for the port located \>0020 apart from the current
    one.
- Return.

###   The subprograms

There are 6 subprograms in the card ROM, all are meant to be called from
Basic (or Extended Basic):
EDITMEM: provides a small editor for cpu, VDP and GRAM/GROM memory.
MODULE: transfers a TI-Basic program from VDP memory to GRAM, thus
making it a module!
GRAM: calls the Loader program in the first Gram Karte (never returns if
found).
GRAM2: calls the Loader in the second Gram Karte found in the PE-box (if
any).
GRAM3: calls the Loader for the third Gram Karte.
GRAM4: calls the Loader for the fourth Gram Karte.

####  EDITMEM

This is a small, fairly primitive memory editor, to be called from Basic
or Extended Basic. It does not have many features (no search function,
no copy function, no page-up/page-down...) but it has two main
advantages: 1) it is always here and thus very usefull to debug crashed
programs, 2) it runs in the card ROM space (with a few bytes in the
scratch-pad for registers and data storage) which means the whole memory
expansion is free for inspection.

  Type of memory:  G
  Start memory:    6000
  Gram-address:    9800


  G-RAM
  6000 AA 01 00 00 60 18 00 00
  6008 00 00 60 28 00 00 00 00
  6010 00 00 00 00 00 00 00 00
  6018 00 00 60 30 08 50 52 4F
  6020 47 52 41 4D 31 00 00 00
  6028 60 32 61 45 04 54 45 53
  6030 54 00 00 73 24 04 53 55
  6038 42 01 07 20 06 60 B4 BC

  Type of memory:  G
  Start memory:    6000
  Gram-address:    9820


  G-RAM
  6000  *  *  *  *  &lt;  *  *  *
  6008  *  *  &gt;  (  *  *  *  *
  6010  *  *  *  *  *  *  *  *
  6018  *  *  &gt;  0  *  P  R  O
  6020  G  R  A  M  1  *  *  *
  6028  &gt;  2  =  E  *  T  E  S
  6030  T  *  *  I  $  *  S  U
  6038  B  1  *     *  `  *  *

Here is how to use this program:

- Call it with CALL EDITMEM. It erases the screen and display "\* Edit
  memory \* at the top of it.
- You are first prompted for a memory type. Enter C for cpu memory, G
  for GRAM/GROM and V for VDP memory.
- You are then prompted for an address. Enter it as a 4-digit
  hexadecimal number. If you make a mistake, just keep typing: only the
  last 4 digits are considered. Press Fctn-9 to start all over again
  (from memory type).
- If you selected G memory, you are now prompted for the GROM base. If
  you enter an illegal base, it defaults to \>9800.
- The lower part of the screen now displays 8 lines of 8 hexadecimal
  bytes, starting from the address you entered.
- You can navigate in this memory window with the arrow keys. Moving
  beyond the top or bottom borders will scroll the display.
- You can toggle between hexadecimal and ASCII display with Fctn-7.
  Non-displayable characters (0-31, 128-255) are displayed as stars \*.
- You can modify the content of a memory (provided it is not ROM, GROM,
  or write protected RAM) by entering a new hexadecimal nibble ("0"-"9"
  and "A"-"F").
- In ASCII mode, you can just type the new characters (codes 32 to 127
  only).
- Press Fctn-5 or Fctn= to go back to the address input.
- Press Fctn-5 again to exit to Basic.

####  MODULE

This subprogram allows to transfer the currently loaded Basic program
into GRAM, at addresses \>6000-9FFF. The program can be given a name
that will appear on the TI main menu, after "TI-Basic". Selecting the
program enters Basic and begin its execution. Upon completion, the
message "Press any key to reset" is displayed so that you have time to
read error messages, if any. Pressing a key resets the TI-99/4A. The
Basic program can be saved from GRAM into a file and thus becomes a
"module" in itself.

One advantage of this system is that much more VDP memory is now
available for variables. Thus you can write larger Basic programs, that
would normally results in a "Memory full" error.

The only drawbacks are first that such a program cannot be edited. You
should therefore keep the normal (VDP loaded) version for future
modifications. In this event, you will have to again call MODULE and
GRAM to save it as a module. A further problem is that the "RESTORE
line_number" instruction does not work (because of a bug in the Basic
interpreter: RESTORE only searches for line numbers in the VDP memory).

MODULE relies on the fact that a Basic program can be executed either
from the VDP memory or from GROM. A flag is set in byte \>8389 to
indicate GROM as a source (\>FF). A special routine is provided in the
console GROMs (at address \>001E) to begin execution of a Basic program
in GROM without having to enter Basic, nor to type "RUN" (which expects
the line numbers in the VDP memory).

The MODULE subprogram thus copies the current Basic program and its line
number table, from the VDP memory to GRAM memory, ending at address
\>9FFF. In addition, MODULE installs a standard header at GRAM address
\>6000-6100 in the current port. This header contains only one program
to appear on the main menu. The program name is TEST, but can be changed
by passing a string as an argument in the CALL MODULE("NAME") statement.
All it does it to enter the Basic interpeter via the GROM subroutine
mentionned above.

Here is the outline of the MODULE program:

- Make sure the current base found at\>83FA is \>9800, if not return to
  Basic immediately.
- Make sure a Basic program is loaded: get the pointer to the first line
  number from \>8330 and to the last line number from \>8332. Return, if
  they are the same.
- Discriminate against Extended Basic: get the address of the value
  stack in VDP memory, from \>8324. Return if it is greater or equal to
  \>0957. (The value stack should be at \>0958 in XB and \>0800 in
  Basic).
- Copy the line number table into GRAM memory, updating the instruction
  pointers so that they point to the new addresses in GRAM (an entry in
  the line number table consists in two integers: the line number and a
  pointer to the position of that line in the Basic program).
- Save the new pointers: pointer to table start in \>8368, pointer to
  table end in \>836A.
- Copy the Basic program into GRAM memory. The program begins just after
  the line number table and ends at the address found in \>8370. This
  arrangement is maintained in GRAM and the loading addresses are chosen
  so that the program ends at address \>9FFF.
- Copy the custom header in GRAM memory, at address \>6000-60FF. The
  source is found in ROM at \>4B00-4B53 (the remaining bytes are unused:
  \>FF).
- Copy the line number pointers saved above into the custom header, so
  that they can be used to enter Basic with subroutine \>001E.
- Get the string passed as a program name in the CALL
  MODULE("MYPROGRAM") Basic instruction. If none is found, or if it is
  bigger than 10 characters: return (the default name will be "TEST").
  Else copy the name into the custom header so that it appears as a
  program in the TI main menu.
- Clean up the remainder of the CALL instruction, and return to Basic.

The GPL program in the custom header does the following, when selected
from the main menu:

- Enter Basic by calling the GPL routine at \>001E in the console GROM:
  this routine executes a Basic program located in GROM. The next two
  words of data contain the pointers to the start and the end of the
  line number table (which is also the start of the program) in GRAM
  memory.
- Scroll-up two lines (`CALL G@>4D00` twice), then close all the files
  (`CALL G@>4012`).
- Display: "Press any key to reset".
- Once the user presses a key, reset the TI-99/4A..

####  GRAM to GRAM4

These are four entry points to the same program. All it does is to copy
the GRAM/GROM management program into the low memory expansion and
branch to it. This program must be run in the memory expansion because
it performs file operations, which will require turning on the ROM in
the drive controller card, therefore turning off the Gram Karte ROMs. It
resides at \>4E00-5FFF in the ROM and will be copied at \>2700-28FF.

Each entry point corresponds to a different Gram Karte, in the order of
their CRU addresses. Thus we can have upto 4 such cards installed in the
PE-Box. The program can determine which card is currently called by
checking the value of R1: it contains the number of time the called
subprogram was found. Generally R1 = \>0001, but if a subprogram returns
with `B *R11` (as opposed to `INCT R11, B *R11`) the scanning will
continue and, if a subprogram with the same name is found elsewhere, it
will be entered with R1 = \>0002.

Therefore, each entry point verifies if the value of R1 matches what it
expects: \>0001 for GRAM, \>0002 for GRAM2, \>0003 for GRAM3 and \>0004
for GRAM4. If not, it returns with B \*R11 and lets the scanning go on.
This may result in a "not found" error if there are not as many Gram
Karte installed.

If the number matches, the program copies the card management program
(improperly called "Loader") from ROM addresses \>4E00 to \>5FFF (using
the current GROM base found at \>83FA) to the low memory expansion, at
addresses \>2700-38FF. It then leaves Basic without any hope of return
and enters the card management program (at \>27FE).

The card management program changes the screen color and displays the
current GROM base at the top of the screen (if called from the main
menu. From basic it displays GRAM-CARD LOADER). It then displays a
7-item main menu, enclosed in a pretty frame:

 +----------------------------+
 | 1 Load (G)RAM with program |
 | 2 Load GRAM with asm-file  |
 | 3 Load RAM with asm-file   |
 | 4 Load GROM 0-2            |
 | 5 Save GROM                |
 | 6 Save ROM                 |
 | 7 Load-file                |
 | Your choice:               |
 +----------------------------+
 |                            |
 |                            |
 +---c) by H. Martin----------+

The user can select an option by pressing the coresponding key, refresh
the screen with Fctn-6 or exit and reset the TI-99/4A by presssing
Fctn-9. Once an option is selected, it can be aborted at any time by
pressing Fctn-6 or Fctn-9, which returns to the main menu.

Save GROM
Is used to save GROM or GRAM memory to a memory-image "program" file.
You will be prompted for the start address (included) and the end
address (not included) of the memry block to dump. So for instance
\>6000 to \>7800 will save \>1800 bytes, from \>6000 to \>77FF. The
maximum size for a file is \>2000 bytes (this is because file operations
go through VDP memory, and space is limited there). Then you must enter
the GROM base. A default value is proposed, that corresponds to the base
in use when the loader was entered. Finally, you are prompted for a file
name.

Once you entered everything, the operation begins. If an error occurs,
you'll get a brief message ("I/O error xyz" or Loader error xyz"),
followed by "Command completed". If all goes well, the only message will
be "Command completed". In any case, you'll return to the main menu by
pressing \ (or Fctn-6 or Fctn-9).

Save ROM
Is used to save a ROM or RAM memory in a memory-image "program" file.
You must first provide the card's CRU address so that the program can
toggle the appropriate CRU bits. The proposed default is the CRU address
of the card the loader was loaded from. You will be prompted for
switching address of the page to save. If you just press \, or
type \>6000, the whole area \>6000-7FFF will be saved. To save a page,
enter its switching address (\>6002, \>6004, etc): the bank \>7000-7FFF
will be saved, after due switching to the selected page. Finally, you'll
be asked for a filename.

Load (G)RAM with program
This is used to load GRAM or RAM memory with data from the files that
were saved with options 5 or 6. You will be prompted for the card's CRU
address and GROM base, but default values are provided and in most cases
you can just accept them with \. You will then be prompted for a
filename. These memory-image files include a 3-word header that tells
the loader which type of memory it is meant for, the destination
address, and the number of bytes to load. If no such header is found,
you'll get the error message "No (G)RAM fike".

Load GROM 0-2
This allows to override the console GROMs with the Gram Karte GRAM
memory in the range \>0000-5FFF. You will be asked for confirmation, as
there is no guaranty that this will not damage the console GROMs (!) If
you answer "Y" the console GROMs are copied intro the GRAM card, at
adresses \>0000-5FFF in both GROM bases (which means you cannot use RAM
at \>6000-6FFF since this correspond to GRAM \>0000-1FFF in the second
base)..

Load RAM with asm-file
This load an assembly program in "tagged object code" into the RAM bank
at \>6000-7FFF. You will be prompted for the card CRU and the switching
address of the bank to load (enter none to load the \>6000-6FFF memory
area). Note that this option only works with the \>9800 port. Finally,
you'll be asked for the name of the assembly file. This file should be
produced by a 9900 assembler, such as the one in the TI Editor/Assembler
module. It contains all necessary address and size informations.

Note that the Gram Karte loader is very primitive: it does not support
relocatable code, and does not allow for REFs, DEFs, nor for CSEG, DSEG
and other sophisticated features of the TI assembler. Although it is
only meant for the cartridge ROM area, it will load code anywhere in
memory if the corresponding AORG was placed in the source program.

Load GRAM with asm-file
This loads a GPL program (a [low-level language](gpl.md) to be
interpreted from GROM memory) into GRAM memory. The file should be
produced with Michael Weiand GPL-assembler or a compatible program (such
as [mine](download.htm#gplassm)). The loader is extremely primitive and
only allows for loading at absolute addresses, with no REF/DEFs, etc.
You'll be asked for the card's CRU address and for the GROM base you
want to use, then for the filename.

Load-file
A load file is a display/variable 80 text file containg a list of files
to load. This comes handy since a module can theoretically consists in 5
GROM files and 16 ROM files! Each line in the load-file must contain one
and only one filename to be passed to option 3. Optionally, a GROM base
can follow the filename after a unique space (and no leading \>). The
last line must be left blank as a signal for the loader to stop.

Upon selecting this option, you will be prompted for the card's CRU
address, for the GROM base to use and for the name of the DV80
load-file.The files listed in the load-file are then loaded
automatically. If a GRAM file has an address is in the range
\>0000-5FFF, it is loaded in both bases and the override of the console
GROMs is turned on. RAM files greater than \>1000 bytes load from \>6000
and leave the bank enabled for writing, but not switchable. Smaller RAM
files load from \>7000 and leave the bank write-protected and
switchable. A wrong file format aborts the loading process with a "No
(G)RAM file" message.

Protected cartridges

It is hard to believe, but this piece of hardware meant to by-pass the
copy protection mechanism provided by solid-state cartridges, actually
contains a copy protection scheme! The author of the software, Heiner
Martin, was appearently involved in a company called Apesoft. He was
mean enough to install a safety mechanism in option 5 (Save GROM), that
prevents us to save an Apesoft module on disk.

The protection subroutine checks the two bytes at address \>6020-6021 in
GROM memory. If they contain either "AP" or "HM" the message "Module
copy protected" is displayed and the save operation aborts. The same
thing is true for the two bytes at address \>E015-E016, except that is
this case the file is saved, but immediately deleted (and we don't get
any error message).

Overcoming the protection is simple enough: we just need to patch the
loader code, to replace the tests with NOPs or JMPs. The only problem is
that the code is in ROM, so we must wait until it is loaded in the
low-memory expansion to patch it. But that's no problem: the loader
itself will do it for us, thanks to its "load asm-file" function
(remember, it can load code anywhere in memory). Just assemble the
following assembly language snippet, then CALL GRAM with the protected
cartridge plugged in, select option 3 and load our assembly file: voila,
the loader's option 5 will now ignore all protection codes.
 

 * Load it with &quot;Load ass-file in RAM&quot; to the disable protection routine
 * (No need to run it)`
        AORG &gt;30C0
        NOP               Remove test for &quot;HM&quot; at &gt;6020
        AORG &gt;30E4
        NOP               Remove test for &quot;AP&quot; at &gt;6020
        AORG &gt;312A
        NOP               Remove test for &quot;AP&quot; at &gt;E015 
        AORG &gt;3134
        JMP  &gt;3150        Remove 2ND test for &quot;HM&quot; at &gt;6020
        END               

###   Programs and the TI main menu

With a Gram Karte installed, you will find that the TI main menu (upon
leaving the title screen) looks slightly different from the usual:

 Press
  1 for TI-Basic
  2     Gramcard &gt;9800
  3     Review Module Library

     

The last item does not come from the GRAM card, it is added by the
power-up routine in the console GROMs that looks for programs and builds
the title screen. This routine searches the cartridge ROM (at \>6000)
and all GROMs (at \>0000, \>2000, \>4000, \>6000, \>8000, \>A000,
\>C000, and \>E000 for all ports from \>9800 to \>981C) for standard
headers and lists all programs it found (see the standard headers
[page](headers.htm#Programs) for details).

This happens if the menu-building routine detects a device that
discriminates between GROM bases (unlike cartridges and console GROMs).
It is detected by comparing the contents of GROM addresses \>6000-601E
in bases \>9800 and \>9804. If there is a difference, the main menu will
display one page for each active GROM base. Each page ends with the
"Review Module Library" item. By selecting it, the user will access the
next page, that deals with the next GROM base.

If you install a module in the Gram Karte port \>9800 and disable the
second port, the main menu will only contain TI-Basic and the program(s)
in that module. If however both ports are enabled, the TI routine
detects different modules in each (provided the card uses base \>9800 or
\>9804 and has non-zero data loaded in GRAM at \>6000-601E). It then
displays a first screen with the programs found at GROM base \>9800:
TI-Basic, the programs in the loaded module(s), then "Review module
library". Selecting the last option, repeats the scanning for the next
used port (\>9820 if we have only one Gram Karte). This second menu will
again display "TI-Basic" since console GROMs show in all bases, plus any
program in the module(s) installed in that port, etc.

When reset, the Gram Karte installs a default program in the GRAM header
at \>6000. The name of this program is "GRAMCARD \>9800", but it is
modified upon installation to reflect the GROM base it is intalled in:
"GRAMCARD \>9820", etc.

Selecting this program enters the same card managment program as CALL
GRAM, but first displays a shortcut menu:

   +------------------------+
   |                        |
   |     Press              |
   |     1 for Loader       |
   |     2     DSK1.XB      |
   |     3     DSK1.EA      |
   |     4     DSK1.DM      |
   |     5     DSK1.MP      |
   |     6     DSK1.WR      |
   |     7     DSK1.TE      |
   |     8     DSK1.LG      |
   |     9     DSK1.MM      |
   +------------------------+

Options 2 to 9 process "load files" meant to load some of the most
popular modules: Extended Basic, Editor/Assembler, Disk Manager,
Multiplan, TI-Writer, Terminal Emulator, TI-Logo and Mini-Memory. The
files are loaded using the CRU address and the GROM base that were in
use when the shortcut menu was entered.

Pressing Fctn-7 toggles between that list and another, that contains
less meaningfull file names:

   +------------------------+
   |                        |
   |     Press              |
   |     1 for Loader       |
   |     2     DSK1.AA      |
   |     3     DSK1.BB      |
   |     4     DSK1.CC      |
   |     5     DSK1.DD      |
   |     6     DSK1.EE      |
   |     7     DSK1.FF      |
   |     8     DSK1.GG      |
   |     9     DSK1.HH      |
   +------------------------+

Pressing 1 enters the Gram Karte managment program with the default CRU
and GROM base corresponding to those in the header selected from the TI
main menu.

##  Loader file formats

### Memory-image files

These "program" files contain a plain copy of the memory saved, after a
3-word header. This header is different for GRAM and RAM files (and it
does not follow the convention established by TI for RAM memory-image
files, nor by the Gram Kracker for GRAM memory-image files).

GRAM files
\>A5A5 Flag to indicate a GRAM file
\>xxxx GRAM address
\>xxxx Number of bytes (max \>2000)

RAM files
\>5A5A Flag to indicate a RAM file
\>600x Switching address
\>xxxx Number of bytes. If greater than \>1000, loading starts at
\>6000, else is starts at \>7000

###  Tagged object files

These are display/fixed 80 files. They consists in 1-character "tags"
generally followed by one or more byte of data.

GPL programs
 


"........"
ignored.
"........"
chars)
"......"
Loader error &gt;0B if incorrect.
Assembly programs
 


"........"
&gt;2000 or error 8 is issued.
The string of 8 chars (program name) is ignored.
"........"
chars)
"......"
&gt;0B if incorrect.
Revision 1. 3/19/99 OK to release

Revision 2. 3/30/99 Polishing
[Back to the TI-99/4A Tech Pages](titechpages.md)
