# Programming the USB-Smartmedia card

This page contains specific informations on how the program the various
chips on board the USB-SM card. For general instructions on how to write
DSRs for the USB-SM card, follow [this link](usb_dsr.md). For a
description of ELM, the EEPROM loader and manager for the USB-SM card,
see [here](usb_load.md).

For information on the USB-SM card hardware, follow [this
link](usb.md). For instructions on how to build the card, look
[here](usbhowto.md).

[CRU map
](#CRU%20map)[Memory map
](#Memory%20map)[StrataFlash operations
](#StrataFlash)[SmartMedia operations
](#SmartMedia)[USB controller operations](#USB%20operations)

###  CRU map

The CRU bas address can be set with the DIP switch to any address from
\>10xx to \>1Fxx.

There are 24 CRU bits available for output, but only 8 for input. The
meaning of the input bits is independent of the corresponding output
bits. Here are the output bits:


1: Card maps at &gt;4000-5FFF
1: SmartMedia maps at &gt;4FF0-4FFF, USB at &gt;5FF0-5FFE
1: Interrupts enabled.
1: Smart Media card selected.
1: EEPROM write-enabled, read-disabled.

And here are the 8 input bits:


1: Flash-EEPROM is ready.
1: SmartMedia card absent or ready.
1: A card is in the connector.
1: Card absent or not protected.

Example: switching pages

* Assume card CRU is in R12, page number in R0
ROMPG  AI   R12,&gt;000A    Points to first bit of page number
       LDCR R0,11        Select page
       AI   R12,-10      Restore original R12
       B    *R11`
* This routine selects a SRAM page
* Assume card CRU is in R12, page number in R0
RAMPG  AI   R12,&gt;0020    Points to first bit of page number
       SLA  R0,8         Page # is only 8 bits
       LDCR R0,8         Select page
       SRL  R0,8         Restore R0
       AI   R12,-32      Restore original R12
       B    *R11

###  Memory map

`>4000-4FFF`: Flash-EEPROM 8 megabytes. Mapped as 2048 pages of 4K,
switched via the CRU.
`>5000-5FFF`: SRAM 1 megabyte. Mapped as 256 pages of 4K, switched via
the CRU.
Only when CRU bit 1 is set to '1':

`>4FF0`: SmartMedia read data
`>4FF8`: SmartMedia write data
`>4FFA`: SmartMedia write address
`>4FFC`: SmartMedia write command

`>5FF0`: USB host controller read data
`>5FF4`: USB device controller read data
`>5FF8`: USB host controller write data
`>5FFA`: USB host controller write command (i.e. register \#)
`>5FFC`: USB device controller write data
`>5FFE`: USB device controller write command (i.e. register \#)

The Smartmedia bus is 8-bit wide, so all SM ports map at even addresses.
You can just disregard the odd byte.

The ISP1161 has a 16-bit bus, although its registers vary in size from 8
to 32 bits. The rules are the following:

- 8-bit registers are accessed through the most significant (i.e. left)
  byte.
- 16-bit registers are accessed word-wise, but remember that the data
  bus is inverted, so perform a SWPB when accessing the ports directly
  (my low-level routines do this for you).
- 32-bit registers are accessed as two words in succession, at the same
  address, with the least significant word first. Both are byte-swapped,
  which means that the bytes come in reverse order: least significant to
  most. Again, my low-level routines hide this from you.
- The stacks (ATL, ITLs and endpoint stacks) are accessed word-wise, but
  the bytes are NOT swapped (they obey the "Intel" byte order, but I
  rewired the data bus to correct this, which is why registers are now
  byte-swapped). Thus data can be transfered to/from the stack in TI's
  usual manner: most significant byte first, i.e. in the left byte of
  the word.

##  StrataFlash operations

[Read array
](#read%20array)[Write word
](#write%20word)[Write buffer
Block erase
](#write%20buffer)[Suspend/Resume
](#suspend)[Set/Clear lock-bits
](#lock%20bits)[Read status
](#read%20status)[Clear status
](#clear%20status)[Read ID
](#read%20id)[Read query
](#read%20query)[Configuration
](#config)[Other commands](#others)

### Introduction

The Flash-EEPROM maps at \>4000-4FFF (although, when CRU bit 1 is '1',
the Smartmedia card maps at \>4FF0-4FFF). This corresponds to a 4-Kbyte
page. You can toggle pages by writing a page number to CRU bits 4
through 15. With an 8-meg chip, you have 2048 available pages, with a
4-meg chip "only" 1024 pages.

Reading from the chip requires no other operation than setting the
proper page number. Writing to the chip, however, is more complicated.
First, all writing operations consists in at least two write cycles. Any
illegal value, or intervening read cycle, will abort the operation.

When writing to, or erasing the Flash-EEPROM, you must first set CRU bit
4 to '1'. This will prevent reading from the chip (because the TMS9900's
"read before write" strategy could abort the unlocking sequence) and
enable the Vpen write-enabling pin. The corresponding switch on the
DIP-switch must be closed for this to work properly. Don't forget to
turn bit 4 back to '0' so you can read the chip once done.

Because the StrataFlash is an EEPROM, you can only turn '1' bits into
'0' when writing. Initially, the chip contain only '1's, so you can
write anything you want. Correcting mistakes is trickier, however.
Sometimes it is possible, e.g.when turning a 'C' (ascii 67, binary 0110
0111) into a 'B' (ascii 66, binary 0110 0110): in this case we are
changing the rightmost bit from '1' to '0', so it's OK. Changing the 'B'
into a 'A' (ascii 65, binary 0110 0101) is not possible however, because
the rightmost bit need to be changed from '0' to '1'.

In such a case, you must first erase the byte, i.e. turn it into an \>FF
(binary 1111 1111). But because the chip is a 'Flash', you cannot erase
only one byte: you must erase 128 kbytes at a time! This might appear
like an inconvenience, but it was done so it is possible to erase the
whole chip very rapidly: erasure is a very slow operation and erasing 8
megs byte-per-byte would take forever.

Let's now review the various commands that can be sent to the
StrataFlash:

    >FF Read multiple words
    >10 Write word
    >40 Write word
    >E8 Setup write from buffer
    >D0 Confirm write from buffer
    >20 Setup block erase
    >D0 Confirm block erase
    >B0 Suspend write/erase
    >D0 Resume write/erase
    >60 Setup set/clear lock bits
    >01 Confirm set lock bit
    >F1 Confirm set master lock bit
    >D0 Confirm clear all lock bits
    >70 Read status
    >50 Clear status
    >90 Read chip ID
    >98 Read query
    >B8 Set configuration

####  Read array

By default, the StrataFlash will be in 'Read array' mode upon power-up,
i.e. it operates like a regular ROM memory: the data port returns the
word found at the address placed on the address bus. The device will
remain in this mode until you send a command to enter another mode. If
you wish to return to 'Read array' mode after switching to another mode,
you only need to write the command \>FF to the chip, at any address.

So does this imply that there are other read modes? Yes there are
several: you can read the chip ID codes, or a more detailed "query
structure". You can also check the chip status, or clear it.

####  Write word

But before we go there, let's first see how to program a word, i.e.
write to the Flash-EEPROM. There are two commands for this, \>40 and
\>10. You can write either one at any address (e.g the adress of the
word to be written). Then you write the word you want at the desired
address. At this point, the 'write state machine' inside the chip takes
over, writes the word, and verifies it. Any incoming read cycle will
return the current status (see 'read status' [below](#read%20status)).
You can also check the status of the STS pin, via CRU bit 4 to determine
when writing is completed: it reads '1' when the chip is done.

Note that the status only reports errors due to bits that did not turn
from '1' to '0' (which they should). The opposite operation is
impossible and requires prior erasure, which can only be done
block-wise. Thus, it is good practice to check that your write operation
is legal beforehand, using for instance the CZC instruction: any '0' bit
that needs to be converted to a '1' won't work.

Remember that you need to toggle CRU bit 4 to '1' before issuing the
command to enable writing (and disable reading), and that the
"antiviral" DIP-switch must be closed. Obviously you can't read the
status byte while in this mode. Thus, it's better to monitor the STS
pin, via CRU bit 4, until the chip is ready to be placed back in reading
mode.

####  Write buffer

To speed things up, it is possible to write several words in parallel.
To do so, you must copy these words to a RAM buffer inside the
StrataFlash, which the 'write machine' will use for the writing.
Obviously, this is a multi-step command.

You must first write the command \>E8 to an address within the block
where you intend to write (e.g. the starting address). This allows the
chip to issue the 'extended status' for this block upon read operations.
You can tell wether the block is available for writing by checking the
status bit with weight \>80: if it reads as '0' the chip isn't ready to
write to this block yet (e.g. it's being erased, but suspended).

Then you must write the number of words you intend to write, again to an
address within the block. This number should obviously be within the
limits of the RAM buffer (which is 32 bytes). Legal values are \>0000
through \>000F, which correspond to 1 through 16 words, since the count
begins with zero.

Now you can start writing the data words at the required addresses. You
don't have to enter them in order, but all addresses must lie between
the start address and the end address (which must be start + count). Be
carefull that all words must be written to the same block.

Finally, you must issue the confirmation command \>D0, at which time the
'write state machine' will begin programming the chip. It will also
issue the status data upon any read cycle. Note that an error will occur
if you trespass the limit between two blocks.

####  Block erase

So, if 'write' can only change 1s to 0s, how can you revert these
changes? This is done with an 'erase' operation. However, because the
chip is a Flash, you cannot erase individual words. You must erase at
least one block, which is 128 Kbytes in length.

To erase a block, you must first write the command \>20 to any address.
Then you must write the confirmation command \>D0 to an address within
the block to be erased. The 'write state machine' then takes over and
turns every word within the block to \>FFFF. It also continuously issues
status data, so you can check the status during and after erasure with a
simple read cycle (for which you must toggle CRU bit 4 back to '0').

####  Suspend & Resume

These commands allow to temporarily suspend erasure, in order to read or
write data to a block other than the one being erased. To do so, write
the \>B0 command to any address within the chip. Then check the status:
bit \>80 and \>40 should both become '1'.

At this point, you can issue any of the following commands: Read array,
Read query, Read status, Clear status, Configure, Write word and Write
to buffer.

Once you are done, issue the 'Resume' command by writing \>D0 to any
address. This will cause the chip to resume erasing where it paused, and
status data to be issued upon read cycles. Note that erasure won't
resume until any intervening write command is completed.

####  Set/Clear lock-bits

An additional protection system is provided to avoid spurious writings:
for each block you can set a 'lock-bit' which effectively protects the
block against any modification, being writing or erasure. These bits can
be cleared subsequently, if you do need to modify a block.

To set a 'lock-bit' first write the command \>60 to an address located
within the block you intend to write-protect. Then write \>01, again to
any address within the block. This command automatically outputs the
status data unpon subsequent read cycles.

Lock-bits cannot be unlocked individually, but you can issue the 'clear
lock-bits' command by writing \>60 to any address, then \>D0 to any
address. This will reset all lock-bits and output the status.

There is also a 'master lock-bit' that can only be set (by writing \>F1
after command \>60), but not reset. Setting it results in making it
impossible to set block lock-bits unless the RP\* pin is held high.
Since this pin is connected to the Reset\* line in the USB-SM card, it
will always be high and the master lock bit won't make any difference
for us.

####  Read status

As you probably noticed, many commands automatically issue the chip
status during and after completion. What this means is that a read cycle
will not show the contents of the memory, but rather the status of
the'write state machine'. You will need to issue the 'read array'
command to return to standard mode.

In case you need to read the status while in a 'Read array' mode, write
command \>70 to any address, then read the status from any address.

Status data consists in the following bits:

    >0001: reserved
    >0002: write-protection error
    >0004: reserved
    >0008: error, low Vpp
    >0010: error in writing, or in setting lock-bits
    >0020: error in erasing, or in clearing lock-bits
    >0040: block erase was suspended
    >0080: write-machine is ready (if not, rest of status is invalid)

####  Clear status

The four error bits in the status are set by the write state machine,
but must be reset by software. You do this by writing the command \>50
to any address in the chip. Note that this won't work if the
write-machine is still programming.

####  Read ID

This command is mainly usefull as a test to make sure that the chip is
answering properly. You can also use it to determine which blocks are
locked, so you can relock them after a 'Clear lock-bits' command.

To enter 'Read ID' mode, write command \>90 to any address. Then read
the desired code from one of the following addresses:

    Address     Contents
    >00000      Manufacturer code (>89 = Intel)
    >00001      Device code (>14 = 4 Meg, >15 = 8 Meg)
    >x0002      Block 0 lock-bit (>01 = block 0 is locked)
    >00003      Master lock-bit (>01 = setting lock-bits needs RP* high)
    >x0002      Block x lock-bit (>01 = block x is locked)

To exit 'Read ID' mode, simply enter another command.

Example:

      MOV  R0,@&gt;4000        Write at any address
      MOV  @&gt;4000,@MFG      Save manufacturer code
      MOV  @&gt;4002,@DEV      Save device code

####  Read query

This feature is mainly used by programs that can handle different kinds
of Flash chips. It lets you access the so-called "query structure", a
set of data describing the chip. To enter "Read query" mode, you write
the command \>98 at any address in the chip. Then you can read the data
you want from the adequate address. Note that the table below contains
word addresses, the actuall address is \>4000 + (2 \* word_address).
Since we're reading data 16 bits at a time, the MSbyte will always be
\>00: e.g. the string "QRY" reads as \>0051, \>0052, \>0059.

    Word address  Contents
    >00000        Manufacturer code
    >00001        Device code
    >00004-F      Reserved for vendor-specific info
    >00010-2      String "QRY"
    >00013-4      Primary vendor command set address (low byte - high byte)
    >00015-6      Primary vendor extended query table address (low - high)
    >00017-8      Alternative vendor command set address (>0000 = none)
    >00019-A      Alternative vendor extended query table address (>0000 = none)
    >0001B        Min Vcc: first nibble = volts, 2nd = 10th volts (e.g. >0045 = 4.5 V)
    >0001C        Max Vcc: coded as above
    >0001D        Min Vpp (programming voltage)
    >0001E        Max Vpp
    >0001F        Single word program time out: 2 at power of 'n' microseconds
    >00020        Buffer write time out: coded as above (e.g. >0007 = 128 usec)
    >00021        Block erase time out: 2 at power of 'n' milliseconds
    >00022        Full chip erase time out: coded as above
    >00023        Maximum single word program time out
    >00024        Maximum buffer write time out
    >00025        Maximum block erase time out
    >00026        Maximum full chip erase time out
    >00027        Device layout info: 2 at power of 'n' bytes
    >00028-9      Device interface: >00000000=byte, >00010000=word, >00020000=mixed
    >0002A-B      Write buffer size: 2 at power of 'n' bytes (n: low - high bytes)
    >0002C        Number of erase block regions
    >0002D-E      Number of blocks in erase region #1
    >0001F-30     Block size (in region #1): 'n' * 256
    >0x002        Status register for block x: >0001 = locked, >0002 = erased failed.
The optional "extended query table" can be found at the address
specified in bytes \>00015-6. Its structure is the following:

    Word   Contents
    0-2    String "PRI"
    3-4    Version number (major - minor) in ascii
    5      Optional features: >01=chip erase, >02=suspend erase, >04=suspend program
           >08: legacy lock/unlock, >10: queud erase
    6-8    Reserved for more options
    9      Program after erase suspended? >0001=yes
    >A     Block status register mask (>01=lock bit status register, >02=lock down bit status)
    >B     Reserved
    >C     Optimal Vcc: first nibble = volts, 2nd = 10th volts
    >D     Optimal Vpp: coded as above
    >E     Reserved

####  Configuration

This command only lets you define the usage of the STS pin. By default,
this pin is high when the chip is ready and low when it's busy. This is
how we want it for the USB-SM card. It is however possible to configure
it so that it stays high all the time and just briefly pulses low for
250 ns when a write operation is completed, or when erasure is
completed, or both. This would be usefull if we had wired that pin to
generate edge-triggered interrupts. Only, we can't because interrupts
are level-trigered in the TI-99/4A.

To set the configuration, you would write the \>B8 command to any
address, followed with the configuration code: \>00 for default
behaviour, \>01 to pulse once erasure is completed, \>02 to pulse once
writing is completed, \>03 to pulse in both cases.

####  Additional commands

There are two addtional command, not explained in the StrataFlash data
sheet: 'Protection program' and 'Set read configuration'.

Protection program involves writing command \>C0 at any address, then
writing a data word at the required address.

Set read configuration involves writing command \>60 (set/clear lock
bits) at any address, followed with data \>03 at a special address
dubbed RCD. And I haven't the faintest idea what it does.

All other commands words are reserved for future versions and should not
be used.

##   SmartMedia operations

[Read data
](#SM%20read)[Read status
](#SM%20status)[Read chip ID
](#SM%20ID)[Reset
](#SM%20reset)[Write data
](#SM%20write)[Multiple write
](#SM%20multi-write)[Erase block
](#SM%20erase)[Mutiple erase](#SM%20multi-erase)

### Introduction

Smartmedia cards contain so-called NAND-Flash EEPROMs. These resemble
GROMs in that all bytes are accessed through a single port: you pass an
address to the Flash, then read (or write) consecutive bytes. The
differences with GROMs are a) that you cannot read back the current
address, and b) that you must first write a command (e.g. read bytes)
before you can actually access the card.

There are thus three "ports" to a Smartmedia card: a bidirectional data
port, a write-only adress port and a write-only command port. Each of
these are 8 bits-wide.

In the USB-SM card, these ports map at 4 distinct addresses in the DSR
space. The data port is split in an "input-only" and an "output-only"
ports. This is necessary because the TMS9900 microprocessor always
performs a read before each write, which would confuse multiple byte
operations. All ports are 8-bit wide, so the least significant byte
should be ignored. The addresses are:

\>4FF0: Read data
\>4FF8: Write data
\>4FFA: Write address
\>4FFC: Write command

In addition the following CRU bits are used:

Bit 3 (output). Set to '1' to enable the card. Set to '0' to place the
card in standby mode (default upon power-up).
Bit 5 (input). 0: card is busy. 1: card is ready
Bit 6 (input). 0: no card inserted. 1: a card is in.
Bit 7 (input). 0: card is write-protected. 1: card can be written to.

The card memory is divided into "sectors" of 512 bytes (although some
early cards used 256-byte sectors). Each sector is accompanied by 16
bytes of extra information (8 bytes for 256 bytes/sector cards): these
can be used for cyclic-redundancy check, "bad sector" mark, or anything
you want. You can begin reading at any byte in any sector. Similarly,
you can begin writing anywhere. However, because it's EEPROM memory,
corrections are often impossible: you can turn a '1' bit into a '0' but
not the opposite. If you need to reprogram a sector, you'll have to
erase it first.

But because its a "flash" EEPROM, you cannot erase an individual byte,
not even an individual sector: you must always erase at least a block of
32sectors (for Toshiba cards.Samsug cards tend to use only 16 sectors
per block). Sectors are attributed to blocks in numerical order, i.e.
sectors 0-31, sectors 32-63, etc. To speed things up when erasing a
large number of sectors, it is possible to send a command that will
erase more than one block. However, the blocks must be located in
different districts. There are 4 districts, each comprising one block
out of four, in alternance.

To begin any operation, whether read, write, erase or read status, you
must send a command byte, optionally followed with an address. Depending
on the card, addresses can be 3, 4 or 5 bytes. Then you would typically
read or write some bytes, and possibly send a termination command. Valid
commands (for Toshiba cards) are the following. Be aware that sending
any other command might corrupt the contents of the card.

    >FF: Reset
    >00: Read
    >01: Read (offset 256)
    >50: Read sector info
    >80: Place data into write buffer
    >10: Write buffered data
    >11: End buffering, but don't write
    >15: Write buffered data, don't reset status
    >60: Prepare for erasing
    >D0: Begin erasing
    >70: Read status
    >71: Read status after multiple write/erase
    >90: Read chip ID
    >91: Read extended ID

Commands should only be sent when the chip is ready, which can be
checked by making sure CRU bit 7 is '1'. The only exceptions are the
reset (\>FF) and the status (\>70 and \>71) commands, which are accepted
while the chip is busy.

The SmartMedia selection bit, CRU bit 3, should remain low until the
command is completed. Bringing it high will deselect the card, and
generally result in aborting the operation.

Important: a new card is not supposed to be 100% functional.
There might be some bad sectors on it (upto 160, for a 8192-sectors
card). So the first thing to do when formatting a new card is to read it
entirely: any sector that contains anything else than \>FF bytes is bad
and should not be used (i.e. do not try to erase it). You may use one of
the info bytes (the 6th one, by convention: \>00 indicates a bad block)
to flag bad sectors, or implement a "bad blocks table" at the beginning
of the chip. If you decide to erase the chip later on, you must first
save the bad blocks table, so that it can be reconstituted afterwards.

####  Read data commands

There are three read commands: \>00, \>01 and \>50. After you write the
command to the command port, you must write a 3- to 5-byte (depending on
the card) address to the address port, then you can start reading bytes
out of the data port.

The first byte in the address is the offset of the first byte you want
to read in a sector. The next bytes are the number of this sector. (NB
few cards have 5-byte addresses. Most 4-byte cards will ignore the 5th
byte, so it won't hurt to send a \>00 anyway. On the other hand, some
3-byte cards do not like extra address bytes: thay cancel the command!).

    Write command >00 at >4FFC
    Write byte offset at >4FFA
    Write sector # LSB at >4FFA
    Write sector # byte 2, at >4FFA (if needed)
    Write sector # byte 3, at >4FFA (if needed)
    Write secor # MSB at >4FFA
    Read bytes from >4FF0

You'll note that there is a slight problem with the above scheme, a
survivance from the time sectors were 256-byte in length: the first byte
to read within the sector must be passed as a byte, i.e. a number
between 0 and 256, yet sectors are 512 bytes! This means that you can
only start reading within the first half of a sector. To overcome this
problem, a second read command (opcode \>01, instead of \>00) is
provided. This command automatically add 256 to the byte offset, so you
start reading in the second half of the sector.

In both cases, if you keep reading past the end of the sector, you'll
reach the info bytes. After these, you will read the next sector, from
its first byte to the last, then its info bytes, etc. (Note: the chip
needs some time to change sectors, about 25 microsecond. During that
time CRU bit 7 will be low. But given the duration of the average copy
loop in assembly language, you probably don't need to worry about this).
You can go on reading this way upto the limit of the block. Then, to
move into another block, you will need to resend the read command.

A third read command (opcode \>50) serves to read the sector info bytes.
Since there are only 16 such bytes, the byte offset is limited to
\>00-0F. In this case, if you attempt to read more bytes, you will not
start reading the next sector. Rather, you will read the info bytes for
the next sector. In other words, command \>02 always reads sector info,
never sector data. It's thus very usefull to look for bad sectors, free
sectors, etc.

Theoretically, you don't need to re-issue a command and long as you keep
reading in the same half-sector: you could just send another address and
keep reading. However, it's good practice to send the proper command
each time you want to change the address.

####  Read status commands

There are two status read commands: \>70 and \>71, the latter being used
for multiblock programming. To read the status, you must write the
command to the command port, then retrieve one byte of data from the
data port.

With command \>70, only 3 status bits are used:

    >01: write/erase failed (only valid if the chip is ready)
    >40: ready
    >80: not protected

With command \>71 (to be used after multiple write/erase), four more
bits come into use:

    >01: write/erase failed (global result)
    >02: write/erase failed in district 0
    >04: write/erase failed in district 1
    >08: write/erase failed in district 2
    >10: write/erase failed in district 3
    >40: ready
    >80: not protected

Note that the chip remains in status mode after the command in
completed. So to return to read mode, you must issue a read command.

####  Read ID command

There are two ID read commands, \>90 and \>91, which each let you
recover some information about the type of chip within the card.

To read the chip ID, first write command \>90 to the command port, then
write \>00 to the address port. You can then read two bytes of data out
of the data port. The first byte is the manufacturer ID (e.g. \>98 for
Toshiba), the second is the chip ID (e.g. \>79 for a TH58100FT).

The procedure is identical for command \>91, except that you can only
read one byte of data: the extended ID code (\>21 indicates a Toshiba
chip that supports multiple writes and erases). Older cards do not
support the extended ID feature, in which case you are likely to read
back the last byte you wrote to the card (this echo phenomenon is due to
the 74LVT245 buffers), in our case \>00.

Example:

      LI   R0,&gt;9000         Command &quot;read ID&quot;
      MOVB R0,@&gt;4FFC        Write it
      CLR  @&gt;4FFA           Set address as 0
      CLR  @&gt;4FFA
      CLR  @&gt;4FFA
      CLR  @&gt;4FFA           If address is 4 bytes
      CLR  @&gt;4FFA           If address is 5 bytes
      MOVB @&gt;4FF0,@MFG      Save manufacturer code
      MOVB @&gt;4FF0,@DEV      Save device code
      SBZ  3                Back to standby mode (optional)

####  Reset command

Issuing the reset command \>FF will abort all pending writing/erasing
operations. It may take some time (upto 1/2 a second) to unload the high
voltage that's internally generated for writing or erasing a chip. By
monitoring CRU bit 7, you can tell when the reset is completed: the bit
goes back to 1. Alternatively, you could use the status read (\>70)
command to monitor the busy bit.

It is highly recommended to issue a reset command after a power-up (in
case some garbage might have been sent to the card during power-up, and
mistaken for a command).

####  Write command

To write data into the SmartMedia chip, you must first send the program
command (\>80) to the command port, then send the 4-byte address of the
first byte to program to the address port, then pass the appropriate
number of bytes to the data port. At this point, the data is only stored
into a RAM buffer within the chip. There is room for upto 528 bytes, so
that you can write the whole sector plus its 16 bytes of info. Older
cards only feature 256 bytes per sector, plus 8 extra info bytes.

Once all the data has been sent, write the confirmation command (\>10)
to the command port. The chip will start "burning" the data you just
sent into its EEPROM. During that time, CRU bit 7 will remain low, so
you can tell when the chip is done by monitoring this bit.

Once writing is completed, it's generally a good idea to perform a
status check (command \>70) to make sure everything went all right.

    Write command >80 at >4FFC
    Write byte offset at >4FFA
    Write sector # LSB at >4FFA
    Write sector # byte 2, at >4FFA (if needed)
    Write sector # byte 3, at >4FFA (if needed)
    Write secor # MSB at >4FFA
    Write data bytes at >4FF8
    Write termination command >10 at >4FFC

Notes:

- If you don't mean to write a whole sector, you can send only part of
  it. However, you should still send 528 bytes (or 264 for older cards)
  of data with the command. Just pass \>FF bytes for the parts of the
  sector that you want to leave untouched. You can then come back later
  and program another part, passing \>FF bytes for the part you already
  did. But you can only do this 3 times, I don't know why.
- It is possible to program only the info bytes, by first performing a
  read with the \>50 command, then a write as described above.
- The manufacturer insits on sectors being used in order within a block,
  i.e. first sector 0, then sector 1, etc. As opposed to a traditonal
  NOR-Flash EEPROM, in which you can program sectors in any order you
  like.
- Obviously, writing is not possible if the card is write-protected. So
  always check CRU bit 6 before attempting to write to the card. If this
  bit is '0' the card is write-protected. It might be possible to ignore
  the protection and program the card anyhow, depending wether the user
  has closed the appropriate DIP-switch on the USB-SM card.

####  Multiple write command

You can write upto 4 sectors at a time. if the SmartMedia card is built
around a chip that supports multiple write (check ID with command \>91).
The only catch is that each sector must be in a different "district".
There is a total of 4 districts in the chip, each comprising one erase
block (of 32 sectors for Toshiba cards) out of four. So district 0
comprises blocks 0, 4, 8, etc, whereas district 1 comprises blocks 1, 5,
9, etc; district 2 comprises blocks 2, 6, 10, etc; and district 3
comprises blocks 3, 7, 11, etc.

    Sectors  Block  District
     >00-1F   00     0
     >20-3F   01     1
     >40-5F   02     2
     >60-7F   03     3
     >80-9F   04     0
     >A0-BF   05     1
     >C0-DF   06     2
     >E0-FF   07     3
            etc

To perform a multi-block write, send a write command with address and
data as described above, but use \>11 instead of \>10 as the termination
command. This tells the chip that there is more to come (the chip may
become busy for a short while at this point: check CUR bit 7). You can
then repeat the whole procedure for the second sector, the third, etc
(in any order). Only for the last sector will you use the \>10
termination command, which triggers programming.

Alternatively, you could use \>15 as a termination command. The
difference is that, with \>15, the chip won't update its status register
before it starts the next write operation. So if you were to program the
whole chip, you could chain 32 multiple writes (4-sector each,
terminated with \>15, except for the last one terminated with \>10) and
check the status at the end with command \>71.

Note that status command \>71 should be used with multiple writes,
because it details the results for each district.

In summary, the command sequence for such a giant write would be ( ...
means: send address and data):

    >80...>11   >80...>11   >80...>11   >80...>15
    >80...>11   >80...>11   >80...>11   >80...>15
          (repeated upto 32 times)
    >80...>11   >80...>11   >80...>11   >80...>10
    >71...

The only other command accepted after an \>80 is reset (\>FF), to abort
the sequence in case you made a mistake.

####  Erase command

SmartMedia cards are actually nothing more than an extra flat
Flash-EEPROM chip embedded into a fancy looking card. Which means that,
like every Flash, it can only be erased blockwise. On most Toshiba
cards, a block is defined as a set of 32 sectors: block 0 is sectors
0-31, block 2 is sectors 32-63, etc. For Samsung, a block is generally
16-sectors long.

To trigger erasure, write the erase command (\>60) to the command port,
followed with the 3-byte address of the block into the address port. The
address in only 3 bytes in this case, because we don't need to specify
the offset of the first byte: the whole block is erased anyhow. For
older cards with 3-byte addresses, you would only need to send the last
2 address bytes, whereas 5-byte addressed cards require only 4 of them
for erasing.

Then you must write the confirmation command \>D0 to the command port.
The chip will begin erasing the block. During that time, CRU bit 7 will
remain low, so you can monitor this bit to know when the chip is
finished. At this point, it's a good idea to perform a status check
(command \>70) to make sure everything went well.

####  Multiple erase command

Just like you can write upto 4 sectors at a time, you can also erase
upto 4 blocks together. Again, this is only possible if the blocks
reside in different sectors.

To initiate a multiple block erase, issue a regular erase sequence
(command \>60 and address) but do not send the confirmation code yet.
Instead, issue another erase command, a third, and even a fourth. Once
you are done, send the \>D0 confirmation command. The chip will erase
all 4 blocks in the same time it would take to erase only one. So this
is a neat trick to speed up erasure.

Once the chip is done, CRU bit 7 will become 1 again. At this point, you
should perform a status check, preferably with command \>71 so you can
tell in which district a problem occured, if any.

##  USB operations

[Brief USB primer
](#USB%20primer)[\_Data encoding
](#data%20encoding)[\_Device structure](#device%20structure)

**[The ISP1161
](#The%20ISP1161)**[\_The root hub controller](#Root%20hub)

[The host controller
\_Command registers
](#Host%20controller)[\_Accessing the stacks
](#Stack%20access)[\_PTD structure
](#PTD%20structure)[\_Managing interrupts
](#Int%20management)[\_Managing frames
](#Frame%20management)[\_Miscellaneous registers
](#Misc%20registers)[\_Host controller register
summary](#HC%20reg%20summary)

[The device controller
](#Device%20controller)[\_Initialization
](#DC%20init)[\_Data flow
](#DC%20data%20flow)[\_Managing interrupts
](#DC%20ints)[\_Managing DMA
](#DC%20DMA)[\_Miscellaneous registers
](#DC%20misc)[\_Device controller register summary](#DC%20reg%20summary)

###  Brief USB primer

The Universal Serial Bus (USB) is a host-centric tiered-star type of
bus. What this means is that there is only one host in the system, which
initiates all transactions. A device thus cannot address the host,
unless prompted for it. Which means that the host must constantly poll
all the devices to see if one of them has something to say. But don't
worry: this chore is taken care of by the host controller and you wont
have to program it yourself.

Each device is connected to the host via a dedicated cable. Since the
host can handle upto 127 devices, a USB card would need 127 plugs! Not
very convenient, is it? To solve this problems, intermediary processors
called "hubs" can merge the connections from several devices into a
single connection to the host. But from the host's point of view, each
device is still independently connected: the hub acts as a proxy for the
host. And of course, hubs can be connected to other hubs...

Physically, each USB connections consists in 4 wires: 2 for the signal
and 2 for power supply. That's right, USB is a powered bus: wire \#1
carries +5 volts (#4 is the ground), which allows for devices that do
not have their own power supply. At power-up time, such devices must
negotiate with the host to determine how much power they'll be allowed
to draw from the bus.

The fact that every device has its dedicated line allows the host
controller to detect the presence of a device, as soon as it's plugged
in. Here is how it works: the host card pull both lines down with weak
pull-down resistors (15K), the device pulls one of the lines up with a
stronger resistor (1.5K): the DM line if the device only handles
half-speed, the DP line if it can handle full speed or high speed (which
of the two is chosen must be negotiated with the host by software).

####  USB data encoding

First, there are 3 possible transmission speeds: low-speed (USB 1 only,
1.5 Mbits/sec), full-speed (USB 1 and 2, 12 Mbits/sec), and high-speed
(USB 2 only, 480 Mbits/sec). The ISP1161 is a USB 1.1 chip, so it won't
do high speed, but it is USB 2.0-compliant as far as full-speed is
concerned.

The signal is encoded on the two signal lines DP and DM (a.k.a. D+ and
D-) that toggle in a differential manner, i.e. one is high (+3.3 volts)
while the other is low (0 volts), although there are a few special
conditions when both lines can have the same value (e.g. to indicate and
end of packet or a reset), or when lines are left "floating" (idle
state). Data is encoded by a "non-return to zero invert" encoding
scheme, which means that lines toggle to indicate a '0' bit and don't
toggle for a '1'. So that synchronization is not lost over a long
stretch of 1s, the sender includes an extra 0 is included after six 1s,
which will be removed by the recipient. This is known as "bit stuffing".
You do not have to worry about these details, it's all taken care of by
the serial engine in the ISP1161.

USB data are always transmitted as packets. There can be four type of
packets: frame packets, token packets, data packets, and handskake
packets. In general, a USB transfer comprises a token packet which
serves as a header, one or more data packets, and a handshake packet for
aknowledgement purposes.

Frame packets are used for synchronization, which is especially
important with isochroneous (real time) transfers. The host sends one
such packet every millisecond. They comprise the following bit fields:

- Synchronization (8 bits)
- Packet ID (SOF, 8 bits)
- Frame number (11 bits)
- Cyclic redundancy check (5 bits)
- End-of-packet mark (3 bits, one with both lines low)

Token packets serve as headers for a transaction: they define the type
of transaction and the target device. They are always generated by the
host. The bit fields are

- Synchronization (8 bits)
- Packet ID (SETUP, IN or OUT, 8 bits)
- Device number (7 bits: 0-127. All devices answer to number 0)
- Endpoint number (4 bits: 0-15. All devices must have endpoint 0)
- Cyclic redundancy check (5 bits)
- End-of-packet mark (3 bits)

Data packets contain the payload. They can be generated by the host (for
an OUT transaction) or by the device (for an IN). The amount of data is
limited to 8 bytes at low speed, and 1023 bytes at full speed. If more
bytes are to be transfered, additional data packets will be sent,
alternating the packet ID between DATA0 and DATA1 so that a missing
packet can be spotted. The bit fields are:

- Synchronization (8 bits)
- Packet ID (DATA0 or DATA1, 8 bits)
- Data (various number of bytes)
- Cyclic redundancy check (16 bits)
- End-of-packet mark (3 bits)

Handshake packets serve for the recepient to acknowledge the
transaction. They comprise the following bit fields:

- Synchronization (8 bits)
- Packet ID (ACK, NAK, STALL or NYET)
- End-of-packet mark (3 bits)

The ISP1161 takes care of all the burden of building packets and sending
them. However, to do so it needs some information from you:

- The type of token you want (IN, OUT or SETUP)
- The device number and endpoint number of the target
- The number of bytes to transfer
- The maximum number of bytes to include in a single data packet

This information is included is a structure called a PTD (Philips
Transfer Descriptor) that you pass to the host controller together with
your data.

You don't have to worry about frame packets: the host controller sends
them automatically. Although you can influence the process if you want
to, by altering a few dedicated registers in the ISP1161.

The host can generate four types of transactions: control, bulk,
interrupt and isochroneous. Control transactions are used to control a
device, query its structure, select its configuration, etc. Bulk
transfers are used to transfer data from/to the device at the initiative
of the host program, e.g. to access a disk drive. Interrupt transfers
are used by devices which must ring up the host computer when something
happens, e.g. when you type on a keyboard (since USB is host-centric,
the host controller must constantly poll the devices for this type of
transfer). Isochroneous transfers are used for real-time data transfer,
e.g. from a webcam.

In general, a transaction consists in 3 stages, each comprising at least
3 packets (token, data and handshake):

1.  A setup stage, where a request is sent (by the host)
2.  An (optional) data stage, during which data is transfered. This
    stage can contain more than 1 data packet.
3.  A status stage, in which the device reports the result of the
    transaction.

You will need to take care of this yourself, by passing three PTDs to
the host controller. The good news is that, if more than one data packet
is required, the host controller will take care of sending them: you
don't need to pass any extra PTD for that.

#### Logical structure of an USB device

Software-wise, a device consists in upto 16 "endpoints" that the host
can address. You can envision endpoints as different workers in the same
office. For instance, you can have a different endpoint for each type of
transfer. Every device must have endpoint 0, because it's the one the
host will address to query the device about its configuration. The
logical connection between the host and a given endpoint in a device is
known as a "pipe". Characteristics of the pipes are direction of
transfer, type of transfer, and speed.

Endpoints are arranged in a hierarchical manner: several endpoints are
grouped in one or more "interface" (although this feature is rarely
used: in general there is only one interface per configuration). Several
interfaces make up a "configuration" and a "device" can have several
configurations. On top of this, a given piece of hardware can contain
more than one device (e.g. a fax machine could have a scanner device, a
printer device and a modem device), which is why USB people like to talk
about USB "functions" rather than devices.

At power up time, the host queries each device for its configuration(s),
and for a description of each interfaces and endpoints, then it selects
the most appropriate configuration and assigns a unique number to each
device. Each time a new device is connected (which the host controller
detects by seeing one data line pulled up) the host should query it.
This is a chore that you must perform yourself. The asnwers from the
various devices should then be stored in RAM for future reference.

For more details, consults the excellent "USB in a nutshell" webpages by
Craig Peakcock, at
[www.beyondlogic.org/usbnutshell/usb1.htm](http://www.beyondlogic.org/usbnutshell/usb1.md)

##  The ISP1161

The ISP1161 actually consists in three different controllers:

- A host controller which controls transactions with USB devices
- A hub controller, which physically handles the two device ports
  present on the card
- A device controller, which makes the card look like a USB device, for
  connection with another computer.

The three controllers operate in a completely independent manner, and
according to slightly different software models.

The interface between the host controller and the TI-99/4A consists in a
whole bunch of registers. To talk to the controller, you pass the number
of the register you want to access to the host command port (\>5FFA),
then you read its contents from the host data port (\>5FF0). To write to
a register, you must pass its number plus \>80 to the command port (a
bit like writing to VDP memory, isn't it?), then write its new content
to the host data port at \>5FF8 (write-only address).

The hub controller is integrated within the host controller, and is
accessed in the same manner via 5 dedicated registers.

The interaction with the USB devices is mediated via a First-In
First-Out (FIFO) stack in the host controller's memory. There is one
register that constantly points at the top of the stack, you use it to
place on the stack the data that you want to send, together with a
header called a Philips Transfer Descriptor (PTD) that tells the host
controller what to do with the data. And then you can forget about it:
the controller will send the data to the device at its leisure. To
reveice data, you just place the appropriate PTD on the stack, together
with some dummy bytes to reserve space, and come back later to read the
data that came in.

Things are a bit more complicated for isochroneous transfers, because
the host controller cannot interrupt itself to let you access the stack.
So there are two stacks (independent of the normal stack), arranged in a
"ping-pong" manner: while the controller is using one, you can access
the other. A dedicated register point to the top of the currently
available stack. There is a total of 4 Kbytes to be divided between the
3 stacks at power up time. The ping-pong stacks must have the same size,
but the regular stack can differ in size. Depending on your system, you
may devote a lot of memory to the ping-pong stacks, or just part of it,
or none at all.

The interface between the device controller and the TI-99/4A follows a
"function" paradigm: you write a function number to the device command
port (\>FFFE) and write parameters to the device data port (\>FFFC,
write only), or retreive the function result from the device data port
(\>FFF4). Which, you will have noticed, is the same as for the host and
hub controllers, just with different names...

Internally, the device controller consists of 16 endpoints. At power-up
time, you must tell the device controller which endpoints you want to
use, how much memory to devote to each, which type of transfer it
handles, and in which direction. The first two enpoints must be endpoint
0, in the IN and OUT direction (each uses 64 bytes), the others are up
to you. There is a total of 2462 bytes to be shared between the
endpoints that you elect to have. Each endpoint will thus have its own
independent FIFO stack.

Example: Reading the chip ID

      MOV  R0,@&gt;5FFA        Write register number (to be read)
      MOV  @&gt;5FF0,@HOST     Save host controller ID
      LI   R0,&gt;00B5         Number for &quot;Read chip ID&quot; function
      MOV  R0,@&gt;5FFE        Write function number
      MOV  @&gt;5FF4,@DEV      Save device controller ID

For more information, refer to the ISP1161A1 data sheet, which you can
find on the [Philips
website](http://www.semiconductors.philips.com/pip/ISP1161ABD.html). I
have a copy [here](isp1161.pdf) (pdf file, 3 Mbytes), but be aware that
it might not be the most recent version.

When it comes to writing higher level DSRs, you will find a very usefull
application note on the Philips website. Again, I have a copy
[here](an1161.pdf) (pdf file, 781 Kbytes) which may not be the most
recent version.

###  The root hub controller

Hubs serve to split a single USB connection from the host into two or
more device connections (the maximum is 15). Hubs themselves are a class
of USB devices and appear as such to the host.

The main functions of a hub controller are:

1.  To mediate traffic between the upstream host and the downstream
    devices. To this end, it can enable or disable a port, suspend or
    resume it, and reset it.
2.  To manage power and distribute it to the devices that need any.
    There are 3 power options: always on, global control, or per-port
    control.

The root hub controller integrated within the ISP1161 handles these
functions for two device ports. Its inteface to the host is a bit
primitive in that it won't answer USB requests for descriptors, process
USB requests, etc. Instead, you access it via registers integrated
within the host controller. The rest has to be emulated by software, but
that's no big deal: when you create your descriptor tables, you can
initialize it with the values corresponding to the root hub controller.
The few parameters that can vary can be extracted from the dedicated
registers.

The root hub controller comprises five registers: two descriptor
register, a hub status register, and two port status registers (one for
each port).

####  HubDescriptorA register \>12 (32-bit)

This register is used to configure the hub controller. It consists in
the following bit fields:

    Bit value  Meaning
    xx00 0000  How long to wait before accessing a port that was just powered on. >01 = 2 msec.
    0000 0100  1 = no overcurrent (OC) protection reported.
    0000 0080  0 = OC condition reported globally. 1 = OC reported per port.
    0000 0040  0 = hub is not a compound device. Always 0.
    0000 0020  1 = all ports always powered.
    0000 0010  0 = all ports powered together. 1 = power controlled per port.
    0000 000x  Number of ports. 1 or 2 depending on the wiring of the NDP_SEL pin.

If you select global overcurrent report, OC conditions will be reported
in the hub status regiser, otherwise they'll be reported in the
corresponding port status register.

If you select global powe control, you can toggle power on/off for all
ports via the hub status register. If you select a per-port power
control, a bitmap located in descriptor register B lets you decide if a
port will be controlled via the hub status register, or via its own
status register.

My advise is to select both OC and power control per port. With only two
ports, the overhead is insignificant and the extra amount of control is
worth it. I'm not sure about how long to wait after powering a port,
\>FF is the safest value (about 1/2 a second) but it's probably
uselessly high...

Suggested initial value: \>8000 0812

####  HubDescriptorB register \>13 (32-bit)

This register holds two bitmaps: in the upper word is a power control
bitmap. Each bit corresponds to one port, if the bit is '1' the port is
controlled globally, via the HubStatus register. If the bit is '0', the
port is controlled individually, via its own PortStatus register.

The lower word holds a similar bitmap that indicated whether the
attached device is removable or not. A '0' means removable, a '1' means
not removable.

Since the ISP1161 handles only two ports, only 2 bits (or of 15) are
meaningful.

    Bit value  Meaning
    0002 0000  Port #1 controlled globally.
    0004 0000  Port #2 controlled globally.
    0000 0002  Port #1 has non-removable device.
    0000 0004  Port #2 has non-removable device.

I suggest you initialize the upper word as \>0006, so that both ports
can be controlled individually.

####  HubStatus register \>14 (32-bit)

Note that the meaning of a bit may be slightly different when it's read
than when it's written.

    Bit value  When read                           When written
    8000 0000      -                               1: resets bit 0000 8000
    0002 0000  1=bit 0000 0002 has changed         1: clear bit 0002 0000
    0001 0000  Always 0                            1: turn power on  "globally"
    0000 8000  1=connecting device wakes up host   1: set bit 0000 8000
    0000 0002  1=overcurrent detected (global mode)      -
    0000 0001  Always 0                            1: turn power off "globally"

####  PortStatus registers \>15 and \>16 (32-bit)

Register \>15 is used to check and control port \#1, register \>16 plays
the same role for port \#2. For many of the status bit, a "change"
feature is available: a bit in the upper word indicates whether the
corresponding bit in the lower word has changed recently. You can clear
these reporter bits by writing a '1' to them (writing a '0' has no
effect), so that next time you check the register you will know if one
of the status bit has toggled back and forth.

Here again, the meaning of a bit changes depending whether it's read
(status) or written to (command). Only '1' bits are meaningfull when
writting: writing a '0' has no effect.

    Bit value  When read                           When written
    0010 0000  1=port reset completed (10 msec)    1: clear bit 0010 0000
    0008 0000  1=bit 0000 0008 has changed         1: clear bit 0008 0000
    0004 0000  1=bit 0000 0004 has changed         1: clear bit 0004 0000
    0002 0000  1=bit 0000 0002 has changed         1: clear bit 0002 0000
    0001 0000  1=bit 0000 0001 has changed         1: clear bit 0001 0000
    0000 0200  1=low speed device                  1: turn power off (if per-port)
    0000 0100  1=port power is on                  1: turn power on (if per-port)
    0000 0010  1=resetting port                    1: reset the port
    0000 0008  1=overcurrent detected (per port)   1: resume suspended port
    0000 0004  1=port is suspended                 1: suspend port
    0000 0002  1=port is enabled                   1: enable port
    0000 0001  1=device connected                  1: disable port

Note that the controller is smart enough to reject meaningless commands.
Namely, you cannot enable, suspend or reset a port if no device is
connected to it. Instead of setting the corresponding status bit, the
controller will report the error by setting bit 0001 0000.

### The host controller

The host controller manages the USB bus and controls all the devices
connected to it via the hub controller. The host controller is
responsible for initiating every USB transaction, including in the
device-to-host direction. It also generates a "Start-of-frame" packet
every millisecond, for bus synchronization purposes.

### Command registers

#### HardwareConfiguration register \>20 (16-bit)

This register is used to setup hardware options. These are best set a
power-up time and left untouched from there on. I recommend that you
keep the default values, except for the pull-down resistors and the OC
detector (we are using the internal ones) for INT1, which should be
enabled.

    Bit   Meaning
    1000  1: Use internal 15K pull-down resistors. 0: There are external resistors (default).
    0800  0: Can stop clock when suspended (default). 1: Don't stop clock.
    0400  1: Use onchip overcurrent detector. 0: There is an external circuit (default).
    0100  0: Regular DACK mode (default). 1: Reserved, do not use.
    0080  0: EOT active low (default). 1: EOT active high.
    0040  0: DACK1 active low (default). 1: Reserved.
    0020  0: DREQ1 active low. 1: DREQ1 active high (default).
    0018  Data bus witdth. 08: 16-bit (leave it so).
    0004  0: INT1+2 active low (default). 1: Active high.
    0002  0: INT1+2 stable (default). 1: Pulsed (i.e. edge-triggered).
    0001  0: INT1 disabled (default). 1: INT1 active.

####  DMAconfiguration register \>21 (16-bit)

This register is used to setup options for Direct Memory Transfer (DMA),
a mode in which data can be tansfered to/from the computer memory
directly, without having the cpu doing the job. Currently, this is not
possible with the TI-99/4A, but I'm thinking of designing a DMA board
that would allow it, and the necessary circuitery was included on the
USB-SM board.

The ISP1161 allows two types of DMA: the classical Intel 8237A type
(which is the one we want) and the Motorola DACK-only type. The type of
DMA and the polarity of all three signals ( DREQ, DACK and EOT) is
programmable in the HardwareConfiguration register.

To perform DMA, you would load the number of bytes to be transfered in
the TransferCounter register, as usual. Then you would program the DMA
controller board, so that it know what to do with the data. Finally, you
would set the DMAconfiguration register, therby triggering the DMA
process.

    Bit   Meaning
    0060  Burst length. 0: 1-cycle, 2: 4-cycle, 4: 8-cycle, 6: reserved.
    0010  1: Enable DMA (will be reset once transfer is completed).
    0004  1: Use byte counter (TransferCounter register) 0: use external EOT signal.
    0002  1: Use ATL stack. 0: Use current ILT stack.
    0001  1: Write to stack from TI-99/4A memory. 0: Read from stack to TI-99/4A.

####  CommandStatus register \>02 (32-bit)

Even though it's 32-bit, this register only implement 3 bits. Two bits
indicate how many time (upto 4) a scheduling overrun has occured so far
(this happens when a frame boundary occurs before an ongoing
transmission was completed). One bit is used to cause a software reset
of the host controller, which will return most registers to their
default values and stop USB traffic.

    Bit value  Meaning
    0003 0000  Number of scheduling overruns (0-3, rolls over to 0).
    0000 0001  1: Reset the host controller (not the root hub).

####  Control register \>01 (32-bit)

This register is mostly used to change the USB operational state of the
host controller, but there are also two configuration bits that decide
which external event will wake up a suspended controller.

    Bit value  Meaning
    0000 0400  1: Remote wakeup signal is enabled.
    0000 0200  1: Remote wakeup signalling is supported (0=power-up value).
    0000 00x0  0: Reset. 4: Resume. 8: Operational. C: Suspended

Moving to an "operational" state will cause a start-of-frame to be sent
1 millisecond later. A reset state can also be triggered by a software
reset, or a hardware reset. The controller can move by itself from
"suspended" to "resume" if a remote wakeup signal is sent by a device.

####  SoftwareReset register \>29 (16-bit)

Writing \>00F6 to this write-only register resets all registers in the
host controller, but does not affect the ATL and ITL stacks.

###  Accessing the stacks

As mentionned earlier, all USB transfer is handled via internal RAM
buffers, organized a FIFO (first in, first out) stacks. There is one
stack for acknolwdged transfers, and two "ping-pong" stacks for
isochroneous transfer. The amount of space devoted to these stacks
should be determined at power-up time by setting up registers \>2A and
\>2B (although it can always be changed later on).

To access the ATL (Acknowledged Transfer List) stack, you must first
write the number of bytes that you intend to read or write into register
\>22. Then you can read or write the bytes from register \>41. To do so,
you pass the register number to \>5FFA (\>40 to read data, \>C0 to
write), then you transfer the proper number of byte to/from \>5FF8. This
is true whether you use regular memory transfer or DMA.

The procedure is the same for the ITL (Isochroneous Transfer List)
stacks, except that the register number is \>40. The ISP1161 decides
which of the "ping" or the "pong" stack you will access through this
register.

The ISP1161 maintains internal pointers to the top of the ATL and ITLs.
Every time you write data to a stack, the corresponding pointer moves
up, when you read data , the pointer moves back down. The ISP1161 itself
can access the stacks without affecting the pointers. In fact it does it
all the time, looking for PTDs with the "active" bit set, i.e. that need
to be processed. When imputing data it's obvious that you have to read
it from the stack, but when sending out data, you may be tempted not to
read it back, or to just read back the PTDs so you know the error code.
However, doing this would prevent the pointer from coming back to the
bottom of the stack: after each such operation you would push the poiner
further ahead, until the whole stack is full. If you have only the ATL
stack, it may be ok, as the pointer will roll over to zero, but if you
are using several stacks, you'll end up overwriting the next one! So the
rule is: always read back the same number of bytes that you have
written: PTD and all.

A very important register is the BufferStatus register, which indicates
the status of the 3 stacks so you can know when to go and read or write
one of them. In addition, there are two registers which tell you how
many bytes are waiting to be read in which ITL stack. For some reason,
there is no such register for the ATL stack (I guess, you're supposed to
know what's waiting for you there, since you set up the PTD for it...).

The drag with these stacks is that they are stacks of bytes, accessed
word-wise, and aligned by double-words! To add insult to injury, Philips
used the Intel byte order, with the odd-address stack byte in the most
significant byte of the data bus and the even address stack byte in the
least significant data bus byte. In practice, this means that data bytes
are swapped during transfer! The dword alignment is not too much of a
problem: it just means that the total number of bytes transfered must
always be a multiple of 4. If it isn't, just add padding bytes.

There is an additional problem with the ITL stacks: the host controller
emits a frame packet every millisecond, at which time it switches the
ping-pong stacks. It expects you to read back the old ITL stack, while
it's using the other one. You don't need to read more than one word, but
you must do it in time. If a new frame begins before you read the ITL
stack, the host controller considers both ITL stacks as full and stops
isochroneous transmissions. From that point on, you can only access ITL1
until you perform a reset.

####  ATLbufferLength register \>2B (16-bit)

In this register you indicate the amount of RAM that you wish to
dedicate to acknowledged transfers, in the form of the ATL stack. The
maximum value is \>1000, which is 4 Kbytes.

####  ITLbufferLength register \>2A (16-bit)

In this register you indicate how much RAM you want to dedicate to each
of the two ITL stacks used for isochroneous transfers. The actual amount
of RAM used for this kind of transfer will thus be twice the number that
you write (because there are two ITL stacks). Thus, the maximum value is
\>800, or 2 Kbytes.

The RAM is shared by the ATL stack and both ITL stacks, so make sure the
total of ATL + 2\*ITL does not exceed \>1000.

It is quite acceptable to use use the whole RAM for the ATL stack
(sizes: \>1000 and \>0000 bytes) or for the ITL stack (\>0000 and \>0800
bytes). Or you could split it between the two types of transfer, for
instance \>800 bytes for the ATL and \>400 bytes for either ITL.

####  BufferStatus register \>2C (16-bit)

    Bit   Meaning
    0020  1: ATL done. Data read by host controller.
    0010  1: ITL1 done. Data read by host controller.
    0008  1: ITL0 done. Data read by host controller.
    0004  1: ATL full.
    0002  1: ITL1 full.
    0001  1: ITL0 full.

####  ReadbackITL0length register \>2D (16-bit)

This register contains the number of bytes waiting to be read in the
ITL0 "ping" stack.

####  ReadbackITL1length register \>2E (16-bit)

This register contains the number of bytes waiting to be read in the
ITL1 "pong" stack.

####  TransferCounter register \>22 (16-bit)

In this register you place the number of bytes that you intend to read
or write to either stack. For instance, when reading the ITL0 stack, you
would place here the number found in register \>2D.

Once you have actually read/written that number of bytes, bit \>0004 in
the MicroprocessorInterrupt register (\>24) will be set and the
BufferStatus register will be updated..

In DMA mode, the TransferCounter register serves to indicate how many
bytes are to be transfered by direct memory access. Again, bit \>0004 in
register \>24 will be set when done (which may generate an interrupt, if
so desired) and the BufferStatus register will be updated.

####  ATLport register \>41

This is the port that you use to write data to the ATL stack, or to read
data back from it. The host controller manages an internal pointer that
always points at the proper location on the stack, and is incremented by
two each time you transfer a word.

####  ITLport register \>40

This is the port to use to write to, or read from one of the ITL
ping-pong stacks. In addition to the internal pointer, the host
controller also determines which of the two stacks (ITL0 or ITL1) will
be accessed.

####  Philips Transfer Descriptor (PTD)

A PTDs is a header that you place before your data on the host
controller stack (ATL or ITL), so that the controller knows what to do
with the data. A PTD consists of 8 bytes. Because communication with the
ISP1161 occurs word-wise and the data bus was inverted, you will need to
swap the bytes from this structure before you place it on the stack.
Payload bytes are not swapped, though.


Completion code
Active
Toggle
Actual bytes
Endpoint #
Last
Speed
Max bytes
Reserved
B5_5
Res
PID
Total bytes
Reserved
Fmt
Device #
First payload
byte
Second byte,
etc

**Total bytes**: indicates the number of data bytes in the payload area
following the PTD.For IN packets, this is the size of the buffer, but
you must load dummy data into the stack, together with the PTD.

**Max bytes**: tells the controller how many bytes can be transfered at
one time. If it's less than the total, the controller will automatically
generate extra transfer operations, each time toggling the "toggle" bit.

**Actual bytes**: is only valid after the operation is completed, when
the PTD is read back. It's the number of payload data bytes that were
actually transfered.

**Device**: is the number of the USB device. All devices will answer to
\#0.

**Endpoint**: is the number of the endpoint addressed on the remote
device. All devices are guaranteed to have endpoint \#0. For others
numbers, you should get the device to enumerate them.

**PID**: Packet ID, this could be \>00=SETUP, \>04=OUT, or \>08=IN

**Speed**: speed of the endpoint: \>00=full, \>04=low.

**Fmt**: type of endpoint: \>80=isochronous, \>00=other (i.e. bulk,
control, or interrupt).

**Active**: always set this bit (i.e. \>08) when loading a PTD. The
controller will reset it once the PTD has been processed. This way it
won't process the same data twice, even if you leave it on the stack for
a while.

**Toggle**: value of the data toggle PID. \>00=DATA0, \>04=DATA1. The
controller will toggle it after each transaction.You must check this bit
when reading back the PTD, and toggle it in the next PTD.

**Last**: a value of \>08 indicates this is the last PTD in the ATL (or
ITL).

**B5_5**: This bit is specific to the ISP1161A1, as opposed to the
former ISP1161. When it's set (i.e. \>20), only 1 transaction will be
sent out per millisecond, during interrupt transfers for this PTD.

**Completion code**: This is only meaningful when reading back the PTD.
The controller will set it to indicate any error condition. The codes
are as follow:

\>00: No error
\>10: CRC error in last data packet from endpoint
\>20: Bit stuffing violation in last data packet from endpoint
\>30: DATA0/DATA1 toggle mismatch
\>40: Endpoint returned a "STALL" PID
\>50: Device not responding
\>60: PID check bits mismatch
\>70: Unexpected PID (illegal value, or legal but at the wrong time)
\>80: Data overrun, i.e. the endpoint returned more than "max bytes" in
a packet, or more than "total bytes".
\>90: Data underrun. i.e. the endpoint did not returned less than "total
bytes", yet less than "max bytes" (i.e. no more coming).
\>A0: reserved
\>B0: reserved
\>C0: Buffer overrun. During an IN, the host controller received data
faster than it could pass them to the system.
\>D0: Buffer underrun. During an OUT, the host controller could not get
data from the system fast enough to send them.

###  Interrupt management

The host controller can generate interrupts upon a number of various
events. These events are signalled in two dedicated status registers,
cascaded into each other (i.e. any interrupt in the first one will raise
a bit into the second). Each register has its associated enabling
register in which you decide which event should generate an interrupt.
In addition, there is one bit (weight \>0001) in the
HardwareConfiguration register that decides whether the interrupt will
actually be signalled on the INT1 pin. Note that, to reset pin INT1,
this bit must be set to '1' and all interrupts disabled. Then, the bit
can be set to '0', so as to freeze pin INT1 in a "no interrupt" state.
On top of that, CRU bit 2 is used to mask out any interrupt coming from
either controller when it is '0'.

####  InterruptStatus register \>03 (32-bit)

This register signals when certain transmission conditions have occured.
A '1' bit does not necessarily generate an interrupt, but can do so if
the corresponding bit is set in the InterruptEnable register. To clear
the interrupt, write '1' to the corresponding bit, writing '0' has no
effect.

    Bit value  Meaning
    0000 0040  1: There was a change in the HubStatus register.
    0000 0020  1: Frame number overflow (leftmost bit has toggled).
    0000 0010  Always 0 (Unrecoverable error: never happens).
    0000 0008  1: Device resuming signal detected.
    0000 0004  1: Start-of-frame packet sent.
    0000 0001  1: Scheduling overrun (current frame time exhausted).

####  InterruptEnable register \>04 (32-bit)

This register enables the generation of interrupts by event signaled in
the InterruptStatus register. To enable an interrupt, write a '1' to the
corresponding bit, writing '0' has no effect. In addition, the register
contains a "master bit" that masks all the others: this bit must be set
to '1' for any interrupt to be generated. Upon a read, the current value
of the interrupt mask is returned (a '1' bit means the interrupt is
enabled).

    Bit value  Meaning
    8000 0000  1: Enable interrupts if below bits are set to '1'.
    0000 0040  1: Enable HubStatus change interrupt.
    0000 0020  1: Enable frame number overflow interrupt.
    0000 0010  1: Enable unrecoverable error interrupt.
    0000 0008  1: Enable device resuming signal interrupt.
    0000 0004  1: Enable start-of-frame interrupt.
    0000 0001  1: Enable scheduling overrun interrupt.

####  InterruptDisable register \>05 (32-bit)

This register plays the same role as the above, except that it is used
to disable interrupts. Writing a '1' disable an interrupt, writing a '0'
has no effect. This scheme allows you to toggle the value of individual
bits without having to worry about maintaining all other bits unchanged.
Upon a read, the current value of the interrupt mask is returned (a '1'
bit means the interrupt is enabled just like for the InterruptEnable
register).

    Bit value  Meaning
    8000 0000  1: Disable all interrupts below.
    0000 0040  1: Disable HubStatus change interrupt.
    0000 0020  1: Disable frame number overflow interrupt.
    0000 0010  1: Disable unrecoverable error interrupt.
    0000 0008  1: Disable device resuming signal interrupt.
    0000 0004  1: Disable start-of-frame interrupt.
    0000 0001  1: Disable scheduling overrun interrupt.

####  MicroprocessorInterrupt register \>24 (16-bit)

This is the main interrupt status register, into which register \>03 is
cascaded in the form of one bit. There are five other ovents that can
cause interrutps by setting a bit in this register. The interrupt will
only occur if the corresponding bit in register \>25 is also set to '1'.
In addition, bit 0001 in the HardwareConfiguration register must be set,
and CRU bit 2 should be '1'.

    Bit   Meaning
    0040  1: Clock is ready (160-1000 usec after a wakeup).
    0010  1: Host controller suspended.
    0010  1: Interrupt(s) occured in register >03.
    0004  1: Data transfer completed (normal or DMA).
    0002  1: ATL stack should be read.
    0001  1: Start of frame occured. ITL should be read.

Before you read the ATL or the ITL, first check the BufferStatus
register (see below).

####  MicroprocesssorInterruptEnable register \>25 (16-bit)

The bits in this register correspond to those in the
MicroprocessorInterrupt register. They are all initialized as '0' upon
power-up and reset, which means that no interrupt will be generated, no
matter what happens inside the MicroprocessorInterrupt register. You can
enable a given interrupt by setting the corresponding bit to '1'.

    Bit   Meaning
    0040  1: Enable clock-ready interrupts.
    0010  1: Enable host-suspended interrupts.
    0010  1: Enable interrupt from InterruptStatus register (>03).
    0004  1: Enable transfer-completed (to/from CPU) interrupts.
    0002  1: Enable ATL interrupts.
    0001  1: Enable start-of-frame interrupts.

###  Frame management

One of the tasks of an USB host is to send a frame packet, containing a
SOF token and a frame number, to all devices every millisecond. This is
especially usefull for real-time transmissions, for synchronization
purposes. The interval between two frames must be very precise, so the
host must make sure that no other packet is being transmitted when it's
time to send a frame packet.

You don't have to worry about frames, since they are taken care of by
the host controller automatically. However, it is possible for you to
snoop on the process, or even to influence it, if you so desire.
Furthermore, you will need to initialize the registers that set the
transmission thresholds, i.e. the point at which the controller won't
start a transfer by fear of being interrupted by the next frame packet.
All this is achieve via a set of four dedicated registers.

####  FrameNumber register \>0F (32-bit)

This read-only register contains the current frame number, from \>0000
to \>FFFF (the most significant word is always \>0000). When the counter
reaches \>8000, or rolls over to \>0000, a bit is set in the
InterruptStatus register. This lets you implement a roll-over counter,
to count more than 65536 frames.

####  FrameRemaining register \>0E (32-bit)

This read-only register contains the remaining bit time, i.e. number of
bits that can still be transmitted until the end of the frame. Once it
reaches \>0000, a frame packet is sent on the USB bus, and the register
is reloaded with the value found in the FramInterval register (including
the leftmost bit). This also happens every time the controller enter the
operational state.

If you want to check this value, you should realize that this register
is updated very often: 12 million times per second! So, for all
practical purposes, the 4 rightmost bits could be safely ignored.

This number is only 14 bits long, so legal values are \>0000 through
\>3FFF. In addition, the leftmost bit (value \>8000 0000) is copied from
the FrameInterval register, which toggles at every such copy, so you can
tell that a roll-over has occured.

####  LowSpeedThreshold register \>11 (32-bit)

This register contains an 11-bit value, which the contoller compares
with the FrameRemaining register. If the remaining number of bit is
lower than the LowSpeedThreshold, no more packet can be sent. This is a
way to ensure that the frame packet will not have to interrupt an
ongoing transmission.

Low-speed data packets are limited to 8 bytes, so 64 bits. To which we
must add about 27 extra bits for synchro, packet ID, CRC and end-mark.
This adds up to 91 bits. Since low speed is 12 time slower than full
speed (1 Mbit/sec vs 12 Mbit/sec) we should multiply this value by 12,
yielding 1092. And then add a few more bits to have a good safety
margin...

The register is initialized with the value \>0628 at power-up time,
which coresponds to 1576. This gives us a safety margin of about 40
low-speed bits. You should probably leave it so...

####  FrameInterval register \>0D (32-bit)

This register contains two values. The lower word contains the maximum
number of bits that can fit into a frame. This is the value that will be
loaded into the FrameRemaining register when it reaches zero. When this
happens, the leftmost bit (weight \>8000 0000) will also toggle.

Given that full-speed means 12 megabits per second, and that frames
occur every millisecond, this value should be 12,000. Accordingly, the
register is initialised as 11,999 (i.e. \>2EDF) but you can modify this
value a little, to improve synchronization with a recalcitrant device.

The upper word (minus the lefmost bit) contains the maximum number of
data bits that can be sent or received in a single full-speed
transaction, without causing a scheduling error. This value provides the
transmission threshold for full speed, but unfortunately it is not
initialized at power-up time. Philips recomends setting it as \>2778.

    Bit value  Meaning
    8000 0000  Toggle when reloading FrameRemaining.
    xxxx 0000  Max number of data bits per transaction (0-7FFF).
    0000 xxxx  Max number of bits per frame (0-3FFF), loaded into FrameRemaining.

###   Miscellaneous registers

These are mostly useful for testing purposes.

####  Scratch register \>28 (16-bit)

This register is available for you to store any value you want, for any
purpose. For instance, I'm using it to store the return point of the
routine that toggles SRAM pages (otherwise, if the the workspace is in
the SRAM, the return point would be lost).

####  Revision register \>00 (32-bits)

This register contains the USB version number, in binary-coded decimal
(BCD) format.

`0000 0010` means USB 1.0
`0000 0011` means USB 1.1

####  ChipID register \>27 (16-bit)

This register contains the chip ID number. The upper byte is the product
number (\>61), the lower byte is the version number.

`6110` is the ISP1161
`6113` is the ISP1161A1

I suppose the ISP1161A must be `6111 `or `6112`...

###  Host controller register summary

Here is a summary of the registers available in the host (and hub)
controller. The first number is used to read from that register, the
second number (with \>80 added) is used when writing to this register.
This number is to be passed at address \>5FFA. Note that some registers
are read-only, and that there is one write-only register.

The third number is is register size in bits. If it's 32, you will need
to perform two successive reads at \>5FF0 or two writes to \>5FF8 to
transfer the whole register (least significant word first). Note that
registers below \>20 are all 32-bit, whereas \>20 and above are 16-bit .
The stack ports \>40 and \>41 have no limit in size, but that of the
stack.

    Number   Bits Name
    00 --    32   Revision
    01 81    32   Control
    02 82    32   CommandStatus
    03 83    32   InterruptStatus
    04 84    32   InterruptEnable
    05 85    32   InterruptDisable
    0D 8D    32   FrameInterval
    0E --    32   FrameRemaining
    0F --    32   FrameNumber
    11 91    32   LowSpeedThreshold
    12 92    32   HubDescriptorA
    13 93    32   HubDescriptorB
    14 94    32   HubStatus
    15 95    32   Port1Status
    16 96    32   Port2Status
    20 A0    16   HardwareConfiguration
    21 A1    16   DMAconfiguration
    22 A2    16   TransferCounter
    24 A4    16   MicroprocessorInterrupt
    25 A5    16   MicroprocessorInterruptEnable
    27 --    16   ChipID
    28 A8    16   Scratch
    -- A9    16   SoftwareReset
    2A AA    16   ITLbufferLength
    2B AB    16   ATLbufferLength
    2C --    16   BufferStatus
    2D --    16   ReadbackITL0Length
    2E --    16   ReadbackITL1Length
    40 C0    oo   ITLport
    41 C1    oo   ATLport

 

### The device controller

Accessing the device controller works about the same way than for the
host controller: you write a register number at \>5FFE, then you either
write data at \>5FFC, or read it at \>5FF4. The difference is that
sometimes no data is required: just writing the register number triggers
an action. For this reason, you may want to think in terms of
"functions" rather than "registers": you pass the function number at
\>5FFE, and its parameters (if any) at \>5FFC/5FF4.

Internally, the device controller consists in 16 endpoints. The first
two are numbered "zero", one in the input direction, one is the output
direction. The remaining ones are numbered from 1 through 14, and you
can program each one either as input or as output. Each endpoint has its
own FIFO stack, and you can decide how much memory to allocate to each,
if any. The total available memory is 2462 bytes (don't ask me why). You
should always configure all 16 endpoints when initializeing the device
controller, even those that you don't use, even the first two (number
zero) endpoints which have a fixed configuration.

Endpoints programmed as inputs receive the "OUT" and "SETUP" packets
from the host. The incoming packet is placed on the stack, the device
controller acknowledges it (unless it's isochroneous), the corresponding
status register is updated and an optional interrupt can be issued to
warn you that the stack should be read. Once you've read it, you must
empty the endpoint's stack, to make room for the next packet, otherwise
the device controller will send a "not acknowledge" handshake packet to
the host.

Endpoints programmed as inputs answer the "IN" packets from the host.
You should first place the data you want to send to the host on the
endpoint. Then you validate the endpoint, i.e. tell the device
controller that the data can be sent, as soon as an IN packet addressed
to this endpoint arrives. Once the host has acknowledge reception of the
data, the corresponding status register will be set, and an optional
interrupt can be issued to tell you that the endpoint is now empty
again. If an "IN" packet were to arrive from the host before the
endpoint is filled and validated, the device controller would send a
"not acknowledge" handshake packet to the host.

###  Initialization

These registers are to be set at power-up time, for the device
controller to work properly. In addition, the host can send a so-called
"bus reset" by holding both data lines low for 3 msec. This will reset
the contents of most registers in the device controller, and require
that you re-initialize them. You can set the controller so that it sends
an interrupt when such a reset occurs.

####  HardwareConfiguration register \>BA/BB (16-bit)

This register is used to set the configuration of the ISP1161 hardware.
Use the number \>BA to write to it, but use \>BB to read it back.

Its bit fields are the following:

    Bit   Meaning
    4000  1: External pull-up on DP. 0: Use soft-connect pull-up.
    2000  1: Disable CLOCKOUT pin when suspended (default). 0: switch it to lazy-clock.
    1000  1: Never stop the clock. 0: Stop clock 2 ms after "suspended" state.
    0x00  Clock frequency = 48 / (x+1). Reset value = 3 (i.e. 12 MHz).
    0080  1: DACK-only DMA mode. 0: Regular 8237-type DMA mode.
    0040  1: DREQ2 active high (default). 0: DREQ2 active low.
    0020  1: DACK2 active high. 0: DACK2 active low.
    0010  1: EOT active high. 0: EOT active low.
    0008  1: Wakeup when CS* pin goes low.
    0004  1: Power off when suspended. 0: reserved, do not use.
    0002  1: Pulse INT2 (edge-tiggered interrupts). 0: Level-triggered interrupts.
    0001  1: INT2 active high. 0: INT2 active low.

The default values upon a reset or a power-up is \>2340. If you did
implement the external pull-up resistor on the card, you should then
change it to \>630C, otherwise to \>230C.

####  DeviceMode register \>B8/B9 (8-bit)

Use \>B8 to write to this register and \>B9 to read from it. The
register contains several bit fields:

    Bit Meaning
    80  1: 16-bit DMA bus.  0: 8-bit DMA bus.
    20  1 then 0 (toggle): go suspended.
    08  1: Enable interrupts.
    04  1: NAK and errors genreate interrupts too. 0: ACK only generate interrupts.
    01  1: Internal pull-up connected. 0: not connected, implicit if external pull-up used.

####  EndpointConfiguration registers \>20-3F (8-bit)

There are 16 such registers, one for each endpoints. They are numbered
\>20 through \>2F for writing purposes, but \>30 through \>3F for
reading.

You must configure all 16 endpoints in order at power-up time. The
device controller will only be operational when all endpoints have been
configured. Note that this configuration will be erased to \>00 after a
hardware reset, or if a bus reset is sent by the host. In such a case,
you will need to reconfigure all the registers.

The bit fields are:

    Bit Meaning
    80  1: Endpoint active. 0: Inactive, no memory reserved.
    40  1: Output endpoint (IN packets). 0: Input (OUT/SETUP packets).
    20  1: Use ping-pong stacks (uses twice more memory).
    10  1: Isochroneous endpoint (does not acknowledge packets).
    0x  Stack size

For non-isochronous endpoints, there are four possible stack sizes:

    0 = 16 bytes, 1 = 32 bytes, 2 = 48 bytes, 3 = 64 bytes.

For isochronous endpoints, there are sixteen possibles stack sizes:

    0 = 16 bytes   1 = 32 bytes   2 = 48 bytes   3 = 64 bytes
    4 = 96 bytes   5 = 128 bytes  6 = 160 bytes  7 = 192 bytes
    8 = 256 bytes  9 = 320 bytes  A = 384 bytes  B = 512 bytes
    C = 640 bytes  D = 768 bytes  E = 896 bytes  F = 1023 bytes

Obviously, if bit \>80 is not set, the stack size is zero, no matter
what value is in the least-significant nibble.

The total available memory to be distributes among the endpoints you are
using is 2462 bytes. Be aware that selecting ping-poing stacks will
result in using twice the amount of memory that you seleted (because the
endpoint will have two stacks). This is mainly useful for isochronous
endpoints.

The first two endpoints ((number 0 IN and OUT) have fixed configuration
values, but you should still pass them to the controller. Each has a
64-byte stack, so their configuration values are \>C3 and \>83
respectively.

####  Reset command \>F6 (0-bit)

This is an example of a register to which you don't need to write any
data. Merely accessing it (i.e. passing its number to address \>5FFE)
will reset the device controller. All DC registers will be reinitialized
to their default values. This is also what happens when you reset the
TI-99/4A console.

####  UnlockDevice register \>B0 (16-bit)

When the device controller is suspended all registers are locked to
prevent accidental alteration of their contents. When the controller
wakes up and enters the "resume" state, these registers remain
write-protected until you send the unlocking command \>AA73 to the
write-only register number \>B0.

####   DeviceAddress register \>B6/B7 (8-bit)

This register hold the device number. At power-up time, this number is 0
as each device must answer requests addressed to device 0. Later on, the
host will assign a dedicated number to the device controller, which
should be placed into this register. Use \>B6 to write to the address
register, \>B7 if you mean to read from it.

    Bit Meaning
    80  1: Device is active. 0: Inactive.
    xx  Device number (1-7F).

A bus reset from the host implicitely resets the device number to 0, but
does not alter the contents of this register, so it's your job to
maintain it properly.

###  Data flow management

#### EndpointBuffer registers \>01-1F

Through these registers, you can access the FIFO stacks for the 16
endpoints (that is, assuming that you did assign some memory to these
endpoints). To write to an output endpoint (so it can asnwer IN
packets), use registers number  \>00 through \>0F. To read from an input
endpoint (after it received an OUT or SETUP packet), use register
numbers \>10 through 1F.

Note that is is illegal to write to an endpoint programmed as input, or
to read from an endpoint programmed as output. This could produce
unpredictable results. For this reason, registers number \>00 and \>11
are not to be used. For the other endpoints, it all depends on the
direction that you assigned to a given endpoint.

Data in an endpoint should be preceded with a very simple header: a
single word containing the number of  data bytes that follow. Data bytes
are stacked in order of transmission. However, the ISP1161 has a 16-bit
data bus and uses the Intel convention placing the least significant
byte in the leftmost byte of a word. Which, of course, is the opposite
of the TI convention we are used to. So that we don't have to SWPB every
data word, I have decided to swap the data bus bytes at hardware level.
The drawback is that you will have to SWPB data for every other
register, but I reasonned that stack access would be the speed limiting
factor.

####  ValidateEndpoint commands \>61-6F (0-bit)

After you placed data into an output endpoint, you must validate it,
i.e. tell the device controller that is can send this data to the host
in answer to an IN packet. To this end, you just write the register
number \>60 through \>6F (no need to pass any data).

It is illegal to validate in input endpoint, so register \>60 (endpoint
0 OUT/SETUP) should never be used. Neither should any register
corresponding to an enpoint that you programmed as input.

####  ClearEndpoint commands \>70-7F (0-bit)

After you have read data that has arrived into an input endpoint with an
OUT or SETUP packet, you must empty its stack to make room for the next
packet. This is done by passing register number \>70 through \>7F to the
address port, with no data to the data port.

It is illegal to clear an endpoint programmed as output, so register
\>71 (enpoint 0 IN) should never be used.

####  EnpointStatus registers \>50-5F (8-bit)

These read-only registers let you read the status of the corresponding
endpoint: 0 OUT, 0 IN, then 1 through 14. The bit fields are:

    Bit  Meaning
    80   1: Endpoint is stalled.
    40   1: Secondary ("pong") stack contains data.
    20   1: Primary ("ping") stack contains data.
    10   1: Data packet ID is DATA1. 0: packet ID is DATA0.
    08   1: Setup packet was ovewritten by a new packet before you read it.
    04   1: Stack contains a setup packet (this unstalls the endpoint).
    02   1: Use secondary ("pong") stack. 0: Use primary ("ping" stack).

Accessing an endpoint status register in this way will also clear the
corresponding bit in the interrupt register. If you wish to check the
status of an endpoint without altering the interrupt register, you
should use the endpoint StatusImage register.

####  EndpointStatusImage registers \>D0-DF (8-bit)

These read-only registers let you access a copy of the EndpointStatus
registers, without clearing any bit in the interrupt register. The bit
fileds are the same as for the EndpointStatus registers.

####  EndpointError registers \>A0-AF (8-bit)

The device controller places an error code for each endpoint in these
read-only registers. The next transaction to a given endpoint will
overwrite its error code with a new value, whether you read it or not.
However, bit \>80 will be set to inform you of that fact.

    Bit  Meaning
    80   1: Previous error code was overwritten.
    40   1: Last successful data packet ID was DATA1. 0: It was DATA0.
    1E   Error code
    01   1: Successful transmission. 0: Error occured.

The error codes are:

    00: No error.
    02: Packet ID encoding error.
    04: Unknown packed ID.
    08: Unexpected packet type (token vs data vs handshake).
    08: CRC error in token packet.
    0A: CRC error in data packet.
    0C: Timeout error.
    0E: "Babble" error (whatever that is).
    10: Unexpected end of packet.
    12: Not-acknowledged (NAK) packet sent or received.
    14: Sent "stall", because a packet arrived to a stalled enpoint.
    16: Overflow, incoming packet larger than endpoint stack.
    18: Sent an empty isochronous packet.
    1A: Bit stuffing error (e.g. missing the extra '0' after six '1' bits).
    1C: Sync error.
    1E: Wrong data ID toggle (DATA0 vs DATA1). Packet ignored.

####  StallEndpoint commands \>40-4F (0-bit)

By writing these registers number to address \>5FFE (with no data), you
will stall the corresponding endpoint. From there on, it will answer any
request from the host with a "STALL" handshake packet. This will most
probably cause the host to send a SETUP packet, which will unstall the
endpoint. Otherwise, you can then unstall it with the UnstallEndpoint
command.

####  UnstallEndpoint commands \>80-8F (0-bit)

By writing these registers number at \>5FFE, you will unstall the
corresponding endpoint. This will also reset the endpoint, i.e. its
stack will be emptied and its toggle bit will be reset to DATA0. The
same thing occurs when a SETUP packet is received by a stalled control
endpoint.

####  AckowledgeSetup command \>F4 (0-bit)

When a setup packet arrives to a control endpoint (i.e. endpoint 0), it
disables the "ClearEndpoint" and "ValidateEndpoint" commands. To
re-enable these commands, you must write register number \>F4 to the
address port, at \>5FFE.

###  Interrupts management

For interrupts to be generated, the appropriate bits must be set in the
InterruptEnable register. In addition, bit \>08 must be set in the
DeviceMode register to enable the INT2 pin of the ISP1161. Finally, CRU
bit 2 should be set to '1' fonr interrupts to be passed to the TI-99/4A.

You can poll the Interrupt register to see if an interrupt is pending,
whether or not it reached the TI-99/4A.

####  InterrupEnable register \>C2/C3 (32-bit)

The corresponding bit in this register must be set to '1' for a given
event to generate an interrupt. Setting it afterwards, even if the event
is still active, will not cause an interrupt. A bus reset does not
affect this register.

    Bit       Meaning
    0080 0000 1: Enable interrupts from endpoint #14.
    0040 0000 1: Enable interrupts from endpoint #13.
    ...
    0000 0400 1: Enable interrupts from endpoint #1.
    0000 0200 1: Enable interrupts from endpoint #0 IN.
    0000 0100 1: Enable interrupts from endpoint #0 OUT.
    0000 0040 1: Enable interrupts when short packets detected.
    0000 0020 1: Enable interrupts when no start-of-frame detected for 1 millisecond.
    0000 0010 1: Enable interrupts when start-of-frame detected.
    0000 0008 1: Enable interrupts when end-of-transmission occurs.
    0000 0004 1: Enable interrupts when entering "suspended" state.
    0000 0002 1: Enable interrupts when entering "resume" state.
    0000 0001 1: Enable interrupts when bus reset occurs.

####  Interrupt register \>C0 (32-bit)

This read-only register identifies the cause of an interrupt and the
endpoint that caused it. The endpoint indicator bit will be cleared when
you read the corresponding EndpointStatus register.

    Bit       Meaning
    0080 0000 1: Interrupt came from endpoint #14.
    0040 0000 1: Interrupt came from endpoint #13.
    ...
    0000 0400 1: Interrupt came from endpoint #1.
    0000 0200 1: Interrupt came from endpoint #0 IN.
    0000 0100 1: Interrupt came from endpoint #0 OUT.
    0000 0080 1: Bus is suspended. 0: Bus is awake.
    0000 0040 1: Short packet detected.
    0000 0020 1: No start-of-frame detected for 1 msec. After 3 msec, DC goes "suspended".
    0000 0010 1: Start-of-frame detected.
    0000 0008 1: End-of-transmission occured (DMA tranfer).
    0000 0004 1: Bus went from "awake" to "suspended" state.
    0000 0002 1: "Resume" state detected.
    0000 0001 1: Bus reset detected.

###  DMA transfer

Direct Memory Access allow you to transfer data to/from memory much
faster than you would with an assembly program. But it requires a
dedicated piece of hardware: a DMA controller, which the TI-99/4A system
does not have. However, I'm planning to build a DMA controller board, so
I included the necessary circuitery on the USB-SM board.

The device controller allows two types of DMA: the classical Intel 8237A
type (which is the one we want) and the Motorola DACK-only type. The
polarity of all three signals, DREQ, DACK and EOT, is programmable (see
HardwareConfiguration register) and so is the number of bytes per
"burst". Note that DMA is not possible with endpoint 0, in either
direction.

To perform DMA, you would place the number of bytes in the DMAcounter
register and the endpoint number in the DMAconfiguration register. You
would also program the DMA controller board so that it knows where to
put the data. Finally, you would trigger DMA by setting bit \>0008 in
the DMAconfiguration register.

####  DMAconfiguration register \>F0/F1 (16-bit)

Use \>F0 to write to this register, \>F1 to read from it. A bus reset
will not affect this register. except for the \>0008 bit, which is
reset, thereby disabling DMA operations.

    Bit  Meaning
    8000 1: Generates EOT when counter reaches 0.
    4000 1: A short or empty OUT packet causes an EOT. (Must be 0 when sending IN packets).
    0100 1: Last DMA access was a word. 0: Last DMA access was a byte.
    00x0 Enpoint number (+1), to do DMA with.
    0008 1: Enable DMA. 0: Disable DMA, abort transfer if needed.
    0003 Burst length: 0 = 1 byte, 1 = 4 bytes, 2 = 8 bytes, 3 = 16 bytes.

It's not allowed to perform DMA with endpoint 0, in either direction.
For the others, you should specify the number of the endpoint plus one
in this reigster, i.e. \>0020 for endpoint \#1, \>00F0 for enpoint \#14
(don't ask me why). The direction of the transfer is determined by the
direction of the enpoint, as programmed in its dedicated configuration
register.

####  DMAcounter register \>F2/F3 (16-bit)

Use \>F2 to write the number of bytes to be transfered by DMA into this
register. This value will be read by the device controller when DMA is
enabled (by bit \>0008 of the DMAconfiguration register). Use \>F3 to
see how many bytes remain to be transfered.

###  Miscellaneous registers

#### Scratch register \>B2/B3 (16-bit)

This register is available for you to store any data you  like, as long
as it's in the range \>0000-1FFF (the first 3 bits are reserved).  For
instance, you could use it to save the device status before it goes
"suspended". Use register number \>B2 to write to the register, \>B3 to
read back from it.

####  FrameNumber register \>B4 (16-bit)

This read-only register contains the number of the last frame received
with a SOF packet.

####  ChipID register \>B5 (16-bit)

This read-only register contains the chip ID for the device controller
portion of the ISP1161. The most significant byte should read \>61 to
signify ISP1161. The least significant byte contains the version number,
e.g. \>23 for version 2.3.

###  Device controller register summary

Here is a summary of the device controller registers. Note that most
come be groups of 16, one for each endpoint. In such cases, register
number \>x0 accesses endpoint 0 OUT (i.e. inputing packets), \>x1
accesses endpoint 0 IN (i.e. outputing data packets), \>x2 accesses
endpoint number 1, as so forth until \>xF which accesses register number
14.

The number in the first column is to be passed to the address port
\>5FFE when you intend to write to a register (or to trigger a command),
the number in the second column is to be used when you intend to read
from a register. The number of bits indicates the size of the data to be
written to \>5FFC, or read from \>5FF4. A dash - indicates that no data
transfer is needed: merely passing the register number to \>5FFE
triggers the command. For 16-bit registers, remember that data bytes are
inverted; for 8-bit registers, the relevant data will be passed to/from
the most significant byte. For the EndpointBuffer registers, the number
of bytes to transfer to/from the data port depends on the size of the
FIFO stack assigned to this breakpoint, and bytes are not inverted.

    Write  Read   Bits  Name
    00-0F  10-1F   #    EndpointBuffer
    20-2F  30-3F   8    EndpointConfiguration
    40-4F          -    StallEndpoint
           50-5F   8    EndpointStatus
    60-6F          -    ValidateEndpoint
    70-7F          -    ClearEndpoint
    80-8F          -    UnstallEndpoint
    A0-AF          8    EndpointError
    B0            16    UnlockDevice
    B2     B3     16    Scratch
           B4     16    FrameNumber
           B5     16    ChipID
    B6     B7      8    DeviceAddress
    B8     B9      8    DeviceMode
    BA     BB     16    HardwareConfiguration
           C0     32    Interrupt
    C2     C3     32    InterruptEnable
           D0-DF   8    EndpointStatusImage
    F0     F1     16    DMAconfiguration
    F2     F3     16    DMAcounter
    F4             -    AcknowledgeSetup
    F6             -    ResetDevice

Revision 1. 6/11/03 Preliminary
Revision 2. 6/29/03 Ok to release

Revision 3.8/4/03 CRU selection by A10+A11. Bit 2 to Vpen. Added
74ALS175 + 74ALS04. Inverted bit3. Interrupt masking circuit redesigned.

Revision 4. 8/18/03 Added Geneve compatibility.

Revision 5. 9/17/03 Added DMA capability.

Revision 6. 10/1/03 Added write-protection switch for EEPROM. Swapped
bits 2 and 4.
Revision 7. 11/3/03. Split in two pages. Added chapters on USB and
ISP1161.
Revision 8. 5/13/04. Added description of PTD structure. Added note on
byte order.

[Next page](usb_dsr.md) (writing DSRs)
[Previous page](usb.md) (hardware)

[Back to the TI-99/4A Tech Pages](titechpages.md)
