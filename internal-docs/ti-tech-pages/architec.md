# The TI-99/4A internal architecture

The TI-99/4A is built around the [TMS9900 microprocessor](tms9900.md).
This is one of the first true 16-bit CPU (central processing unit): it
has a 16-bit wide data bus and a 15-bit address bus. This corresponds to
an address space of 64 Kbytes, arranged as 32 Kwords (a word is two
bytes). For some obscure reason, probably for compatibility with
existing chips, the console has been crippled by multiplexing the data
bus as 2x8 bits to access most memories and peripherals.

64K of memory is not much according to nowadays standards, and most of
it was not implemented in the console anyway. The console only contains
8K of ROM and 256 bytes (sic) of RAM, both accessed as 16-bit.
Additional RAM space is provided by accessing the memory of the
videoprocessor, but only one byte at a time can be transfered.
Additional ROM space is implemented via GROMs (Graphic Read Only
Memories), that are 8-bit serial ROMs and are accessed as memory mapped
devices (i.e. via a few addresses in the CPU memory).

Contrarily to the regular memory that contains [machine
language](assembly.md) directly executable by the CPU, the GROMs
contain a special, low-level, interpreted language called [GPL](gpl.md)
(graphic programing language). It is slower than assembly language, but
somewhat moe compact and easier to use. The VDP memory is typically used
to load TI-Basic programs, to be executed by the [Basic
interpreter](basic.md): a GPL program resident in the console GROMs.

Several peripheral devices can be used to further increase the available
memory:
- a memory expansion card, installed in a peripheral expansion box, can
provide 32K of RAM,
- cartridges that can be plugged directly into the console can provide
upto 40K of GROM and upto 16K of ROM,
- any card in the periperal expansion box can have 8K of ROM that become
available when the card is selected.

Several co-processors and accessory chips help the CPU to perform its
tasks. These are:
- The TMS9918A video-processor. It has 16K of dedicated memory and can
handle a 16-color, 192x256 pixels display.
- The TIM9904 clock driver, feeds the CPU and other chips with a 3 MHz
clock signal,
- The TMS9901 peripheral system interface handles communications with
additional hardware: keyboard, joysticks, cassette tape recorder, and
direct sound output. It also serves as an interrupt handler. It is
accessed via the CRU.
- The TMS9919 sound chip can simutaneously generate three tones and one
noise.
- An external module contains a speech synthesizer and a vocabulary
chip.

   ,---|TIM |     |      |        +-----+    |########|            o
  ===  |9904|--4--| TMS  |  CRU   | TMS |====|########|            |
   &#39;---|Clk |     | 9900 |---3----| 9901|================&gt; &gt;=====|___|
       +----+     |      |        |     |                        ______
,---3-------------| CPU  |        | PIC |================&gt; &gt;====| O__O |
|  CRU            +-+--+-+        +---+-+                       |______|
|                   |  |    +-----+   |                         Cassette
|        | RAM |____|16|____| ROM |   |Interrupts
|        | 256 |____    ____|     |   |
|        |bytes|    |  |    | 8K  |   |
|                   |  |    |     |   |
|                   |  |    +-----+   |
|                   |  |____________+-+---+     +-----+
|                   |   ____,--8----| TMS |     | RAM |
|                   |  |            |9918a|-----|     |
|        +-----+  |  mpx  |     ,---|/9929|     | 16K |
|        | GROM|     |8|       ===  |     |     |     |
|        |     |-----| |        &#39;---| VDP |--,  +-----+         Monitor
|        | 24K |     | |            +-----+  |               +----------+
|        |     |     | |                     |               |(        )|
|        +-----+     | |                     &#39;---------&gt; &gt;---|( screen )|
|   Cartridges       | |                                     |(________)|
|    _+-----+        | |                                     +----------+
|   _|| GROM|        | |
|   |||     |        | |
|   ||| 40K |---&gt; &gt;--&#39; |        +-----+                           _ /|
|   |||     |        | |        | TMS |----------------&gt; &gt;-------| | |
|   ||+-----+        | &#39;--------| 9919|                             \|
|   +------&#39; x16     | |        |sound|
+-&gt;                  | |      ,-|chip |
|    _+-----+        | |      | +-----+
|    || ROM |        | |      |                  Speech synthesizer module
|    ||     |---&gt; &gt;--&#39; |      |                     +-----+   +------+
|    || 8K  |        | |      &#39;-------------&lt;  &lt;----| TMS |   |serial|
|    ||     |        | &#39;--------------------&gt;  &gt;----|5220C|===| ROM  |
|    |+-----+ x2     | |                            |     |   |      |
|                    | |                            |synth|   | 16K  |
|                    | |                            +-----+   +------+
V                    VVV
                                                V   Peripheral       VVV       +-----+
