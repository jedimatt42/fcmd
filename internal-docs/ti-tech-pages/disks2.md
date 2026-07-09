## The TI disk controller card

[CRU and registers map](#CRUmap)
**Sample programs  **
[Low-level routines
](#Low-level%20routines)[Track-by-track access](#Track%20access)
[Sector access
](#Sector%20R/W)[Checking drive speed](#Check%20speed)
[Goofy formats](#formats)
**The card ROM  **
[File buffers in VDP memory
](#VDPmem)[Power-up routine
](#power-up)[Device Service Routines
](#DSR)[Subprograms](#Subprograms)

**[Floppy disk drives](#Drives)**

### CRU map and FDC registers

A 74LS259 and a 74LS251 provide as a 8-bit CRU interface. The CRU base
address of the card is \>1100. The meaning of the 8 bits is the
following:

|     |                               |                                 |
|-----|-------------------------------|---------------------------------|
| Bit | Meaning when read             | Meaning when written to         |
| 0   | Load head requested (HLD pin) | 1: Select ROM, turn light on    |
| 1   | Drive 1 selected              | 1: Strobe motor (for 4.23 sec)  |
| 2   | Drive 2 selected              | 0: Ignore IRQ + DRQ             |
| 3   | Drive 3 selected              | 1: Signal head loaded (HLT pin) |
| 4   | 0: Motor strobe on            | 1: Select drive 1               |
| 5   | Always 0                      | 1: Select drive 2               |
| 6   | Always 1                      | 1: Select drive 3               |
| 7   | Selected side                 | Select side                     |

As mentionned above, the five registers of the floppy disk controller
map in the memory area \>FFF0-FFFF. The addresses are different for read
operations than for write operations:

|         |     |                  |
|---------|-----|------------------|
| Address | I/O | Name             |
| \>5FF0  | I   | Status register  |
| \>5FF2  | I   | Track register   |
| \>5FF4  | I   | Sector register  |
| \>5FF6  | I   | Data register    |
| \>5FF8  | O   | Command register |
| \>5FFA  | O   | Track register   |
| \>5FFC  | O   | Sector register  |
| \>5FFE  | O   | Data register    |

###  Electronic circuitery

#### Selection logic

Most of the selection logic is provided by an 18-pin custom control
chip. Its inputs are mostly address lines (combined via external AND, OR
or inverting gates). The outputs control the different parts of the
card:

Inputs
Memen\*
CRU bit 0 (from 74LS259).
A0 to A7.
Tripple OR gate (74LS27): A4 to A6.
Hex AND gate (74LS30): A4 to A11.
Tripple AND gate (74LS11): AMA&AMB&AMC (always high).
PCBEN (always high).

Ouputs
Enable the first ROM chip.
Enable the second ROM chip.
Enable the FDC (CE\* pin), also controls WE\* and RE\* pins in
conjunction with the A12, A11 and WE\* bus lines.
Enable CRU input (74LS251).
Enable CRU output (ORed with CRUCLK\* to 74LS259).
Enable data bus transceiver (74LS245) and the RDBEN\* bus line.

####  CRU interface

As mentionned above, the interface is implemented the classical way,
with a 74LS251 for CRU input and a 74LS259 for CRU output. With the
exception of bit 7, none of the output is directly reflected to an
input. So let's examine them separately.

Ouputs (74LS259)
**Bit 0**: Turn on a LED via a 2N3904 transistor, goes to the custom
control IC.
**Bit 1**: Triggers the B input of a 74LS123 one-shot. The A input is
tied to ground. The cap is 47 uF, the resistor 200K, thus the pulse
lenght will be 4.23 sec. The one-shot is retriggable though, which means
that the pulse can be prolonged indefinitely by constantly toggling CRU
bit 1. The Q output is inverted via a 74LS16 and goes to the disk
connector, pin 16 (motor on). Before inversion, it also goes to the
READY pin of the FDC and is combined with CRU bits 4-6 (see below).
**Bit 2**: Inverted, NORed with the CS\* signal for the FDC and the
"ORed" ouputs of the above one-shot, the IRQ and the DRQ pin combined
via two D flip-flops synchronized by the clock line. When set to 1, it
allows the IRQ and DRQ pins to inactivate the READY line in the PE-box
bus, which puts the TMS9900 CPU on hold.
**Bit 3**: Connected directly to the HLT pin of the FDC, provides a
"head loaded" confirmation.
**Bit 4**: NANDed with bit 1, goes to pin 10 of the drive connector
(SEL1). Also reflected to CRU input bit 1, via a +12V LM2901 buffer and
1K pull-ups to +5V (beats me).
**Bit 5**: Same as bit 4, to pin 12 of connector (SEL2) and CRU input
bit 2.
**Bit 6**: Same as bit 4, to pin 14 of connector (SEL3) and CRU input
bit 3.
**Bit 7**: Connected directly to CRU input bit 7, goes to pin 32 of the
drive connector (SIDE) via a 74LS16 inverter.

Inputs (74LS251)
**Bit 0**: Connected directly to the HLD output pin of the FDC
("please load head" signal).
**Bit 1**: Reflects the SEL1 connection, via a LM2901 buffer.
**Bit 2**: Reflects the SEL2 connection, via a LM2901 buffer.
**Bit 3**: Reflects the SEL3 connection, via a LM2901 buffer.
**Bit 4**: Inverted output of the "motor strobe" one-shot.
**Bit 5**: Grounded.
**Bit 6**: tied to +5V.
**Bit 7**: reflects CRU output bit 7.

####  FDC connections

The card uses a FD1771 single-density-only FDC.

CPU interface

MR\* gets its input from a 74LS16 tripple AND gate whose inputs are:
+5V, RESET\* bus line and a power-up reset circuit (47 uF cap to ground,
47K to ground, 15K to +5V).
CS\* is triggered by the custom control chip.

DAL0-DAL7: connected to D7-D0 via a 74LS245 bus transceiver whose DIR
pin is controlled by DBIN (via a 74LS244) and enabled by the custom
control IC (together with RDBEN\*).
A0 and A1 are connected to bus lines A14 and A13 respectively.
WE\* and RE\* are connected to the first two ouputs of a 74LS139 2-to-4
decoder, whose selection inputs are the A12 and WE\* bus lines, and
which is enabled by the custom control chip (same signal as CS\*, ORed
with A15).

CLK receives a clock signal from a crystal oscillator. The crystal is a
8MHz NDK080, oscillating via two 74LS04 inverters. The signal is
inverted twice via 74LS00 NAND gates (1 pin tied to +5V via 470 ohm
resistors), then triggers the CLK input of a 74LS161 4-bit counter. The
Q2 output is connected to the Clk pin, thereby providing a 1 MHz clock
signal.

INTREQ and DRQ are ORed together and with the inverted motor strobe then
go the the D input of a 74LS74 D-flip-flop clocked by the CLKOUT\* bus
line (buffered by a 74LS244) and cleared by the CS\* signal from the
custom control chip, via a 74LS04 inverter. The Q output clocks a second
74LS74 whose D input is pulled-up (also cleared by the inverted CS\*
signal) and whose Q output is ORed with the (non-inverted) CS\* signal
from the custom IC and CRU bit 2 inverted by a 74LS04. The resulting
output is inverted again by a 74LS04 and enabled a 74LS125 buffer whose
input is grouded and whose output controls the READY bus line. Got that?

Drive interface

An external data deparator is used, so XTDS is grounded. FDCLK and
FDDATA receive their input from pin 30 of the drive connector (READ
DATA), via an awfully complicated circuit involving: three 74LS123
one-shots, four 74LS74 D-flip-flop, six 74LS00 NAND gates, four 74LS04
inverters and one 74LS244 buffer.

TR00\* receives its input from a 74LS244 buffer, whose input is pull-up
by a 1K resistor and connected to pin 26 of the drive connector
(TRACK0).
IP\* is similarly connected to pin 8 of the connector (INDEX PULSE).
WRPT\* is similarly connected to pin 28 of the connector
(W PROTECTED).
READY receives its input from the 74LS123 one-shot that provides the
motor strobe.

WG is NANDed with the MR\* signal and send to pin 24 of the connector (W
GATE).
WD outputs to pin 22 of the drive connector (W DATA) via a 74LS16
inverter.
STEP is similarly connected to pin 18 of the drive connector (STEP) via
a 74LS16 inverter.
DIRC is similarly connected to pin 20 of the drive connector (DIR) via a
74LS16 inverter.
3PM is connected to +5V to specify a step/dir type of interface.

PH3 and TG43 are not connected.
TEST, WF\* and DINT\* are not used and are connected to +5V.

###   Sample programs

#### Low-level access routines

Here are some routines that you could use to talk directly to the FDC.
Most of them are adapted from the routines in the disk controller card
ROM. The main difference when using a double-density CorComp controller
is that the card is built around a FD179x that does not invert its data
bus. Also, the head-step time must be determined in a fairly complicated
way.

* and that the card has been turned on.
       LI   R12,&gt;1100     Disk controller CRU
       SBO  0             Must be on to talk to the FDC
* Once done, don&#39;t forget to turn the card off:
       SBZ  0             Now you can return to caller...`
* This routine is used to insert delays between commands.
* It expects the delay value in a data word following the call.
* Delays are for a 3MHz console with a workspace in 16-bits memory.
DELAY  MOV  *R11+,R6       Get delay
DLP1   SRC  R6,8           Kill time
       SRC  R6,8
       DEC  R6
       JNE  DLP1
       B    *R11`
* This routine selects the drive specified in R0
* It assumes R0 contains &gt;0001 to &gt;0003
* and R12 contains the CRU base address of the controller card
SELDRV MOV  R11,R10        Save return point
       AI   R12,8          Address of CRU bit 4 (SEL1)
       LI   R1,&gt;0080
       SLA  R1,0           Selection bit (&gt;0100, &gt;0200 or &gt;0400)
       LDCR R0,3           Deselect all drives
       AI   R12,-6         Address of CRU bit 1 (optional)
       STCR R0,3           Read drive selection lines
       CZC  R1,R0          See if our drive is deselected
       JNE  ERROR          Something went wrong
       AI   R12,6          Back to CRU bit 4
       LDCR R1,3           Select drive
       AI   R12,-8         Back to CRU base address
       BL   @DELAY         Here we need a short delay
       DATA &gt;0BB8          until the drive is ready
       B    *R10`
* This routine selects the side specified in R0
SIDE   MOV  R0,R0          Which side?
       JNE  SIDE1
       SBZ  7              Select side 0
       B    *R11
SIDE1  SBO  7              Select side 1
       B    *R11`
* This routine sends a command to the FDC
* It expects the command in the data word following the call
* and the CRU base address of the card in R12
SENDCM MOV  *R11+,R0       Get command
       MOV  R11,R10        Save return address
       INV  R0             FDC has an inverted data bus
       MOVB @&gt;5FF0,R6      Read FDC status register
       SBZ  1              Strobe motor
       SBO  1
       JLT  RDY            See if the &quot;ready&quot; bit is set
       BL   @DELAY         Drive is not ready
       DATA &gt;7530          Wait a long time, then assume it is
RDY    MOVB R0,@&gt;5FF8      Send command to FDC command register
       SBO  3              Signal &quot;head loaded&quot; on HLT pin
       SRC  R0,8           Kill time
       SRC  R0,8
       B    *R10`
* This routine waits for command completion
* It returns with the status byte in R0 msb
WAITCM MOVB @&gt;5FF0,R0      Read FDC status register
       INV  R0             FDC has an inverted data bus
       JLT  ERROR          Drive in not ready any more!
       SRC  R0,9           Test the &quot;busy&quot; bit
       JOC  WAITCM         Drive is busy: keep waiting
       SRC  R0,7           Restore original status byte
       B    *R11            `
* This routine should be called after an error occured.
ERR    MOV  R11,R9         Save return address
       BL   @SENDCM        Send &quot;Force interrupt&quot; command
       DATA &gt;D000          with no interrupt allowed
INAL1  MOVB @&gt;5FF0,R0      Read FDC status register
       SRC  R0,9           Check &quot;Busy&quot; bit
       JNC  INAL1          Remember: the byte is inverted
       B    *R9            Not busy any more: command completed`
* This routine reinitializes the controller card.
INIALL AI   R12,8          Address of CRU bit 4 (SEL1 line)
       LI   R6,&gt;0000
       LDCR R6,4           Reset all 3 SEL lines and SIDE line
       AI   R12,-8         Back to CRU base address
       SBZ  1              Strobe motor
       SBO  1
       LI   R6,&gt;2F00       &quot;Force interrupt&quot; command (&gt;D0 inverted)
       MOVB R6,@&gt;5FF8      Send it, whether ready or not
       BL   @WAITCM        Wait for command completion
       B    *R9

####  Track-by-track access

* It returns with the status byte in R0 msb
TRACK0 MOV  R11,R9         Save return address
       BL   @SENDCM        Send &quot;Reset&quot; command
       DATA &gt;0A00          (head loaded, no verify, step time=20 ms)
       BL   @WAITCM        Wait for completion
       MOVB @&gt;5FF0,R0      Read FDC status register again
       INV  R0             FDC has an inverted data bus
       SRC  R0,11          Check &quot;Track 0&quot; bit
       JNC  ERROR          We did not reach it!
       SRC  R0,5           Restore original status byte
       B    *R9            The track register is automatically loaded with &gt;00
 * This routine moves to the next track
NXTRAK MOV  R11,R9         Save return address
       BL   @SENDCM        Send &quot;Step in&quot; command
       DATA &gt;5A00          (update track reg, head loaded, no verify, 20 msec steps)
       BL   @WAITCM        Wait for command completion
       B    *R9  `
* This routine moves to the previous track
PRTRAK MOV  R11,R9         Save return address
       BL   @SENDCM        Send &quot;Step out&quot; command
       DATA &gt;7A00          (update track reg, head loaded, no verify, 20 msec steps)
       BL   @WAITCM        Wait for command completion
       B    *R9            `
* This routine seeks a given track, whose number is in R1
* It assumes the FDC track register contains the current track # for this drive
* if this were not the case either update the register or call TRACK0 first.
TOTRAK MOV  R11,R9         Save return address
       INV  R1             FDC has an inverted data bus
       MOVB R1,@&gt;5FFE      Put desired track # in FDC data register
*      INV  R7             Optional (if we just changed drives)
*      MOVB R7,@&gt;5FFA      Place current track # (for this drive)in FDC track register
       BL   @SENDCM        Send &quot;Seek&quot; command
       DATA &gt;1A00          (head loaded, no verify, step time=20 ms)
       BL   @WAITCM        Wait for completion
       SRC  R0,13          Check &quot;Seek error&quot; flag } only meaningfull if &quot;verify&quot;
       JOC  ERROR          Track ID does not match } bit is set (command &gt;1E00)
       B    *R9     `
* This reads a track into a buffer in VDP memory.
* It assumes the VDP address has been set to write to it.
* The number of bytes to be read from that track should be in R3.
RDTRAK MOV  R11,R9         Save return address
       BL   @SENDCM        Send &quot;Read track&quot; command
       DATA &gt;E400          with a 15 msec delay
       SBO  2              Enable wait states
RDTL1  MOVB @&gt;5FF6,R0      Get a byte from FDC data register
       INV  R0             FDC has an inverted data bus
       MOVB R0,@&gt;8C00      Save byte to VDP buffer
       MOVB @&gt;5FF6,R0      Repeat that twice (I&#39;ve no idea why, but
       INV  R0             the TI routines do it this way...)
       MOVB R0,@&gt;8C00
       DECT R3             We read 2 bytes
       JNE  RDTL1          More to come
       SBZ  2              Disable wait states
       BL   @WAITCM        Wait for command completion
       SLA  R0,4           Test the &quot;Lost data&quot; status bit
       JOC  ERROR          Optionally, we could try again a few times
       B    *R9`
* This routine formats a track. It assumes the data for the track has been
* placed in VDP memory and the VDP address set to read it.
* The number of bytes for that track should be in R3.
WRTRAK MOV  R11,R9         Save return address
       BL   @SENDCM        Send &quot;Write track&quot; command
       DATA &gt;F400          with a 15 msec delay
       SBO  2              Enable wait states
WTL1   MOVB @&gt;8800,R0      Get a byte
       INV  R0             FDC has an inverted data bus
       MOVB R0,@&gt;5FFE      Send byte to FDC data register
       MOVB @&gt;8800,R0      Repeat that twice (I&#39;ve no idea why, but
       INV  R0             the TI routines do it this way...)
       MOVB R0,@&gt;5FFE
       DECT R3             We wrote 2 bytes
       JNE  WTL1           More to come
       SBZ  2              Disable wait states
       BL   @WAITCM        Wait for command completion
       SLA  R0,2           Test the &quot;Write protect&quot; status bit
       JOC  ERROR          The darn disk is write protected!
       SLA  R0,4           Test the &quot;Lost data&quot; status bit
       JOC  ERROR          Optionally, we could try again a few times
       B    *R9

You'll have noted that I wrote "format a track" and not "write a track".
This is because the FDC interprets most data bytes in the range \>F5-FE
as formating instructions. Therefore, the "write track" command cannot
be used to write a track that contains data, since these data are likely
to include such a byte which would screw up the track format.

I don't know how the track copy programs overcome that problem. A
possible, although clumsy, solution would be:

Read a track from the source drive into a buffer

Copy the buffer into another buffer. While doing so, identify the data
blocks in each sector and replace all bytes \>F5 to \>FE with \>00.
Also, change the CRC bytes to \>F7 (which tells the FDC to calculate a
new CRC for the data block).

Write that track to the source disk

Access each sector on that track and patch the bytes we have modified,
fetching the correct values from the first buffer.

####  Sector access

* It returns with R0msb=side, R1msb=track #, R2msb=sector #, R3=sector size.
RDID   MOV  R11,R9         Save return address
       BL   @SENDCM        Send &quot;Read ID&quot; command
       DATA &gt;C000          with no 15 msec delay
       SBO  2              Enable wait states
       MOVB @&gt;5FF6,R1      Get track # from FDC data register
       INV  R1             FDC has an inverted data bus
       MOVB @&gt;5FF6,R6      Get side #
       INV  R6
       MOVB @&gt;5FF6,R2      Get sector #
       INV  R2
       MOVB @&gt;5FF6,R3      Get sector size code
       INV  R3
       MOVB @&gt;5FF6,R11     Skip CRC, first byte
       SRL  R3,7           Translate size code to # of bytes (optional)
       ANDI R3,&gt;0006
       MOV  @SIZES(R3),R3
       MOVB @&gt;5FF6,R11     Skip CRC, second byte
       SBZ  2              Disable wait states
       BL   @WAITCM        Wait for command completion
       SLA  R0,4           Test the &quot;Rec not found&quot; status bit
       JOC  ERROR          No ID block was found
       SLA  R0,1           Test the &quot;CRC&quot; then the &quot;Lost data&quot; bits
       JOC  ERROR          CRC mismatch. Optionally, we could try next ID block
       JLT  ERROR          Lost data. Here also, we could try again
       MOV  R6,R0          Put side # in R0
       B    *R9 `
SIZES DATA 128,256,512,1024 Meaning of the (standard) size code`
* This routine reads a given sector on the current track into a buffer in VDP memory.
* It expects the VDP to be ready to write to this buffer.
* R3 should contain the # of bytes per sector (i.e. the size of the buffer).
* R2 should contain the sector # (on the track) and R1 the track #.
* The proper track (and side) should already have been seeked.
RDSECT MOV  R11,R9         Save return address
       INV  R1             Because of the inverted data bus
       MOVB R1,@&gt;5FFA      Put current track # in track register
       INV  R2             Inverted data bus
       MOVB R2,@&gt;5FFC      Put required sector # in sector register
       BL   @SENDCM        Send &quot;Read sector&quot; command
       DATA &gt;8800          (1 sect, standard sizes, no delay, don&#39;t check sides)
       SBO  2              Enable wait states
RDSL1  MOVB @&gt;5FF6,R0      Get a byte from FDC data register
       INV  R0             FDC has an inverted data bus
       MOVB R0,@&gt;8C00      Save byte to VDP buffer
       MOVB @&gt;5FF6,R0      Repeat that twice (I&#39;ve no idea why, but
       INV  R0             the TI routines do it this way...)
       MOVB R0,@&gt;8C00
       DECT R3             We read 2 bytes
       JNE  RDSL1          More to come
       SBZ  2              Disable wait states
       BL   @WAITCM        Wait for command completion
       SLA  R0,4           Test the &quot;Rec not found&quot; status bit
       JOC  ERROR          No data block was found
       SLA  R0,1           Test the &quot;CRC&quot; then the &quot;Lost data&quot; bits
       JOC  ERROR          CRC mismatch. Optionally, we could try again
       JLT  ERROR          Lost data. Here also, we could try again
       B    *R9  `
* This routine fills a given sector with data from a buffer in VDP memory.
* It expects the VDP to be ready to read data from this buffer.
* R3 should contain the # of bytes per sector (i.e. the size of the buffer).
* R2 should contain the sector # (on the track) and R1 the track #.
* The proper track (and side) should already have been seeked.
WRSECT MOV  R11,R9         Save return address
       INV  R1             Because of the inverted data bus
       MOVB R1,@&gt;5FFA      Put current track # in track register
       INV  R2             Inverted data bus
       MOVB R2,@&gt;5FFC      Put required sector # in sector register
       BL   @SENDCM        Send &quot;Write sector&quot; command
       DATA &gt;A800          (1 sect, standard sizes, no delay, no side check, normal mark)
       SBO  2              Enable wait states
WRSL1  MOVB @&gt;8800,R0      Get a byte from VDP buffer
       INV  R0             FDC has an inverted data bus
       MOVB R0,@&gt;5FFE      Send byte to FDC data register
       MOVB @&gt;8800,R0      Repeat that twice (I&#39;ve no idea why, but
       INV  R0             the TI routines do it this way...)
       MOVB R0,@&gt;5FFE
       DECT R3             We wrote 2 bytes
       JNE  WRSL1          More to come
       SBZ  2              Disable wait states
       BL   @WAITCM        Wait for command completion
       SLA  R0,2           Test the &quot;write protected&quot; status bit
       JOC  ERROR          The disk is write-protected
       SLA  R0,2           Test the &quot;Rec not found&quot; status bit
       JOC  ERROR          No data block was found
       SLA  R0,2           Test the &quot;Lost data&quot; status bit
       JLT  ERROR          Lost data. Alternatively, we could try again
       B    *R9
     

#### Checking the rotation speed

We saw above that one of the bits in the Status register returns the
state of the index hole. We could use it for several purposes.

First, to detect whether there is a disk in the drive. If not, the
"hole" will always be here. If there is a disk, the hole will be closed
most of the time, and only briefly flash open. If the drive door has
been left open, the disk will not rotate and (most probably) the hole
will be permanently close.

* It returns with R0 = 0 if there is no disk in the drive (test with JEQ),
* R0 = &gt;FFFF if there is a disk, but not spinnig (test with JLT)
* R0 = 1 if there is a disk and it&#39;s spinning (test with JGT)`
TSTDSK MOV  R11,R9               Save return address
       BL   @SELDRV              Specify drive number
       SBZ  1                    Strobe motor
       SBO  1
       BL   @DELAY               Let drive start spinning
       DATA &gt;FFFF
       MOV  R9,R11               Restore return address
       MOV  @&gt;5FF0,R0            Read FDC status register
       SRL  R0,10                Test &quot;Index hole&quot; bit in status
       JOC  CLOSED               Hole is closed (remember: bus is inverted) `
       LI   R1,&gt;1000             Hole is open, see if it stays so for a time
TSL1   MOVB @&gt;5FF0,R0            Get status again
       SRL  R0,10                Check if hole is still open
       JOC  CHANGE               Now it&#39;s closed
       DEC  R1
       JNE  TDL1                 Keep trying
       CLR  R0                   Still open: no disk in drive
       B    *R11`
CLOSED LI   R1,&gt;3000             Hole is closed, see if it stays so
TDL2   MOVB @&gt;5FF0,R0            Get status again
       SRL  R0,10                Check if hole still closed
       JNC  CHANGE               Now it&#39;s open
       DEC  R1
       JNE  TDL2                 Keep trying
       SETO R0                   Still closed: drive door not closed
       B    *R11`
CHANGE CLR  R0                   Hole toggles between open and close
       INC  R0                   A disk is spinning
       B    *R11
Another thing we could do is to time the occurences of the hole. This
will allow us to measure the rotation speed of the drive. Defective
drives could then be sent out for fine-tuning of their rotation speed
(unless you now how to adjust it yourself). TI drives spin at 300 rpm
nominal, which means we should detect the index hole five times per
second.

* It assumes there is a disk in the drive (else speed=0)
* and that the door is closed (else it never returns).
* It uses the timer in the TMS9901 to impart a 7.5 sec measuring time.
* It then returns with the speed in RPM in R1.`
SPEED  MOV  R11,R9               Save return address
       BL   @SELDRV              Specify drive number
SPL1   SBZ  1                    Strobe motor
       SBO  1
       MOVB @&gt;5FF0,R0            Get status
       SRC  R0,10                Check if hole is open
       JNE  SPL1                 No: keep waiting`
       LI   R0,4692              Delay for 100 msec (100msec / 64*333ns = 4692)
       LI   R1,MYISR             Where to go when timer fires
       BL   @TIMEON              Starts timer
       LI   R2,75                75*100msec = 7.5 secs
       LIMI 2                    Enable interrupts`
       CLR  R1                   Reset counter
SPL2   MOVB @&gt;5FF0,R0            Get status
       SRC  R0,10                Check if hole is still open
       JOC  SPL2                 Yes: keep waiting
SPL3   MOVB @&gt;5FF0,R0            Get status again
       SRC  R0,10                Check if hole is still closed
       JNC  SPL3                 Yes: keep waiting
       INC  R1                   One more rotation completed
       SBZ  1                    Keep motors running
       SBO  1
       JMP  SPL2                 Loop: timer will interrupt us`
MYISR  DEC  R2                   Timer fired: count 100 msec
       JEQ  TIMOUT               Have 7.5 seconds elapsed?
       LIMI 2                    Enable interrupts (disabled by main ISR)
       SLA  R0,1                 Not 7.5 secs yet, keep waiting
       JOC  SPL2                 Hole was open
       JMP  SLP3                 Hole was closed`
TIMOUT BL   @TIMOFF              Time out: stop timer and clean up
       SLA  R1,3                 Calculate speed in RPM (resolution: 60/7.5 = 8)
       B    *R11
* NB Since we are only testing for 7.5 sec, the expected resolution is:
* 60 / 7.5 = +/-8 rpm. To get a more accurate value, we must increase the sampling time.
* E.g. 30 sec (LI R2,300) gives a resolution of +/-2 rpm (SLA R1,1 to calculate speed),
* 60 sec (LI R2,600) gives a resolution of +/-1 rpm (comment out the SLA R1), etc

NB See the page on the TMS9901 for an explanation of [timer
interrupts](../tms9901.htm#Timer%20mode).

####  Goofy formats

Now that we know how to format a track, we can format our disk is all
kind of weird ways. All we have to do is to place the proper data in the
buffer, then call WRTRAK. The main purpose of doing so would be to
prevent standard disk controllers from copying a disk, but there are
also less mean reasons.

Sector size

We could format one or more sectors with a size different from the
standard 256 bytes. For instance, setting the size length code as \>03
in the ID block allows us to use sectors of 1024 bytes. Obviously, only
two of these will fit on a track (possibly together with a standard
256-byte sector). This will considerably speed up disk access since we
don't need to look for a new sector every 256 bytes. The drawback is
that a 1025 bytes long file will use 2048 bytes of disk space: what a
waste!

Sector number

Normally, sectors are numbered from 0 to 8 on each track. This is
because the sector register is one byte wide, but there are more than
256 sectors on a disk (hopefully).

However, nothing prevents us from scrambling the sector numbers, as long
as we know which is where. Thereafter any program trying to access a
sector with the "Read sector" or "Write sector" commands will fail,
since the sector field in the ID block will never match. But if we know
that a sector on track 6 is numbered \>A8, we can fetch it easily by
placing \>A8 in the sector register, while on track 6.

Of course, this is not a very good protection scheme: a simple "Read ID"
command will reveal the "secret" sector number to anybody willing to
check.

Track / side number

Similarly, we could scramble the track numbering. Note that this will
not perturbate the track seeking commands, unless the "V" verify bit was
set in the command. However, most disk manager will check that the
correct track was reached by reading an ID block. If we number our
tracks 1-2-3-144-5-6... all accesses to track 4 are likely to fail.

CRC

Instead of using the \>F7 value to tell the FDC to issue the proper CRC,
we could place a dummy value in these bytes upon formating. This will
cause a CRC error when the user tries to read such an ID (or data)
block. Our program will purposedly ignore such errors. However, the
missing \>F7 code could cause a different problem:

Missing marks

The "Read track" command ends with a "lost data" error if it does not
encouter data marks and ID marks (encoded in a special way) where they
should be. This is because the command uses these mark to synchronize
the flow of bits, and arrange them in bytes. By omiting such a mark, we
can make a track unreadable as a whole. However, it will still be
accessible for the "Read sector", "Read ID" and "Write sector" commands
(except for the sector with the missing mark, of course). NB: there is a
way to prevent address mark detection with the FD1771 controller: thus
this method is not fool-proofed.

Multiple index holes

You certainly know about "flippies": double-sided disks that can be used
in a single-side drive, by just inserting them upside down to access the
other side. These disks have a second write-protection notch, symetrical
to the original one. They also have a second index hole in the disk
envelope, but not in the disk itself. What I'm suggesting here is to do
just the opposite: pierce a second hole in the disk itself, but not in
the envelope. This will effectively abort the "Read track" command when
the second hole is encountered and prevent track-by-track copying.

On the other hand, the "Read ID", "Read sector" and "Write sector" will
operate normally since they only abort after 5-6 occurences of an index
hole.

Once again, this is not a very good protection method since the bad guy
can just tape the extra hole shut!

Pin hole in the media

This method was sometimes used in the PC world: make a small hole in the
disk (using a laser) then detect the position of the hole to ensure the
disk is an original.

We could do this with a pin: make a scratch or a small hole towards the
outside of the disk (but not completely to the rim, to avoid damaging
sector 0 and 1). Then use a disk manager to locate the position of the
damage on the disk and include this information in your program

The protected program will then look for the presence of the damage in
one or more sectors and react accordingly. Please don't make it a
JEQ/JNE test: a quick disassembly will allow any hacker to bypass your
protection. A more clever scheme would be to encrypt your main program
and to place the (preferably long) key in a damaged sector, at a fixed
distance from the damage.

To locate a damaged sector, your program can just read it, invert every
byte, and write it back. Then read it again: the damaged bytes won't be
inverted (just don't forget to restore the other bytes once you're
done).

##  The controller card ROM

The controller card comprises a 8192 bytes (\>2000) ROM that contains a
power-up subroutine, four DSR for high-level file access and many
subprograms for low- or mid- or high-level disk access. As usual with
peripheral cards, that ROM appears at \>4000-5FFF when CRU bit 0 is set
to one for that card. Note that the controller's register map at
\>5FF0-5FFF, thus the last 16 bytes of the ROM cannot be accessed.
You'll find a commented disassemly listing of the whole ROM on my
[download](download.htm#disass) [](../download.htm#disass) page
(warning: it's written in an ugly programming style).

###  File buffers in VDP memory

Upon initialisation, the disk controller power-up routine reserves room
for three files at the end of the VDP memory. Subsequently, the
subroutines FILES (to be called from Basic) or \>16 (to be called from
assembly) can modify the amount of memory reserved, from 1 to 16 files.
The bottom of the reserved area (i.e. the highest free address) is
pointed at by \>8370.

This memory area is used by the disk controller to store characteristics
of the current disk, and of the currently accessed files. It has the
following structure (exemple with 3 files):


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

###   The power-up routine

This routine performs the equivalent of a CALL FILES(3), i.e. it
reserves room for three file acess blocks at the end of the VDP memory.

###  The DSRs

There are four DSR subroutines in the original TI disk controller card:

**DSK1** accesses the first disk drive
**DSK2** accesses the second disk drive
**DSK3** accesses the third disk drive
**DSK** expects a disk name after a decimal point (e.g. DSK.DISKNAME)
and accesses the drive that contains this disk (if any).

A file name should be provided after the DSR name, using a decimal point
as a separator (e.g. DSK1.MYFILE). If no filename is provided after the
dot, the disk directory is accessed as a read-only, D/F 38 file.

If the disk directory is accessed, only opcode 0-2 (Open, Close and
Read) are recognized. Otherwise, the DSRs understands all the standard
PAB opcodes, except for Scratch Record (\>08).

####  0 Open

Opens a file, either exisiting or new. Puts its characteristics in a
file access block in VDP memory, provided there is one free.

- Get file type from PAB+1. Return with "bad attributes" error if a
  Dis/Fix file is opened for "append".
- Get record length from PAB+4. Return with "bad attributes" is a Var
  file has a record size of 255.
- If the file is open for "output":
  - Check if it exists. If yes return with error "file error".
  - Create the file. Return with "memory full " error if there is not 1
    sector free on disk, or if there are already 127 file pointers in
    sector 1.
  - Set file type and rec len in FDR. Use a default rec length of 80 if
    it is 0 in PAB.
  - Calculate the \# records per sector (using max value for var). Set
    it in FDR.
  - Get the \# of records needed from PAB+6
    - Return with "bad attributes" if it is greater than 32767.
    - If it is not 0:
      - Calculate the number of sectors needed (using max size for var).
      - Set it in FDR.
      - Add that number of sectors to the file.
  - Return
- If the file is not open for "output":
  - Check if it exists. If not:
    - Return with "file error" if it was open for "input".
    - Otherwise create the file: goto "output" above.
  - Get file status from FDR.
  - Return with "write protected" error if a protected file is opened
    for "update" or "append".
  - Return with "bad attributes" error if the file type does not match
    the one specified in PAB+1.
  - Check rec len in PAB+4:
    - If it is 0, replace it with the rec len taken from the FDR
    - If it is not 0, compare it with that in the FDR. Return with "bad
      attributes" if they differ.
  - If the file is opened for "append":
    - Get the \# of sectors and the eof-offset byte from the FDR.
    - Unless the file is empty, load the last sector into memory.
  - Return

####  1 Close

Closes the file. Frees the file access block in VDP memory.

- Find the file in the VDP buffers. Return with "file error" if not
  found.
- Save any modifications to the FDR and the current sector.
- Return.

####  2 Read

Input data from a file, one record at a time. Data is transfered into a
buffer located in VDP memory.

- Find the file in the VDP buffers. Return with "file error" if not
  found.
- Return with "illegal opcode" if the file was opened for "append" or
  "output" (from PAB+1).
- Get file type from FDR (not PAB+1).
- If fixed:
  - Get the record \# from PAB+6.
  - Calculate which sector it is in, using the rec/sect byte from the
    FDR.
  - If it is not in file: return with "EOF reached" error.
  - Load that sector into the data buffer are in VDP buffers.
  - Write rec length in PAB+5 (# of bytes read).
  - Transfer that number of bytes into the data buffer in VDP mem, whose
    address is at PAB+2.
  - Return.
- If variable:
  - Get the record \# from PAB+6.
  - Read the file sector by sector, count the records on each, until
    that number is reached (slow!).
  - If not in file: return with "EOF reached" error.
  - Get record length from the data in sector, write it length in PAB+5
    (# of bytes read).
  - Transfer that number of bytes into the data buffer in VDP memory,
    whose address is at PAB+2.
  - Return.

####  3 Write

Writes data to a file, one record at a time . Data is taken from a
buffer in VDP memory.

- Find the file in the VDP buffers. Return with "file error" if not
  found.
- Return with "illegal opcode" if the file was opened for "input" (from
  PAB+1).
- Get file type from FDR (not PAB+1).
- If fixed:
  - Get the record \# from PAB+6.
  - Calculate which sector it is in, using the rec/sect byte from the
    FDR. Check it it is in file, using the sect/file byte.
  - If it is not in file:
    - Append enough sectors to the file to fit that record in.
    - Or return with "memory full" error, if there is not enough room on
      disk.
  - If it is in file:
    - Load the sector it is on in the data buffer area in VDP memory.
  - Get \# of bytes to write from the record length byte in FDR.
  - Transfer that number of bytes from the data buffer in VDP mem, whose
    address is at PAB+2, into the data buffer area for that file.
  - Return.
- If variable:
  - Get the record \# from PAB+6.
  - Read the file sector by sector, count the records on each, until
    that number is reached.
  - If not it file:
    - Append enough sectors to file so as to fit that \# of records,
      assuming max size for each.
    - Or return with "memory full" error, if there is not enough room on
      disk.
  - Get \# of bytes to write from PAB+5.
  - See if it fits in the current sector. If not get next sector (if no
    more: append one).
  - Update the \# of sectors byte and the eof-offset byte in the FDR.
  - Write the \# of bytes as 1 byte at the beginning of the new record.
  - Transfer that number of bytes from the data buffer in VDP memory,
    whose address is at PAB+2.
  - Return.

####  4 Rewind

Sets the internal pointer to a given record in a relative file, to
record 0 in a sequential file.

- Find the file in the VDP buffers. Return with "file error" if not
  found.
- Return with "illegal opcode" if the file was opened for "output" or
  "append" (from PAB+1).
- Update data buffer if needed.
- If the file is sequential:
  - Write 0 into PAB+6 (record number).
- Return.

####  5 Load

Loads a memory image ("program") file into a buffer in VDP memory.

- Find FDR on disk. If not found return with "file error".
- Make sure the file is "program". If not return with "file error".
- Get the number of sectors in file. If it's 0 return with "file error".
- Get eof-offset byte from FDR, combine with \# of sectors to get the
  file size in bytes.
- Compare with the size in PAB+6. Return with "file error" if the file
  is too big.
- Write the whole file to VDP memory, at address found at PAB+2 (only
  the relevant part of the last sector if transfered).

####  6 Save

Saves data in a "program" file, from a buffer located in VDP memory.

- Check if file exists. If yes, return with error
- Create the file. Return with "memory full " error if there is not 1
  sector free on disk, or if there are already 127 file pointers in
  sector 1.
- Write the content of the buffer to the file, in as many sectors as it
  takes to store the \# of bytes specified in PAB+6. The last sector
  will be padded with garbage, if necessary.
- Set file type to "program", adjust \# of sectors and eof-offset bytes
  in FDR.

####  7 Delete

Removes a file from the disk directory.

- Find the file in the VDP buffers.
  - If found: read sector 1, find the FDR pointer for that file in the
    directory list.
  - If not found: find it on disk (this gives us the pointer in sector
    1).
    - If not found return.
- Free the sectors occupied by the file and its FDR in VIB bitmap
  (sector 0).
- Remove the FDR pointer from the directory list (sector 1).
- Clear the first character of the filename in the FDR copy in VDP
  buffers: invalidates the file.
- Return

####  8 Scratch record

Not implemented. Returns with "bad attributes" (why not "illegal
opcode"?).

####  9 Status

Returns the status of the specified file according to the following
format:

\>80: File not found
\>40: File is write protected
\>20 (not used)
\>10: File is internal
\>08: File is program
\>04: File is variable
\>02: Memory full
\>01: EOF reached

- Look for the file in the VDP buffers.
- If not there, look for it on disk.
  - If not found, set the "not found" bit and return with result in
    PAB+8.
  - If the file is found on disk:
    - Invalidate its newly loaded FDR in VDP buffers (so that it's not
      mistaken for an open file).
    - Set the file-type and file-protection bits and return with result
      in PAB+8.
- If the file is found in VDP buffers, it is open. Check if it's Var or
  Fix.
- If var:
  - Check if the record \# in PAB+6 is in file (using \# of rec/file
    byte in the FDR). If not:
    - Set the "EOF" bit.
    - See if there is room for one more sector on disk. If not, set the
      "memory full" bit
  - Add file-type and file-protection bits and return with result in
    PAB+8.
- If fixed:
  - Calculate the sector \# from the record \# in PAB+6 and the rec/sect
    byte in the FDR.
  - Check if the record is in the file. If not:
    - Set the "EOF" bit.
    - Calculate how many sectors would be needed for it. Check if there
      are that many free sectors on disk. If not, set the "memory full"
      bit.
  - Add file-type and file-protection bits and return with result in
    PAB+8.

####  0 Open directory

Allows to catalog a disk by opening a file called "DSKx." or
"DSK.DISKNAME." (i.e. missing a filename).

- Get file type from PAB+1.
- Return with "bad attributes" if the file is not Int/Fix, opened as
  "output".
- If rec len is 0, make it 38. If it's not 38 return with "bad
  attributes".
- Check if file was already open (look for a filename beginning with a
  space in the VDP buffers).
  - If yes, return with "file error".
- Check if there is a free buffer to open the file. If not, return with
  "memory full".
- Set the filename as " xxxxxxxx" (1 space and 9 bytes of garbage) in
  the VDP buffers.
- Return

####  1 Close directory

- Look for the file in the VDP buffers. If not found, return with "file
  error" (was not open).
- Clear the drive \# in the file control block (invalidate it).
- Return.

####  2 Read directory

Record 0 provides disk informations, records 1 through 128 provide file
informations.

- Look for the file in the VDP buffers. If not found, return with "file
  error" (was not open).
- Get record \# from PAB+6.
- Write it in the VDP buffer, after the space in the filename (first 2
  bytes of garbage). Why??
- Return with "EOF reached" error if it is greater than 128.
- For record 0:
  - Read VIB (sector 0) into data buffer area after FDR.
  - Get \# of sectors on disk from VIB
  - Count free sectors in VIB bitmap (upto last byte in sector, even if
    no drive can use that many sectors).
  - Pass parameters in VDP buffer whose address is found in PAB+2:
    - Diskname: a 10-characters ascii string (i.e. with a leading length
      byte).
    - 0 (as an 8-bytes floating point number, plus a leading size byte).
    - Number of sectors on disk (floating point number).
    - Number of free sectors on disk (floating point number).
- For other records:
  - Read sector 1 (FDR list) into data buffer area.
  - Get the pointer for the file corresponding to that record number.
  - If the pointer is null, pass parameters in the VDP buffer whose
    address in found in PAB+2:
    - Empty string (size byte = 0).
    - 0 (as an 8-bytes floating point number, plus a leading size byte).
    - 0
    - 0
  - If the pointer is valid:
    - Read FDR into data buffer area (not into FDR buffer: this is for
      directory file).
    - Get the \# of sectors from FDR, add 1 for FDR itself. Get the
      file-type and rec-len bytes.
    - Pass parameters in VDP buffer whose address is found in PAB+2:
      - Filename: a 10-characters ascii string.
      - File type: 1=D/F 2=D/V, 3=I/F, 4=I/V, 5=Program. Negated if the
        file is protected.
      - File size, in sectors including the FDR.
      - Record length.
- Write 38 in PAB+5 (# of bytes read), although there may be less than
  that if the string is less than 10 chars.
- Return.

###    The subprograms

Most of these subprograms have 1-char long names, the character being in
the range \>10 to \>16. Therefore, they cannot be called from Basic (the
only exception is FILES). Each subprogram expects parameters to be
passed at specific addresses in the scratch-pad RAM (generally
\>834A-\>8350).

Subprograms can be classified in three categories.

- Low-level subprograms (Sector I/O and Format disk) whose function is
  to make higher level routines independent on the physical disk medium.
- Mid-level subprograms (File protect and File rename) that introduce
  the concept of files, as a block of information but don't care how the
  information is organised in the file. They call low-level subroutines
  to perform all disk operations.
- High-level subprograms (File input, File output, Number of files) deal
  with data buffering from files to VDP memory and conversely.
- DSRs provide more sophisticated file access, that take the format of
  the data into account. They call the above subprograms to perform most
  of their functions.

####  \>10 Sector I/O

This subprogram reads or writes a sector from the disk. The number of
bytes is always 256.

Input parameters

-
Drive # (1-3)
0: write
&lt;&gt;0: read
VDP buffer address
Sector number

Output parameters

Sector number
-
-
-
Error code

#### \>11 Format disk

This routine formats a disk.

Input parameters

-
# of tracks
VDP buffer address
Density
DSR: 0 = standard format, can be done by DSR version 1.0
1 = needs DSR version 2.0 (double-sided, or track \# not 35 nor 40)
2 = needs a higher DSR version (double density, etc)

Output parameters

# of sectors / disk
-
-
-
Error code

#### \>12 File protection

This routine is used to modify the protection status of a file, i.e
whether the file can or cannot be deleted and modified.

Input parameters

Drive # (1-3)
0: unprotected
&gt;FF: protected
Filename ptr (in VDP mem)
-

The filename must be a left-justified, 10-char string. Add as many
trailing spaces as necessary.
Output parameters

-
-
-
Error codes

#### \>13 Rename file

This routine changes the name of a file on a given drive.

Input parameters

Drive # (1-3)
-
New name ptr (in VDP mem)
Old name ptr (in VDP mem)

Filenames must be 10 character strings, with as many trailing spaces as
necessary. No check is made to ensure that the new name is legal!
Output parameters

-
-
-
Error codes

#### \>14 File input

This routine reads data from a file sector-wise, irrespective of its
type, record length, etc. For instance, it can read a "program" file by
chunks of 512 bytes (or whatever is convenient).

Input parameters

Drive # (1-3)
# of sectors
Filename ptr (in VDP mem)
Info ptr (&gt;83xy)
-

\# of sectors: 0 = Get file info (fill the structure below, using data
from the file's FDR)
\<\>0 = Read that number of sectors (from first sector indicated in the
structure below).

VDP buffer address
First sector #
File type
Recs / sector
EOF offset
Rec lenght
# of rec
-
Output parameters

-
# of sectors read
-
Error codes

Result of get file info:

-
-
File type
Recs / sector
EOF offset
Rec lenght
# of rec
-

####  \>15 File output

This routine writes data to a file sector-wise, irrespective of its
type, record lenght etc.

Input parameters

Drive # (1-3)
# of sectors
Filename ptr (in VDP mem)
Info ptr (&gt;83xy)
-

\# of sectors: 0 = Create file (using info in the structure below to
create the FDR)
\<\>0 = Write that number of sectors (from first sector indicated in the
structure below).

VDP buffer address
First sector #
File type
Recs / sector
EOF offset
Rec lenght
# of rec
-
Output parameters

-
# of sectors writen
-
Error codes

####  \>16 Number of files

This subprogram sets the number of file access blocks reserved at the
end of the VDP memory. Thus, it controls the number of files that can
simultaneously be open. The maximum is 16.

Input parameters

# of files(1-16)
-
-
-
Output parameters

-
-
-
Error codes

#### FILES Number of files

This is the same subprogram as \>16, except that it is meant to be
called from Basic. It limitates the number of files to 9.

------------------------------------------------------------------------

## Floppy disk drives

[Models
](#Drive%20models)[Connection cable
](#Connection%20cable)[Terminal resistor pack
](#Resistor%20pack)[Shunt pack
](#Shunt%20pack)[Power supply](#Power%20plug)

### Models

Many 5/25" floppy drives are compatible with the TI-99/4A disk
controller. Here are a few examples:

Texas Instruments PHP1250
Magnetic Peripherals BR8B1A
Mitsubishi M4851-362U
MPI 51 & 52
Pertec FD200
Qumetrak 142
Remex RFD480
Shugart 400L, 450
Siemens 82, FDD100-5
Tandon TM65-2, Tm100-2A
Wango 82

###  Connection cable

The connection cable follows a standard convention:

    #  I/O  Name            Function
    -- ---- --------------- ---------
     2         ?             (used on PCs, for 1.44 MB drives)
     4         -             not used
     6  >    SEL4            Select DSK4 (not on original TI FDC)
     8  <    INDEX           Index pulse (hole in floppy detected)
    10  >    SEL1            Select DSK1
    12  >    SEL2            Select DSK2
    14  >    SEL3            Select DSK3
    16  >    STROBE          Motor on
    18  >    DIR             Direction to step at (0=out, 1=in)
    20  >    STEP            Step by one track
    22  >    WDATA           Data output
    24  >    WGATE           Enable data output
    26  <    TRACK0          Track 0 reached
    28  <    WPROTECT        Write protection detected
    30  <    RDATA           Data input
    32  >    SIDE            Side selection
    34        -              not used

    All odd numbered pins are connected to ground

So that you don't have to set your drive number yourself, Texas
Instruments provided small connectors that redirect the SEL1, SEL2 and
SEL3 lines, in the following way:

    Controller  Drive 1    Connector    Drive 2    Connector    Drive 3
    SEL1 >------ SEL1 ------ ,---------- SEL1 ------ ,---------- SEL1
    SEL2 >------ SEL2 -------' ,-------- SEL2 -------' ,-------- SEL2
    SEL3 >------ SEL3 ---------' ------- SEL3 ---------' ------- SEL3

See how it works? All drives are set to react to SEL1, but for the
second drive SEL1 is actually connected to the SEL2 line. Similarly, for
the last drive, SEL1 is actually connected to the SEL3 line. This
technique is known as "wire twisting".

###  Terminal resistor pack

Some of the connection lines require a pull-up resistors: SEL1, SEL2,
SEL3, STROBE, DIR, SETP, WDATA and WGATE. For reasons that are not very
clear to me, the pull-up resistors must be located at the far end of the
cable, i.e. in the last drive in the chain. This means that this drive
MUST ALWAYS BE TURNED ON, even if you are not actually using it.
Otherwise signals will not carry properly.

Normally, all drives come out of factory with a resistor pack installed.
However, we cannot leave the pack in place in each drive as this would
pull up the lines too strongly and may damage the controller card.
Therefore, you MUST remove the resistor pack from each drive but the
last one in the chain. Let me mellow that: in fact, this depends on the
value of the resistor. Most drives use 150 ohm resistor, and having more
than 1 pack installed may damage the card, but some manufacturers (e.g.
TEAC) use 500 ohm resistors and these can be left in place without any
risk of damaging the controller (three 500 ohms resistors in parallel
add up to 167 ohms).

Most of the time, the resistors come in the form of a DIP pack
(dual-in-line) that looks like a small integrated circuit. It is
generally installed on a socket, so you can remove it easily. Make sure
you save it, in case you want to reinstall it later...

###  Shunt pack / DIP switch

As floppy disk drives were built to work on many different systems, they
can be configure according to the user's needs. Sometimes, this is done
by cutting connections on a so-called shunt pack. Occasionally, the
shunt pack is replaced with a DIP-switch pack which only requires
flipping tiny switches. Finally, you may encounter jumpers, i.e. two
small pins connected by a little cap: just remove the cap to open the
connection.

The problem is that, even though there is a convention on which options
should be made available, each manufacturer does things in a slightly
different way, labels the connections with different names, arranges
them in a different order, or even splits the pack in several jumpers.
Normally, you should see a single 14 or 16-pin pack (or switch) with the
following labels:

HL or HS or H: Head loaded by drive selection line.
DS0 or DS1: Drive selected by SEL1.
DS1 or DS2: Drive selected by SEL2.
DS2 or DS3: Drive selected by SEL3.
DS3 or DS4: Drive selected by SEL4.
MX or X: mutiplex. Caution: DS4 and MX are often inverted.
HM or MH or M: Head loaded by the Strobe signal.

In case of doubt, follow the SEL1 connection (pin 10 on the cable) and
see where it ends in the shunt pack.

#### Settings

As mentionned above, all drives should respond to SEL1, because we are
using twisted connectors to redirect SEL2 and SEL3 to the SEL1 line.
Thus, open all DS connections, except for DS0 (sometimes labelled DS1).
If you decide for a straight cable solution (for instance, if you have
twin, slim drives) connect the line you want and leave the other three
open. Each drive should have one and only one DSx line connected.

HL and HM are mutually exclusive. They determine which signal should
cause the magnetic head to be loaded on the disk: the SELx signal or the
STROBE signal. Normally, you would want it to be the selection signal,
so that only the drive you are accessing loads its head. To this end,
connect HL and open HM. However, if you have only one drive, using HM
may result in a slightly better performance (in this case, you should
also connect MX).

MX determines whether the drive output lines should be enabled by the
selection signal (MX open) or be permanently enabled (MX connected).
Obviously, when using multiple drives MX should be left open. In a
single-drive system, connecting MX may slightly improve the drive
performance.

###  Power connector

The power connector in the drive looks like this (looking inside the
drive plug):

    ,---+
    | O | +12V
    | O | Gnd (for 12V)
    | O | Gnd (for 52V)
    | O | +5V
    `---+

[Previous page](disks.md)

Preliminary version. 3/19/99. Not for release.
Revision 1. 5/15/99. OK to release (fully disassembled card ROM).
Revision 2. 5/30/99. Tested & debugged examples.

Revision 3. 7/4/99. Added an explanation of the CRC.

Revision 4. 7/13/99. Added a paragraph on magnetic media and write
precomp.
Revision 5. 9/18/99. Added data on the FD1771, a word on the CorComp
card.
Revision 6. 6/16/00. Got the FD171 manual. Added section on drives.
Split in two pages.
[Back to the TI-99/4A Tech Pages](titechpages.md)
