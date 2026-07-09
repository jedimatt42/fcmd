# IDEDIAG: a diagnostic program for the IDE card

IDEDIAG is a diagnostic program that lets you check your IDE card for
potential hardware problems.

[Getting started
](#Start)[Card tests
](#card%20tests)[Drive tests
](#Drive%20tests)[SRAM tests
](#SRAM%20tests)[Configuring IDEDIAG](#Configuration)

##  Getting started

IDEDIAG is an assembly language program, in the form of a memory-image
"program" file that should be loaded with Editor-Assembler option 5, or
a similar loader.

When entering the program, you will first be asked for the CRU assigned
to the IDE card. Enter the second digit of the CRU address, i.e. the one
you have set with the rotary selector on the card. You can also press
Fctn-9 to abort the program.

The program then asks whether the front light-emmiting diode (LED) in
the card is now shining. Press "Y" if it is the case. If it isn't, it
may be because you picked the wrong CRU. If so, press "R" to re-enter
it. Or it may be that the LED circuitery is bad, in which case you
should press "N" to go on with further tests.

The main menu then offers you the choice between 4 options:

Press "1" to test the general card circuitery, including the clock
chip.
Press "2" to test your hard drive. It must be installed as the "master"
drive.
Press "3" to test the SRAM.
Press "0" to exit the program.

##  Card tests

These consist mainly in CRU bit tests, but the program also tests the
three busses (data, address, and page address), the clock chip, and
checks whether a drive is answering.

You will first be asked whether the LED at the back of the card is now
shining. Answer by "Y" or "N", but don't be in too much of a hurry to
answer: there is a delay loop here, to allow the clock time to move to
the next second and generate an interrupt.

The program then reports a 2-word error code, and possible bus errors.
If the card works as expected, all codes should be zero.

####  Bus low/high

Bus lines are tested to see whether they are clamped low or clamped
high. A line clamped low would indicate a shortcut to the ground,
whereas a line clamped high can be caused by a shortcut to +5 volts, or
more likely be a broken trace or poor soldering (TTL chips read high by
default).

To find out which line is affected, use to following code

Data bus: 80=D0, 40=D1, 20=D2, 10=D3, 08=D4, 04=D5, 02=D6, 01=D7
Address bus: 1000=A3, 0800=A4,...0001=A15
Page bus: 20=P2, 10=P3, 08=P4, 04=P5, 02=P6, 01=P7

Note that address lines A0 to A2 are not tested: if anything is wrong
with these, the card will not answer at all. Similarly, page lines P0
and P1 are not used with a 512K SRAM, and thus not tested (but you can
change this by changing the number of pages in the configuration, see
[below](#Configuration)).

####  Card error code

The meaning of the individual bits in the first error word is the
following:

0001: CRU bit 0 does not work (card select)
0002: CRU bit 1 does not work (registers vs SRAM at \>4000-4100)
0004: CRU bit 2 does not work (page switching)
0008: CRU bit 3 does not work (fix page 0 at \>4000-4FFF)
0010: CRU bit 4 does not work (SRAM mapping at \>6000-7FFF)
0020: CRU bit 5 does not work (SRAM write-protection)
0040: DIP-switch \#1 is closed (should be open with a bq4847 clock)
0080: CRU bit 2 reads low
0100: CRU bit 2 reads high (clock interrupt not detected)
0200: CRU bit 4 reads low
0400: CRU bit 4 reads high
0800: CRU bit 5 reads low
1000: CRU bit 5 reads high
2000: SRAM read error (all data identical over \>4100-5FFF)
4000: SRAM write error
8000: (reserved)

Second word:

0001: Clock read error (all bytes identical)
0002: Clock write error
0004: Clock won't tick
0008: Clock won't issue interrupts
0010: Clock battery is low
0020: Drive not answering (reset command)
0800: Back LED stays off
All other bits are reserved

Note that many hardware problems will impact on more than one test, so
the final error codes will be the sum of several of the above codes. And
yes, you will need to do some hexadecimal math to break it down. Nothing
too complicated though: just remember that A=10, B=11, C=12, D=13, E=14
and F=15.

Note: the SRAM read error is generated if the whole memory area
\>4000-5FFF contains the same word. I reasonned that a non-initialized
SRAM would contain random data, whereas an initialized SRAM would
contain DSRs. If the SRAM is not answering, then you may read \>0000 or
\>FFFF everywhere. Except of course, that you may have filled the SRAM
with the same data word, for instance by running the SRAM pattern
filling test! So if you did that, and then get a SRAM read error, don't
worry about it.

My own card is working perfectly, thank you, so the only hardware
failures I could simulate were by having switches in the wrong position.
It resulted in the following composite codes:

6B51 081B Wrong CRU address (could also be caused by a defect in the CRU
decoding logic).
6110 081B Rear switch in "off" position.
0000 0020 Middle switch in "Geneve" position with a TI-99/4A.

If you experience (and pinpoint) other harware problems, let me know
what error code they generated, so I can add them to this list.

##  Drive tests

IDEDIAG issues several standard IDE commands, and a few "enhanced IDE
commands". These are: Software reset, Set parameters, Read sector, Write
sector, Read buffer, Write buffer, Read drive ID, and Drive self-test.
It also performs a hardware reset via CRU bits 6 and 7.

Among the above, the Drive self-test command may take quite a long time.
But of course, if something is wrong with the drive circuitery, you may
end up waiting forever... So if nothing seems to happen after 10 to 20
seconds, press Fctn-4 \ to quit waiting and go ahead with
displaying the results of the tests.

#### Drive error code

Errors are announced as above, by means of as composite code which
breaks down as follow:

0001: Controller busy (BSY bit) or drive not ready (RDY bit in status
register)
0002: Command timed out (BSY bit)
0004: Error bit set in status register (ERR bit)
0008: Data request timed out (DRQ bit)
0010: Error during "Set parameters" command
0020: Error during software reset
0040: Error during "Self test" command
0080: Error while reading sector
0100: Error while writing sector
0200: Error while reading buffer
0400: Error while writing to buffer
0800: (reserved)
1000: (reserved)
2000: (reserved)
4000: Hardware reset does not work (CRU bits 6 and 7)
8000: (reserved)

The contents of the drive's error register is displayed as one byte,
just after the error code. This value only makes sense if an "Error bit
set in status register" error was issued. The individual bits read as
follow:

01: Address mark not found
02: Track 0 not found
04: Command aborted
08: Medium change required (enhanced IDE only)
10: ID mark not found
20: Medium changed (enhanced IDE only)
40: Uncorrectable data error
80: Sector marked as bad

####  Drive self-test

The drive self-test command reports one byte which reads as follows:

01: Master drive ok
02: Formatting circuit error
03: Buffer error
04: Error-correction code (ECC) logic error
05: Microprocessor error
06: Interface circuit error
80: At least one error in slave drive

This is the only case where the normal value is 01, not 00.

####  Ringing

IDEDIAG also tests for ringing on the DIORD\* and DIOWR\* lines. Ringing
is a common problem which has plagued the IDE interface ever since the
new standards required the drive to answer so fast. It only affect
signal lines that are edge-trigered, in our case DIORD\* and DIOWR\*. It
is due to resonnance within the long IDE cable, which cause a signal to
bounce up and down when brought low. If this signal is edge-triggered,
the drive may mistake the falling edge of a bounce for a second command
pulse.

With DIORD\*, the drive would then send a second word, when the computer
is only reading one. This results in "missing" words in the data read
from the drive. With DIOWR\*, the drive writes the same word twice into
a sector.

IDEDIAG writes and reads back sectors 7 though 1 sixteen times, looking
for duplications and missing words (the initial contents of these
sectors is saved first, so the test is non-destructive, assuming
everything goes well). It then performs the same test with the drive's
internal buffer. Errors that do not correspond to single-word ringing
are also counted and announced as write/readback or "W/R errors".

####  Drive information

Under the dashed line, the program displays part of the information
returned by the Drive ID command: the drive model, its serial number,
whether it supports logical block addressing (LBA) and direct memory
addressing (DMA). Your drive must support LBA to be compatible with my
IDEAL DSR, although the IDE card itself imposes no such requirements. If
LBA is supported, the maximum address available in LBA mode will be
displayed.

You will also get information on the drive's geometry: number of heads,
number of cylinders (i.e. tracks), number of sectors per cylinder, and
number of bytes per sector. For each of these, you get two values: the
first one is the actual physical value, the second is the logical value
used to perform address translation.

Finally, the drive configuration flags are reported, according to the
following code:

0001: (reserved)
0002: Hard-sectored drive
0004: Soft-sectored drive
0008: RLL/ARLL format
0010: Head switch delay = 15 microseconds
0020: Power-down mode implemented
0040: Hard disk
0080: Removable disk (e.g. CD-ROM)
0100: Internal data transfer rate \< 5 Mbits/sec
0200: Transfer rate between 5 and 10 Mbits/sec
0400: Transfer rate \> 10 Mbits/sec
0800: Rotation deviation \> 0.5% (notebooks)
x000: (reserved)

##  SRAM tests

When testing the SRAM, you have the option to preserve the DSRs, or to
test the whole chip. If you elect to preserve the DSRs, all tests will
begin with page 16 instead of page 0.

Press "1" to test the whole SRAM, which will erase the DSRs.
Press "2" to preserve the DSR area, i.e. not to test it.
Press Fctn-9 \ to return to the main menu.

####  Bus shorts

The program first tests the three busses: data, address, and page
address. This test is more comprehensive than the one performed with
general cards tests: it is meant to detect shortcuts between the various
bus lines. Errors are reported as described above, the difference being
that you expect to see at least two lines affected by a shortcut.

Data bus: 80=D0, 40=D1,... 01=D7
Address bus: 100 =A3, 0800=A4,... 0001=A15
Page bus: 20=P2, 10=P3,... 01=P7

####  Pattern filling

Then the program fills the SRAM with a test pattern and reads it back,
looking for mismatches. By default, there are four test patterns: 0000,
FFFF, AAAA and 5555, but you can add more by configuring the program
(see [below](#Configuration)). For each pattern, the total number of
mismatches is announced.

####  Data retention

Once the pattern filling test has been completed, you can test memory
rentention, i.e. whether the battery-backed SRAM maintains its integrity
while power is off. To do so, exit the program and turn power off for a
while (from 1 minute to several days). You may also want to try removing
the IDE card from the PE-box and give it a few gentle taps, to reveal
poor connections.

Then reload IDEDIAG, and select option 3 in the SRAM menu: "Check data
retention". The program first checks whether the DSRs are still in
memory (i.e. whether byte \>4000 in page 0 contains \>AA). If so, it
skips the first 16 pages of the SRAM. Then it looks for the presence of
the last test pattern into all remaining pages and announces the number
of mismatches.

##  Configuring the program

You can alter the behaviour of IDEDIAG by changing a several data words
in the file.You will need a sector editor (such as DISKU) to edit the
program. Parameters that you may change are located at the beginning of
the program, between a "\>\>" and a "\<\<" mark.

    >3C3C                   >> mark

    >0040                   maximum number of pages

    >0010                   first page to use when preserving DSRs

    >2000                   timeout delay

    >0010                   number of read/write cycles when checking for ringing

    >2000,>E800             parameters used by the "Set parameters" command

    >0000,>FFFF,>AAAA,>5555 patterns for the SRAM tests

    0,0,0,0,0,0,0           room for more patterns

    >5555                   pattern to test for data retention

    >3C3C                   << mark

The list of patterns is processed until one matches the pattern used for
data retention. If no such match is found, one more test will be
performed, using the pattern for data retention. Please to not use
\>AAxx as a data retention pattern, as this is the DSR mark and may
cause your computer to lock-up upon power-up.

Revision 1. 10/17/00. Ok to release.

[Back to IDEAL page](ideal.md)

[Back to the TI-99/4A Tech Pages](titechpages.md)