|  Expansion Box     | |       | RAM |
|                    | &#39;-------|     |
|                    | |       | 32K |
|  Upto 7 cards      | |       |     |
|                    | |       +-----+
|                  +-+-+-+_
|     CRU          | ROM ||_              _____
&#39;------------------|     |||=============| === | Floppy drive
                   | 8K  |||
                   |     |||=========&lt;  Parallel + serial ports
                   +-----+||
                    &#39;------&#39;

Here is commented [picture](motherb.jpg) of the motherboard (68K).

##  Address space

    >0000 ------------------+
          | Console ROM     |
          +                 +
          |                 |            +------------------+ >8000
    >2000 +-----------------+            | (mirror of RAM)  |
          | Low memory      |           /|                  |
          +                 +          / |>8300-83FF: RAM   |
          | expansion       |         /  +------------------+ >8400
    >4000 +-----------------+        /   |>8400: sound chip |
          | Peripheral      |       /    |       write      |
          + cards ROM       +      /     |                  |
          |                 |     /      +------------------+ >8800
    >6000 +-----------------+    /       |>8800: VDP read   |
          | Cartridge       |   /        |>8802: VDP status |
          + ROM/RAM         +  /         |                  |
          |                 | /          |>8C00: VDP write  |
    >8000 +-----------------+-           |>8C02: set address|
          | scratch-pad RAM |            |                  |
          + memory-mapped   +            |                  |
          | devices         |            +------------------+ >9000
    >A000 +-----------------+-           |>9000: speech     |
          | High memory     | \          | synthesizer read |
          + expansion       +  \         |                  |
          |                 |   \        |>9400: speech     |
    >C000 +                 +    \       | synthesizer write|
          |                 |     \      +------------------+ >9800
          +                 +      \     |>9800: GROM read  |
          |                 |       \    |>9802: read addr  |
    >E000 +                 +        \   |                  |
          |                 |         \  |>9C00: write data |
          +                 +          \ |>9C02: set address|
          |                 |           \|                  |
    >FFFF +-----------------+            |                  |
                                         +------------------+ >9FFF

#### Console ROM

There are 8 Kbytes of fast, 16-bit ROM in the console. They contain the
GPL interpreter (that runs programs loaded in GROM), the keyboard
scanning routine, the interrupt service routine, and several routines to
be used for calculation with real number, Basic variables handling, and
peripheral cards access.

####  Memory expansion

The 32 Kbytes of slow, 8-bit memory installed in the PE-Box are
installed as two distinct domains: 8K at \>2000-3FFF known as the "low
memory expansion", and 24K at \>A000-FFFF, known as the "high memory
expansion".

####  Peripheral card ROM

Each card can have upto 8K of memory mapping at adresses \>4000-5FFF.
This is generally ROM, although the Horizon ramdisk uses non-volatile
RAM here. This memory typically contains the routines needed to operate
the card: DSR (device service routines), subprograms, interrupt service
routines (ISR), and power-up routines. Some cards, such as the P-code
card, need more than 8K and bank-switch part of the domain, but I don't
think there is a defined standard for this.

####  Cartridge ROM

Each cartridge can have upto 8K of memory mapping at addresses
\>6000-7FFF. This is generally ROM, although the Mini-Memory has 4K of
ROM and 4K or RAM. If a cartridge needs more than 8K, it can bank-switch
the second half of the domain from \>7000 to \>7FFF. Switching is
achieved by writing to the ROM: writing to \>6000 selects the first
page, writing to \>6002 selects the second. The german Gram Karte
expands this convention upto 16 pages, but I am not aware of any
cartridge with more than two pages.

I was told that cartridges like Extended Basic in fact bank-switch the
whole 8K domain, but the first 4 K are identical in both banks, so it
just looks like only the upper 4K is switched. I was also told that some
third party cartridges use a different switching method, but I don't
have any details.

####  Scratch-PAD RAM

