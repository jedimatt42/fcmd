# Cassette tape interface

The TI-99/4A was issued with a built-in interface for cassette tape
recorders. This was meant as a cheap alternative to a disk drive. The
console contains the necessary hardware to read from, write to, and
control the motor of two tape recorders. The relevant software is
located in the console ROMs and GROMs.

Texas Instruments originally recommended the following recorders, but
many others will work. I even used the one on my stereo, with
tune-tracking system, etc. Most of the time the volume control should be
set at mid-range, and the tone control (if any) at the highest possible
pitch.
 

|            |         |            |          |
|------------|---------|------------|----------|
| Brand      | Model   | Volume set | Tone set |
| Superscope | C2L00LP | 8.0        | N/A      |
| Panasonic  | RQ2309A | 5.0        | 10       |
| Sears      | 2165    | Mid Range  | N/A      |
| Sears      | 21686   | Mid Range  | HI       |
| JC Penney  | 6568    | Mid Range  | High     |

**Hardware  **
[Connector](#Connector)
[Internal circuitery](#Internal%20circuitery)
[CRU map](#CRU%20map)

**Software  **
[Direct control](#Direct%20control)
[Cassette tape format](#Cassette%20tape%20format)
[ROM routines](#ROM%20routines)
[DSRs in GROM](#DSRs%20in%20GROM)

##  Hardware

### Connector

                     ___________ 
                    ( 1 2 3 4 5 ) 
                     \ 6 7 8 9 / 
    # I/O Use         \_______/ 
    - --- ------- 
    1  >  Cass 1 motor control
    2  >  Ditto (negative)
    3  >  Output to tape 1 or 2 (neg)
    4  >  Audio gate
    5  >  Output to tape 1 or 2 
    6  >  Cass 2 motor control
    7  >  Ditto (negative)
    8  <  Input from tape 1
    9  <  Ditto (neg)

Texas Instruments marketed a connection cable, with a 9-holes connector
at one end, and 5 jacks at the other (three for one tape recorder, two
for the other).
Pins 5 and 3 go to two jacks (on red cables) to be plugged in the the
microphone socket on each recorder.  Pins 8 and 9 go to a jack (on a
white cable) to be plugged in the "ear-plug" socket of  recorder \#1.
Finally pins 1 and 2 go to a smaller jack (on a black cable) to be
plugged into the remote control socket on recorder \#1, while pins 6 and
7 play the same role for recorder \#2.

As you see, only one recorder can be read from. I'm not sure why. May be
because, if the output lines of two recorders are connected together,
there is a risk that one is damaged when the other is outputing sound?

An important point is that the connections are polarised (see circuitery
below). In particular, the remote-control jacks are connected to
transistors, not to relays. This may cause some tape recorders to
malfunction if their polarity is inverted with respect to that of the
cassette jack. But it's a trivial job to build an adapter: get a male
and a female jack and just cross their wires. I think you can even buy
such adapters.

###  Internal circuitery

#### Motor control

----+
    |       ,---+5V
  P6|_____|/         Opto-isolator
    |     |\ TIS92    TIL119                         F
    |       V__________    ,---------+----+---------uuu---&gt; Pin 1
    |                  | |/          |    |   1nF         CS1 motor
    |                  V |\________|/     &#39;---||---Gnd    jack
    |                  |           |\
    |                 Gnd      TIS92 V___,----------------&gt; Pin 2
    |                                    |   1nF
    |       ,---+5V                      &#39;---||---Gnd
  P7|_____|/         Opto-isolator
    |     |\ TIS92    TIL119                         F
    |       V__________    ,---------+----+---------uuu---&gt; Pin 6
    |                  | |/          |    |   1nF         CS2 motor
    |                  V |\________|/     &#39;---||---Gnd    jack     |                  |           |\
----+                 Gnd      TIS92 V____,---------------&gt; Pin 7
                                          |   1nF
                                          &#39;---||---Gnd

####  Magnetic output

----+
    |   6.8K      1nF  5.6K
  P9|---WWW---+---||---WWW---+-------------------+---&gt; Pin 5
    |         |              |   200 Ohm         |   Microphone     |         = 0.1uF        +---WWW---Gnd   1nF =   jacks
    |         |                                  |   CS1 + CS2
----+        Gnd                           Gnd---+---&gt; Pin 3

####  Magnetic input

                               +---||---+
TMS                            |  39K   |
9901             120K          +--WWW---+
---+          +--WWW---+       |    /|  | 6.8K  10nF           F
   |          |    /|  |  6.8K |   /6|--+--WWW--||--+---+-----uuu----&lt; Pin 8
   |     2.2K |   /3|--+--WWW--+__/7 |              |   |            Ear-plug P11|--+--WWW--+__/1 |  |          \  |              |   +--||--Gnd   jack    |  |          \  |  = 0.22uF    \5|--Gnd         |           F    CS1
   |  ^ PG3992    \2|--+            \|              |       +--uuu---&lt; Pin 9
   |  |            \|  |           4558             | 15 Ohm|
   | Gnd         4558 Gnd                 ,---------+--WWW--+--||--Gnd
   |                                      |           5.6K  |  1nF
   |               +---WWW----------------&#39;      Gnd---WWW--&#39;
   |               |   10K
   |               |         +---WWW---+5V
   |               |         |   9.1K
   |   12K     ,---+---WWW---+--------------------WWW-------+--------&gt; Pin 4
 P8|---WWW---|/    |   10K   |    Sound chip      5.6K      |
   |         |\    = 10nF    |     TMS9919                  =
   |    TIS92  V___|         |   +---------+                |
   |               |         &#39;---|AUDIOIN  |               Gnd
---+              Gnd            |         |
                                 +---------+

###  CRU map

|     |          |     |                                      |
|-----|----------|-----|--------------------------------------|
| Bit | R12 addr | I/O | Usage                                |
| 22  | \>002C   | O   | 1: Turn motor on for recorder \#1    |
| 23  | \>002E   | O   | 1: Turn motor on for recorder \#2    |
| 24  | \>0030   | O   | Audio gate 1: Silent input           |
| 25  | \>0032   | O   | Data output to recorders \#1 and \#2 |
| 27  | \>0036   | I   | Data input from recorder \#1         |

##   Software

### Direct control

You can control the tape recorders directly, using the CRU bits
mentionned above. Note that it does not have to be tape recorders... I'm
thinking of using the cassette port to hook-up two TI consoles and have
a debugger program running on one and monitor the other. Communications
would be slow (in serial mode by definition), but it would not require
accessing any peripheral card.

* It assumes recorder #1 is set to record, and #2 to replay.    `
CSTEST LI   R12,&gt;0000             CRU address of the TMS9901
       SBO  23                    Turn motor on, recorder #2
       BL   @WAIT                 Let it reach cruising speed
       SBO  25                    Toggle output
       BL   @DELAY                Leave it so, for a while
       SBZ  25                    Toggle back
       ...                        etc.`
CSTES2 CLR  R12                   CRU address = &gt;0000
       SBO  22                    Motor on, recorder #1
       BL   @WAIT
       TB   27                    Get data from recorder
       JEQ  ...                   Do something
       JNE  ...  

###   Cassette tape format

Texaz Instruments adopted a frequency modulation encoding system to
store data on tape. This is only a convention, and you may come up with
another, if you feel like it. Similarly, TI defined the format the data
should have whithin a tape file. Again, this is only a convention.
 

#### Data encoding

Bits are encoded by output level *changes*. With a 3 MHz console, the
output toggles every 725.3 microseconds. To encode a 1, invert the
output in the middle of this time period:

      0   0  _1  _0_  1_  1_  0     Bits to encode
    |   |___| |_|   |_| |_| |___|   Output (low/high) 
    c   c   c   c   c   c   c   c   Clock intervals


Courtesy Dean Corcoran

This results in frequencies of 689.37 Hz for a space (0) and 1379 Hz for
a mark (1) which is well within the audio range, thus suitable for a
tape player. In addition, I was told that you can connect the cassette
port to a PC sound card and save/load programs as .wav files (The "Scott
Adams compilation" CD ROM by Frank Traut uses this trick, if I'm well
informed).
 

#### File format

    Name      # of bytes Content 
    --------- ---------- -------- 
    File sync     768    >00 

    Data mark       1    >FF 
    Size            1    # of records 
    Repeat size     1    ditto

    Rec sync        8    >00                       } 
    Data mark       1    >FF                       } Rec 1 
    Data           64    data bytes                } 
    Checksum        1    sum of the 64 data bytes  }

    Repeat rec 1

    Rec 2 
    Repeat rec 2

    etc.
At the beginning of the files are 768 zeros (\>300), for synchronisation
purposes. The motor speed can vary a little from recorder to recorder
(not too much, otherwise you would notice it when listening to music).
So the tape reading routine in the console ROMs uses this stretch of
zeros to time the tape recorder. Once it has determine how long a "0"
bit lasts, it is a simple matter to detect a "1" which cuts this
interval in half.

The data mark signals the end of the sync stretch. It is followed by the
number of records, repeted twice (to ensure reading is correct).

Each record in the file is repeted twice. It begins with a short sync
stretch of 8 zeros, followed by an \>FF data mark. The only allowed file
format is Dis/Fix 64, so there is always 64 data bytes  in a record. The
record ends with a checksum: this is the sum of all 64 data bytes: it is
likely to be greater than 256, and thus requires two bytes. However,
only the least significant byte is recorded on tape.

The tape reading routine calculates its own checksum while reading data,
then compares it with the recorded value. If it does not match, the
routine will get another chance, as the record repeats. If the checksum
matches the first time, the second repeat is just ignored. This format
slows down cassette operations by a factor of two, but is more
user-friendly: since a tape recorder is by definition sequential, a bad
record that aborts reading forces you to start all over again, from the
beginning of the file. In the worst case, it means the file cannot be
read. At least with this method, even if a record is unreadable its copy
might still be good. The probabilty that both copies go bad is very low
(the square of the probability that one record goes bad).

There is no special end-of-file mark. Recording just stops after the
second repeat of the last record.
 

###  ROM routines

There are three cassette operating routines in the console ROM: cassette
write, cassette read and cassette verify. All three can be called by the
GPL opcode "I/O", with respectively 3, 4 or 5 as a source argument. The
destination argument consists in two words: the number of bytes to
transfer, and the VDP buffer address.

|           |         |                |
|-----------|---------|----------------|
| Operation | GPL I/O | Address in ROM |
| Write     | 3       | \>1346         |
| Read      | 4       | \>142E         |
| Verify    | 5       | \>1426         |

For obvious reasons, they all require precise timing and this is done by
using the built-in timer in the TMS9901 interface controller chip, which
is part of the console. The cassette routines enable the interrupt
function of the TMS9901, which requires taking control of the interrupt
service routine. This is done by setting a flag bit (value \>20) in byte
\>83FD. From now on, the console main ISR will treat any interrupt as
issued by the TMS9901 and branch to the cassette ISR at \>1404. If you
are not familiar with the concept of interrupts service routines (ISR)
you may want to have a look at the page on [interrupts](ints.md), as
well as at the page describing the [TMS9901](tms9901.md).

What all this implies is that no other interrupt should be allowed, and
cassette routines begin by masking them off (by setting CRU bit 1 and 2
to 0 in the TMS9901). This would be necessary anyway, given the nature
of a tape recorder: once you start reading you can't stop and come back
later, as the tape keeps running! A temporary interruption would mean
skipping one or more records, which would force the user to start the
cassette operation all over again.

* It is adapted from the one in the console ROM (at &gt;13CC-13E0)`
SETCS  LI   R3,&gt;0023       Timer value (&gt;0011) plus clock bit, inverted
       CLR  R1             Flag: no vector in R6
       CLR  R12            Console CRU base
       SOC  @H0020,R14     Set bit &gt;0020 in &gt;83FD (WS is &gt;83E0 in GPL)
       SBZ  2              Disable VDP interrupts
       SBZ  12             This pin is not connected (INT12: +5V pull-up)
       LDCR R3,15          Load &gt;0011 in decrementer
       SBZ  0              Leave clock mode
       SBZ  1              Disable peripheral interrupts
       SBO  3              Enable clock interrupts
       B    *R11`
* This routine returns to normal operations
* It is found at &gt;155E-1570 in the console ROM`
CLRCS  SZC  @H0010,R14    Clear read/verify flag
       SZC  @H0020,R14    Clear cassette ints flag (byte &gt;83FD)
       SBZ  3             Disable cassette interrupts
       SBO  12            Enable interrupts by pin INT12 (nc)
       SBO  1             Enable VDP interrupts
       SBO  2             Enable peripheral interrupt
       B    *R11          The real routine returns to the GPL interpreter:
*      B    @&gt;0070

#### Cassette write

Upon writing, the timing is ensured by loading \>0011 (i.e. 17) into the
clock register of the TMS9901 chip. The resulting delay is:

          17     = 363.6 usec 
     (3MHz / 64)

The write subroutine writes to the output by toggling CRU bit 19 and
entering a forever loop:
The only way to get out of this loop is via the interrupt that will
occur once the timer has elapsed. The cassette ISR checks whether the
main program is trapped into such a loop and (if this is the case)
returns to the next instruction, thereby effectively jumping out of the
loop.

* R8 contains either &gt;1E19 which means SBZ 19
* or &gt;1D19 which means SBO 19
* This routine is located in the console ROM at &gt;13E2-1402`
EMITBY LI    R6,&gt;0008      Bit counter: 8 bits per byte
       INV   R4            Invert the byte
LP1    JMP   LP1           Wait for an interrupt`
       X     R8            Toggle CRU bit 19
       XOR   @H0200,R8     Change SBZ to SBO and conversely
LP2    JMP   LP2           Wait for an interrupt`
       MOV   R4,R4         Test leftmost bit
       JLT   SK1           It&#39;s &quot;1&quot; (was &quot;0&quot; before inversion)
       X     R8            It&#39;s &quot;0&quot; (was &quot;1&quot;): toggle CRU bit 19
       XOR   @H0200,R8     Change SBO to SBZ and conversely
SK1    SLA   R4,1          Next bit
       DEC   R6            More to do?
       JNE   LP1           Yes
       B     *R11          No: return. R8 is ready for next byte

And here is the infamous cassette-specific interrupt service routine:

* The main ISR enters it with WS &gt;83E0 if bit &gt;0020 is set in &gt;83FD`
CASISR SBZ  0              Make sure we&#39;re not in timer mode
       SBO  3              Clear pending interrupt (remain enabled)
       MOV  R1,R1          Flag: is there a vector in R6?
       JLT  SK2            Yes
       LWPI &gt;83C0          Back to ISR workspace
       C    *R14,@HERE     Is instruction at return point a forever loop?
       JNE  SK2            No: branch to vector in R6 (crashes if none!!)
       INCT R14            Skip the HERE JMP HERE
       RTWP                Return just outside the loop`
SK2    LWPI &gt;83C0          Branch to vector in R6, with ISR workspace
       MOV  @&gt;83E6,R14     AAAARGH! Overwrites the return address!!!
       RTWP                Branch, with no hope of return

Now, all Write has to do is to use the above routines to send the
appropriate bytes. Please refer to [file
format](#Cassette%20tape%20format), above.

- Get \# of bytes (round in up to next 64, derive the \# of records) and
  the buffer address in VDP memory
- Initialize cassette interrupts (with SETCS above)
- Allow interrupts with `LIMI 1`
- Send \>300 times byte \>00 (file synchronisation)
- Send byte \>FF once
- Send \# of records, twice
- Send data in 64 bytes records. Each record is sent twice:
  - Send 8 times byte \>00 (record sync)
  - Send byte \>FF once
  - Send 64 bytes
  - Send checksum as two bytes
- Wait for interrupt
- Branch to CLRCS (return to GPL interpreter, which resets LIMI 0)

####  Cassette read

The Read subroutine also makes use of the timer to time the stretches of
"0" bits in the sync field at the beginning of each record. First it
loads the maximum value (\>3FFF) in the TMS9901 timer and waits for 8
bits to arrive. Then it reads the timer and see how much time has
elapsed. It uses 8 bits rather than just one to get an average value.

* If yes, it returns with B *R11
* If no, it returns with INCT R11, B *R11
* R1(lsb) contains the previous value of the input (&gt;xx00 or &gt;xxFF)
* It&#39;s located at &gt;15BA-15D2 in the console ROM `
ISTOGL TB   27            Check tape input port
       JEQ  ISONE         It&#39;s 1
       CZC  @H00FF,R1     It&#39;s 0. What was it before?
       JEQ  NOTOGL        It was 0: no toggle
TOGGLE XOR  @H00FF,R1     Save new value
       B    *R11          Return, signaling a toggle`
ISONE  CZC  @H00FF,R1     Input is 1. What was it before?
       JEQ  TOGGLE        It was 0: toggle detected
NOTOGL INCT R11           It was 1: signal no toggling
       B    *R11          Return`
* This routine makes use of the above to time the cassette recorder
* It assumes a &gt;00 byte is coming and waits for the input to
* toggle 8 times.`
TIMECS LI   R9,&gt;7FFF      Timer value &gt;3FFF, plus clock bit
       LI   R8,&gt;0008      Count 8 toggling
       LDCR R9,15         Load the decrementer
       SBZ  0             Leave clock mode (which starts the timer)
       SBO  3             Enable interrupts (in case no toggle occurs)
LP3    @ISTOGL            Check if input toggled
       JMP  YES           Yes, it did
       JMP  LP3           Not yet
YES    DEC  R8            More to count?
       JNE  LP3           Yes`
       SBO  0             Enter clock mode
       STCR R3,15         Get value remaining in timer
       S    R3,R9         Calculate # of times it was decremented
       MOV  R9,R3         Multiply by 5
       SLA  R9,2
       A    R9,R3
       SRL  R3,6          Divide by 64
       ORI  R3,&gt;0001      Add clock bit
       CI   R3,&gt;001F      Check if resonnable value
       JLT  AGAIN         Too fast (not &gt;00): reinit and try again
       ...                (Actually, jumps to SK3 in GETBIT, below)
       LDCR R3,15         Load new value
       SBZ  0             Leave clock mode
       SBO  3             Enable timer interrupts
       ...

Once this value has been determined, it is fed to the timer for further
reading operations. If the timer fires before the output toggles, the
incoming bit is a "0". If the output toggles while the timer is still
mid-way to zero, the incoming bit is a "1". Simple enough, no?

* Once done, it resets the timer with the value calculated above (in R3).
* It is located at &gt;1572-159E in the console ROM`
GETBIT MOV  R11,R10
LP0    JMP  LP0           Wait for interrupt
       BL   @ISTOGL       Check if input toggled
       INCT R10           No: bit is &quot;0&quot;`
       ORI  R1,&gt;FF00      Flag for ISR: use vector in R6 if times out
SK3    CZC  @H00FF,R1     What is the detected input status?
       JEQ  LP5           It&#39;s 0
LP4    TB   27            It&#39;s 1. We must wait until it toggles again
       JNE  LP5           It did. Now wait for a 1
       JMP  LP4           Not yet, keep waiting`
LP5    TB   27            Wait till input toggles back to 1
       JNE  LP5           Not yet, keep waiting`
       LDCR R3,15         Reload delay in clock
       SBZ  0             Leave clock mode
       SBO  3             Enable timer interrupts
       ANDI R1,&gt;00FF      Clear ISR flag
       XOR  @H00FF,R1     Update last input bit
       B    *R10`
* This routine receives a byte.
* It uses the above one to build the byte in R4.
* It is located at &gt;15A0-15B8 in the console ROM.`
GETBY  LI   R8,&gt;0008      Bit counter: 8 bits per byte
       CLR  R4            Init byte
       MOV  R11,R9        Save return point
LP6    SLA  R4,1          Next bit
       BL   @GETBIT       Get it
       JMP  SK4           It was a &quot;0&quot;
       INC  R4            It was a &quot;1&quot;: put it in R4
SK4    DEC  R8            More to do?
       JNE  LP6           Yes
       A    R4,R7         Build checksum in R7
       B    *R9           Return

Now, here is how Read works:

- Get max \# of bytes (derive max \# of records) and buffer address
- Initialize cassette interrupts (see SETCS above) with a default delay
  of \>0015
- Receive all records:
  - Allow interrupts with LIMI 1
  - Set ISR vector in R6 to come here
  - Wait a long time. If it is elapsed, return with Cnd bit set in
    \>837C.
    - Receive bytes until 6 in a row are \>00 (12 for top of file)
    - Times the tape recorder with TIMECS, put the new value in timer
    - Receive bits until 8 are "1" (i.e. the \>FF mark arrived)
    - Set ISR vector to "try once more, then error"
    - At top of file, get the number of records:
      - Receive one byte
      - If higher than max \# allowed, return with Cnd bit and error bit
        (\>01) in \>837C
      - Receive the 2nd copy. If different return with error
    - Elsewhere, get a record:
      - Receive 64 bytes, transfer them to VDP buffer
      - Receive the checksum byte. Check if it matches
        - If not try once more. Then return with Cnd bit and error bit
          in \>837C
        - If checksum matches the first time, skip the repetition of the
          record
  - Next record
- Branch to CLRCS

####  Cassette verify

This routine is nothing more than a cassette read that compares the
incoming data with the content of the buffer, instead of filling the
buffer with them. If a mismatch occurs, the routine returns with bits
\>21 set in \>837C (GPL status byte).

- Set flag to indicate verify: bit \>0010 in R14 (byte \>83FD)
- Set VDP to read from data buffer, rather than to write to it
- Enter the Read subroutine

###  DSRs in GROM

There are two DSR in the first console GROM: **CS1** and **CS2**. They
can be found as GROM addresses: \>1320-16DC. As you might expect, CS1
deals with the first tape recorder, and CS2 with the second. These are
unusually high-level DSRs, that directly interact with the user. For
instance, they display prompting messages on screen and wait for the
user to press a key. It is one of the rare occurences when byte 8 in the
PAB is effectively used to pass a screen bias to a DSR.

By the way, the prompting messages are send by calling a subprogram,
whose name is \>03. It must be called with the screen address in \>8364
and the a string number in \>8362. This number (0 to 32, by steps of 2)
is an offset into a table of 2-byte long string pointers located at GROM
address \>15A0. For some strings, the subprogram also adds the cassette
number (CS1 or CS2) to the string and displays "then press enter". In
any case it beeps by calling the "accept tone" GROM routine at \>0034.

As mentionned above, the file type must be display, variable 64, and
sequential. In addition, CS2 can only be opened for output (as recorder
\#2 cannot be read from).

#### Open

- Set record length to 64, of none is provided in the PAB. Else round it
  up to 64

- If file "variable" or "relative" return with "Illegal opcode" code in
  PAB byte 1

- If file is opened as "update" or "append" return with "Illegal opcode"
  code in PAB byte 1

- If file is opened as "input":
  - If cassette is CS2, return with "Illegal opcode" code in PAB byte 1
  - Turn motor on. Wait for 30 VDP interrupts (0.5-0.6 sec).
  - Display "Rewind cassette tape. Then press enter". Beep
  - Call GETKEY
  - Turn motor off. Wait for 30 VDP interrupts (0.5-0.6 sec). Beep
  - Display "Press cassette play. Then press enter". Beep

- If file is opened as "output":
  - Turn motor on. Wait for 30 VDP interrupts (0.5-0.6 sec).
  - Display "Rewind cassette tape. Then press enter". Beep
  - Call GETKEY
  - Turn motor off. Wait for 30 VDP interrupts (0.5-0.6 sec). Beep
  - Display "Press cassette record. Then press enter". Beep

- Call GETKEY

- Turn motor on.. Wait for 30 VDP interrupts (0.5 sec. 0.6 sec for
  european consoles).

- Wait for 300 VDP interrupts (10 seconds. 12 sec for european
  consoles).

- Turn motor off.. Wait for 30 VDP interrupts (0.5-0.6 sec).

- Goto EXIT

  **EXIT**

- Turn motor on. Wait for 30 VDP interrupts (0.5-0.6 sec)

- Scroll up one line, move cursor to column 1

- Return to Basic (or caller, whatever it is)

  **GETKEY**

- Wait for a key to be pressed

- If it is \
  - Return to caller

- If it is \:
  - Re-enter DSR from the beginning

- If it is \:
  - Goto CHECK

- If it is \:
  - Set error code for "Device error" in PAB byte 1
  - Display "Press cassette stop. Then press enter". Beep
  - Call GETKEY
  - Goto EXIT

- If another key was pressed, keep waiting

  **CHECK**

- Display "Rewind cassette tape. Then press enter". Beep

- Call GETKEY

- If \ was pressed. turn motor off. Wait for 30 VDP interrupts
  (0.5-0.6 sec). Beep

- Display "Checking". Beep

- Wait for the beep to end

- Call cassette verify ROM routine with GPL opcode I/O 6

- If no error occured:
  - Display "Data OK". Beep
  - Goto Close

- If an error occured:
  - Turn motor off. Wait for 30 VDP interrupts (0.5-0.6 sec). Beep
  - Check the error bit (value \>01) was set in \>837C:
    - If set, display "Error detected in data". Beep
    - Else display "Error - no data found". Beep
  - Check opcode:
    - For "Save", display "Press R to record, C to check, E to exit".
      Be-be-beep
    - For others, display "Press R to read, C to check, E to exit".
      Be-be-beep
  - Call GETKEY
  - Re-enter DSR from the beginning

####  Close

- Turn motor off. Wait for 30 VDP interrupts (0.5-0.6 sec).
- Display "Press cassette stop. Then press enter". Beep
- Call GETKEY
- Goto EXIT

####  Read

- Copy record length in character count
- Turn motor on. Wait for 30 VDP interrupts (0.5-0.6 sec).
- Call the ROM routine, with GPL opcode I/O 5. Number of bytes taken
  from record length.
- If Cnd bit set when returning, an error occured:
  - Set error code for "Device error" in PAB byte 1
  - Display "Press cassette stop. Then press enter". Beep
  - Call GETKEY
  - Goto EXIT.
- Else turn motor off. Wait for 30 VDP interrupts (0.5-0.6 sec).
- Goto EXIT

####  Write

- Turn motor on. Wait for 30 VDP interrupts (0.5-0.6 sec).
- Call the ROM routine, with GPL opcode I/O 4. Number of bytes taken
  from record length.
- If Cnd bit set when returning, an error occured:
  - Display "Press cassette stop. Then press enter". Beep
  - Call GETKEY
  - Goto EXIT.
- Turn motor off. Wait for 30 VDP interrupts (0.5-0.6 sec).
- Goto EXIT

####  Restore

- Same as Open

####  Load

- If cassette is CS2, return with "Illegal opcode" code in PAB byte 1
- Turn motor on. Wait for 30 VDP interrupts (0.5-0.6 sec).
- Display "Rewind cassette tape. Then press enter". Beep
- Call GETKEY
- Turn motor off. Wait for 30 VDP interrupts (0.5-0.6 sec). Beep
- Display "Press cassette play. Then press enter". Beep
- Wait for the beep to end
- Call the ROM "Read" routine, with GPL opcode I/O 5. Number of bytes
  PAB bytes 6-7.
- If Cnd bit set when returning, an error occured:
  - Turn motor off. Wait for 30 VDP interrupts (0.5-0.6 sec). Beep
  - Check the error bit (value \>01) was set in \>837C:
    - If set, display "Error detected in data". Beep
    - Else display "Error - no data found". Beep
  - Display "Press R to read, C to check, E to exit". Be-be-beep
  - Call GETKEY
  - Re-enter DSR from the beginning
- If no error occured, display "Data OK". Beep
- Goto Close

####  Save

- Turn motor on. Wait for 30 VDP interrupts (0.5-0.6 sec).
- Display "Rewind cassette tape. Then press enter". Beep
- Call GETKEY
- Turn motor off. Wait for 30 VDP interrupts (0.5-0.6 sec). Beep
- Display "Press cassette record. Then press enter". Beep
- Wait for 600 VDP interrupts, i.e 10 seconds (12 sec for european
  consoles).
- Call the ROM routine, with GPL opcode I/O 4. Number of bytes taken
  from PAB bytes 6+7.
- Turn motor off. Wait for 30 VDP interrupts (0.5-0.6 sec).
- Display "Press cassette stop. Then press enter". Beep
- Call GETKEY
- If cassette is CS2, goto EXIT
- Display "Check tape (Y or N)?".Beep
- Wait for \ or \ to be pressed:
  - If \ goto EXIT
  - If \ goto CHECK

####  Delete

- Same as Close

####  Scratch record

- Set error flags for "Illegal opcode" in PAB, byte 1.
- Goto EXIT

####  Status 

- Clear byte 6 in PAB (should be byte 8, but used for bias)
- Goto EXIT
Preliminary 4/1/99

Revision 1 4/3/99 Polished, OK to release
 

[Back to the TI-99/4A Tech Pages](titechpages.md)