This is the only console RAM directly available to the CPU: 256 bytes of
fast, 16-bit memory installed at addresses \>8300-83FF. Since the adress
bus is not completely decoded by the console circuitery, these 256 bytes
map redundantly at \>8000-80FF, at \>8100-81FF and at \>8200-82FF. The
GPL interpreter accesses them at \>8300-83FF and the DSRs in peripheral
cards generally derive the scratch-pad address from the registers of the
calling program. This suggests that Texas Instruments may have had the
intention to install 1024 bytes of RAM in a future version of the
console... that never saw the light.

####  GROMs

Graphic Read-Only Memories, or GROMs, were the solution that Texas
Instruments found for software protection: these unique 8-bit serial
ROMs were only manufactured by TI. They contain an internal pointer that
is automatically incremented each time you access the chip. You can set
the value of the pointer by passing two bytes to the chip at address
\>9C02, or retrieve the current address (plus 1) by reading two bytes at
address \>9802. Data can be read one byte at a time from address \>9800
and written at \>9C00 (of course nothing will be written since these are
ROMs). This provides an additional 64K of memory. The console contains
three GROM chips, each \>1800 bytes in length, installed at GROM
adresses \>0000, \>2000 and \>4000. They contain the main power-up
routines, mathematical routines and the TI-Basic interpreter. The
remaining space is available for cartridges. Note the awkward way TI had
of leaving \>800 unavailable bytes at the end of each GROMs.

If more memory is needed, provisions were made in the console to access
upto 16 distinct GROM domains, using different sets of mapping
addresses. For instance, the next domain would be accessed at \>9804
(the GROM base), \>9806, \>9C04 and \>9C06. The calling routines in the
console ROM automatically scan all blocks upto base \>983C. However, the
address-decoding circuit for the console GROMs does not take this into
account, so console GROMs will map at GROM addresses \>0000-5FFF in any
base.

Later on, several companies manufactured GRAM cards. As there is no such
thing as a GRAM, these were in fact RAM cards, with a complicated
circuitery in charge of mimicking the GROM behaviour. The german GRAM
Karte for instance, provides 64K of GRAM at base \>9800 and 64K at base
\>9820. The second block can alternatively used as cartridge RAM,
appearing at \>6000-7FFF in the CPU addressing space.

####  VDP memory

The videoprocessor has 16K of private RAM that it used to store the
screen image, the character patterns and colors, and the sprite
characteristics. Unless it is used in bit-map mode, only a small portion
of this memory is effectively needed. The remaining is therefore
available for use by the CPU. However all reading and writing operations
must be carried on by the videoprocessor. The CPU passes the required
address to the VDP as two bytes at address \>8C02, and can then read
bytes from \>8800 or write bytes at \>8C00. The VDP increments an
internal pointer after each read/write operation. Contrarily to the
GROMs, one cannot retrieve the value of this pointer from the VDP.
Instead, address \>8802 is used to read the VDP status register. This is
definitely a slow way to access RAM, but it probably allowed to market
the TI-99/4A at a much lower price, since RAM was fairly expensive in
the 80s.

####  Data bus multiplexer

The TMS9900 being a 16-bit processor, it has 16 data lines and 15
address lines. However, only the console GROMs (\>0000-1FFF) and the
scratch-pad RAM (\>8300-83FF) are accessed in this way. Peripheral
memory in the range \>2000-7FFF and \>A000-FFFF is accessed via an 8-bit
data bus, and a 16-bit address bus. A small circuit in the console
multiplexes the data bus, creates the 16th address line (A15) and puts
the TMS9900 on hold while the least significant byte (the one at the odd
address) is processed.

The VDP is hooked on the 16-bit bus, but only to lines D0-D7, i.e. it
accesses the most significant byte only. The GROMs, the sound chip, and
the speech synthesizer are hooked to the multiplexed 8-bit bus. However,
their selection logic senses A15, so they only react to even addresses.
In both cases the multiplexer (uselessly) puts the CPU on hold, since
all these devices map in the range \>8400-9FFF.

##  Coprocessors

### Videoprocessor

The TMS9918a (TMS9929 in european consoles) videoprocessor is in charge
of handling the screen display, as 192 lines of 256 pixels, in 16
colors. There are four display modes:

Standard mode: 24 lines and 32 columns of 8x8 pixel characters, with 2
colors (foreground and backgroud) per group of 8 characters.

Text mode: 24 lines and 40 columns of 8x6 pixel characters. Monochrome
display: 2 colors (foreground, background) for all characters. No
sprites.

Multicolor mode: 48 lines and 64 coloumns of 4x4 boxes. Each box can
have its own color.

Graphic mode: 192 lines and 32 columns of 1x8 pixels "characters". Each
character can have its own set of two colors.

In addition, the VDP can handle upto 32 "sprites". These are graphic
objects that can be displayed anywhere on screen and have each a color
of their own (their background is always transparent). They provide a
very convenient way to display animated objects.

To achieve these tasks, the VDP has a private RAM memory of 4 Kbytes.
Since only the rarely used graphic mode requires that much memory, most
of it is loaned to the CPU for data storage. However, the CPU cannot
access this memory directly: all requests are processed by the VDP, when
it's not busy drawing the screen. The VDP maintains an internal register
that points to the next address to be accessed by the CPU. In addition,
it also contains 8 write-only configuration registers, that define the
display mode, the size of the sprites and the location of the various
data tables in VDP memory. It also has a status register that can pass
various information to the CPU.

The CPU and the VDP communicate via 4 addresses in the CPU memory
space:
\>8800: the CPU reads data from the VDP,
\>8802: the CPU reads the VDP status register,
\>8C00: the CPU writes data to the VDP memory,
\>8C02: the CPU sends an address request to the VDP or writes to one of
its registers.
Note that, even though the VDP is installed on the data bus before the
16-to-8 bit multiplexer, it is only connected to D0-D7.

The VDP can periodically issue interrupts, when it has finished with
drawing the screen. This happens 50 times a second on European consoles,
60 times a second on North-American consoles. These so-called "vertical
retrace interrupts" are processed by the TMS9901 co-processor.

The VDP has its own crystal, with a frequency of 10.738635 MHz, for
proper generation of the video-signal.

[To the VDP page.](tms9918a.md)

###  Peripheral Interface Controller

The TMS9901 peripheral system interface is in charge of interfacing the
CPU with the outside world. Its has many input/output pins, most of
which can be programed either as input, output or interrupt-generating
pins. In the TI-99/4A, this chip is in charge of handling the keyboard,
the joysticks and the cassette tape recorders. It also receives all
interrupt requests from the VDP and from peripheral cards and forwards
them to the CPU. Finally, it can "talk" to the sound chip.

The TMS9901 also contains an internal decrementer/timer, which is mainly
used by the cassette operation routines. Due to a major desing flaw in
the interrupt service routine in the console ROM, the timer is very
infortunately of little use to the average user.

The TMS9901 and the CPU communicate via the CRU (communication register
unit): this can be viewed as a private 1-bit data bus, used for I/O
operations. The CRU address space is 4096-bit wide, the top 2048 bits
being reserved for peripheral cards. The TMS9901 uses the lowest 32 bits
of this space.

[To the TMS9901 page.
](tms9901.md)[To the keyboard page.
](keyboard.md)[To the joystick page.
](joystick.md)[To the cassette tape page.
](cassette.md)[To the interrupts page.](ints.md)

###

### Sound chip

The TMS9919 sound chip can emit simultaneously upto 3 tones and 1 noise
(which can be a periodic or a white noise). The theoretical frequency
range is from 28 Hz to 111 kHz and the volume can be adjusted on a scale
of 0 to 15.

The TMS9919 outputs sound directly to the monitor plug, independently
from the VDP.

It can also accept an external sound signal and mix it with its own,
internally generated signals. There are two connections for this input
line: one is in the cassette recorder plug and can be enabled/disabled
by the TMS9901, which provides the programmer with a possibility for
direct sound output. The second connection is in the console side port,
which allows for the use of an external speech synthesizer module.

The sound chip communicates with the CPU via a single address in the CPU
memory space:
\>8400: the CPU sends sound data to the TMS9919.

[To the TMS9919 page.](tms9919.md)

###  Clock generator

The TIM9904 is not really a co-processor. It's a clock generator that
uses a crystal oscillator to generate the 3 MHz clock signals needed by
the CPU. In fact, it generates four such signals, shifted by 1/4 of a
phase with respect to each other. The CPU requires all four phases, but
the remaining coprocessors and circuits can do with only one (the third
one was choosen in the TI-99/4A).

[To the TIM9904 page.](tim9904.md)

###  Speech synthesizer

The speech-synthesizer co-processor is not part of the console, rather
it is integrated in a separate module to be plugged in the console side
port. The module also includes some specialized serial ROM memory that
hold a minimal vocabulary encoded in a special format called LPC (linear
predictive coding).. However, the synthesizer can also be fed LPC
directly from the CPU. The CPU can access the ROM memory, but only
through the synthesizer.

The sound signal generated by the synthesizer is fed to the TMS9919
sound chip via the console side port. The sound chip merely relays it to
the loudspeaker connection in the monitor plug.

The CPU interacts with the speech synthesizer via two addresses in CPU
memory:
\>9000: the CPU reads data from the synthesizer,
\>9400: the CPU sends sound data or commands to the synthesizer.

[To the speech synthesizer page.](speech.md)

##  Cartridges

In front panel of the console is a [36-pin port](pinouts.htm#Cartridge)
that can be used to plug in a cartridge. Such cartridges can containg
GROM, ROM or both.

The available GROM addresses are \>6000 to \>F000. Theoretically, a
cartridge could also contain a selection circuitery so that its GROM
only answer to a given base, e.g. \>9800, \>9804, etc. To my knowledge,
no cartridge contains such a circuit and all answer to each and every
GROM base.

The available addresses for ROM in CPU memory are \>6000 to \>7FFF. If
needed, a cartridge can bank-switch (part of) this domain. The standard
switching circuits are triggered by writing to ROM addresses \>6000
(select first bank) or \>6002 (select second bank).

Note that the address bus is incomplete in the cartridge port: the three
most significant line A0, A1 and A2, are missing. On the other hand, two
extra address lines are available: GS\* which is active (low) when the
address is in the range \>9800-9FFF and therefore can be used to detect
GROM access. And ROMG\* which is active (low) when the address is in the
range \>6000-7FFF and can thus be used to select the cartridge ROM.

Strangely enough, the cartridge port also receives the three CRU lines:
CRUCLK (the clock), CRUIN (data to the CPU) and CRUOUT (data from the
CPU) which is multiplexed with A15. It would therefore be possible to
control cartridge functions via the CRU, although no addess is reserved
to this end in the CRU address space.

[To the Mini-memory cartridge page.
](mm_mod.md)[To the Editor/Assembler cartridge page.
](ea_mod.md)[To the Ti-Writer cartridge page.](wr_mod.md)

##   Peripheral expansion box

This bulky and heavy box contains an independent power supply, a disk
drive and has room for eight peripheral cards. One of them has to be the
connection card that merely contains buffers and connects the box to the
TI-99/4A via a [44-connection port](pinouts.htm#Side) on the right side
of the console. The remaining seven slots can be used by additional
hardware.

Typical peripheral cards are:

The 32K [memory expansion card](mem32k.md), that provides the TI-99/4A
with 32K of DRAM, mapping at \>2000-3FFF and \>A000-FFFF in CPU memory.

The [disk controller card](disks.md), that can handle upto 3
single-density dobble-sided floppy disk drives.

The [RS232/PIO interface card](rs232c.md), that provides a custom
parallel port (PIO) and two serial port obeying the RS232C standard.

There is all a flock of third party cards to be installed in the PE-Box:
[extra-memory](superams.md) cards, hard-drive controller, [GRAM
cards](gramkart.md), etc.

The CPU typically interacts with peripheral cards via the [communication
register unit](cru.md): CRU addresses \>1000 to \>1F00 are reserved to
this end. In principle, each card is allotted a pool of upto 128 CRU
bits, starting at an even address: \>1000, \>1100, \>1200, etc. This
first bit is reserved to turn on the card ROM memory that maps at
\>4000-5FFF in the CPU address space.

Some cards use this memory space to map special registers (e.g the disk
controller's registers map at \>5FF0-5FFE) but this is completely
hardware-dependent. Fortunately, since the driver that handle a given
card can be included in the card ROM, there is no need for the user to
worry to much on how to access a card: scanning routines in the console
ROM are in charge of calling the appropriate programs in the card ROMs.
According to their function, these programs are called power-up
routines, DSR, subprograms, or ISR. Of course, this implies that the
card ROM must follow a special convention to list all its available
routines. This convention is very similar to the one adopted to organise
routines inside GROM memory (that however contain GPL, not assembly
language): it is called a [standard header](headers.md).

[To the PE-box page.](pebox.md)

Revision 1. 3/19/99 OK to release

Revision 2. 5/28/99 Added "Headers" and "PAB" chapters.
Revision 3. 6/9/99 Removed them, after all. They 'll be on a different
page.

Revision 4. 7/3/99 Main schematic. Added co-processors chapter. Added
links to other pages.
Revision 5. 8/20/99 VDP on 16-bit bus. Added paragraph on multiplexer.

Revision 6. 8/16/00 Added a few links, link to motherboard pix.

[Back to the TI-99/4A Tech Pages](titechpages.md)
