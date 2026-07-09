# Keyboard challenge and tutorial

The following was posted on the online user group
[http://groups.yahoo.com/groups/swpb](http://groups.yahoo.com/group/swpb)
in the form of a challenge for assembly programmers: 10 tasks to
achieve, all dealing with the keyboard.

**Beginner level:  **
1) Scan the keyboard with the KSCAN routine. Exit the program if a key
is pressed.
2) Ditto, but only exit if this key is the spacebar.

**Intermediary level  **
3) Ditto, but do not use KSCAN.
4) Ditto but detect the \ key. Make sure it also works if
\ was held down since the last time it was detected.
5) Detect the \ key (fctn-4) using a routine faster than KSCAN.

**Advanced level  **
6) Assume the console ROM is unavailable. Detect the spacebar by
accessing the keyboard directly.
7) Ditto, but react if any key is pressed, not just the spacebar.
8) Also check the joystick fire buttons
9) Also check the alpha-lock key.

**Expert challenge  **
10) Disable VDP interrupts and peripheral interrupts. Then enter a
forever loop: `HERE JMP HERE` Now exit the loop when the spacebar is
pressed.
When I posted the answers, I interspersed them with a lot of comments
and explanations, which together make a small tutorial on how to access
the keyboard. This tutorial is reproduced below.

If you have some notions of assembly, I encourage you to try your hand
at the above challenge. The tasks are arranged in increasing order of
difficulty. Try and see which ones you can solve without having to look
up the answers.

If you don't know much assembly, read on: I'm introducing several basic
instructions and subroutines. If you feel lost, have a look at my
[assembly primer](assembly.md).

Solutions [1](#A1) [2](#A2) [3](#A3) [4](#A4) [5](#A5) [6](#A6) [7](#A7)
[8](#A8) [9](#A9) [10](#A10)

**Tutorials  **
[The KSCAN routine
](#KSCAN)[Testing bits
](#Bit%20test)[Bit shifts
](#Bit%20shift)[The ANDI instruction
](#ANDI)[The ORI instruction
](#ORI)[Bitwise comparisons
](#Bit%20compare)**[Self-test \#1](#Test1)**

[Branch tables
](#Branch%20tables)[Changing workspace](#Change%20WS)

[What is the CRU?
](#CRU)[CRU addresses
](#CRU%20addresses)[Decoding the keyboard
](#CRU%20keyboard)**[Self-test \#2](#test2)**

------------------------------------------------------------------------

### 1. Scan the keyboard with the KSCAN routine. Exit the program if a key is pressed

One possible answer:

           LI   R0,>0500       Keyboard argument
           MOVB R0,@>8374      Place it where needed
    NOKEY  BLWP @KSCAN         Call keyboard scanning routine
           MOVB @>837C,R0      Get GPL status byte
           SLA  R0,3           Test the Cnd bit (>20)
           JNC  NOKEY          No key pressed or same key held
           B    *R11           Return to caller if a key was pressed
Here are two other ways to test the Cnd bit:

           MOVB @>837C,R0      Get GPL status byte
           ANDI R0,>2000       Keep only Cnd bit
           JEQ  NOKEY          It was 0
    or
           MOVB @>837C,R0      Get GPL status byte
           COC  @H2000,R0      where H2000 is: DATA >2000
           JNE  NOKEY          Cnd bit was not set

### 2. Ditto, but only exit if this key is the spacebar

Building on the above code (only new code is commented)

           LI   R0,>0500
           MOVB R0,@>8374
    NOKEY  BLWP @KSCAN
           MOVB @>837C,R0
           SLA  R0,3
           JNC  NOKEY
           CB   @>8375,@H2000  Ascii code 32? (>20 in hex notation)
           JNE  NOKEY          No: try again
           B    *R11
    *
    H2000  DATA >2000

    or
           MOVB @>8375,R0      Transfer ascii code to a register
           CI   R0,>2000       because CI only works on registers
           JNE  NOKEY          (NB We know R0 2nd byte is zero)

#### The KSCAN subroutine

The KSCAN subroutine calls the keyboard scanning routine in the console
ROMs. It should be entered with a BLWP. But before you call it, you must
place the desired keyboard layout code in byte \>8374. The layout codes
are the same than in Basic, with \>00 meaning "same as before". Note
that KSCAN will place \>00 in \>8374 upon return.

If a new key was pressed, the Cnd bit (value \>20) will be set in the
GPL status byte at \>837C. It will not be set if the same key was held
down since last time KSCAN was called.

Byte \>8375 will contain the ascii code for the key that's down,
according to the keyboard layout you selected. This code is valid
whether the key was freshly pressed or held down. If no key is down,
\>8375 contains \>FF.

KSCAN can also be used to test the joysticks, by specifying keyboard
layout 1 or 2. Byte \>8376 contains the vertical deflection: either \>00
or \>04 or \>FC (which corresponds to -4). Byte \>8377 contains the
horizontal deflection. The fire buttons are reported in \>8375, as if
they were keys.

Note that, if you do not test byte \>837C in the second example, you
will exit the program even if the spacebar was pressed and held since
the last time the keyboard was scanned:

           LI   R0,>0500
           MOVB R0,@>8374
    NOSPAC BLWP @KSCAN
           CB   @>8375,@H2000  Check if ascii code is 32
           JNE  NOSPAC         If not, try again
           B    *R11

####  Testing a bit

There are several ways to test the Cnd bit. All involve making a copy of
the GPL status byte \>837C, so as not to disturb it. In the above
example, I'm using a shift instruction to transfer the bit I want into
the "carry" flag.

Shift instructions moves the bits to the left or to the right, inside a
given register. They are not available for general memory addresses,
only for registers.

Assume R0 contains \>2541, which corresponds to the following bits:

      8421 8421 8421 8421     Bit values
      0010 0101 0100 0001     R0 in binary notation: >2541

Here is how we could rotate it:

    c 8421 8421 8421 8421
    0 0100 1010 1000 0010     SLA R0,1
                        <
    c 8421 8421 8421 8421
    0 1001 0101 0000 0100     SLA R0,2
                       <-
    c 8421 8421 8421 8421
    1 0010 1010 0000 1000     SLA R0,3
                      <--
    etc, upto
    c 8421 8421 8421 8421
    0 1000 0000 0000 0000     SLA R0,15
       <-----------------

As you see, this instruction moves bits leftwards by the specified
number of steps, filling the right with 0 bits.

The LAST bit that was shifted out from the left ends up in the "carry"
bit of the microprocessor status register (marked 'c' above). Which
means that it can be tested with the following instructions:

           JOC  KEY       jump if the bit was 1 (Jump On Carry)
           JNC  NOKEY     jump if the bit was 0 (Jump if No Carry)

####  Bit shifting instructions

There are four shifting instructions: SLA, SRL, SRA and SRC. We just saw
that SLA fills the register with '0' bits on the right, but when
shifting to the right you have the choice to fill the left with zeros of
with copies of the first bit. The latter preserves the sign of the word
(the leftmost bit is the sign bit for arithmetic values).

**SLA** shifts bits to the left, filling the right with zeros (Shift
Left Arithmetic).
**SRL** shifts bits to the right, filling the left with zeros (Shift
Right Logical).
**SRA** shifts bits to the right, filling the left with copies of the
sign bit (Shift Right Arithmetic).
**SRC** shifts bits to the right, feeding them back to the left (Shift
Right Circular).

There is no SLL because the sign bit is on the left and will be shifted
out anyhow. So SLL would be the same as SLA.

There is no SLC, because all possible circular shifts can be achieved
with SRC. For instance SRC R1,15 is the same as SLC R1,1 since registers
have16 bits.

One last trick. What if you were to write:

           SRL R1,0

A shift by 0 positions is obviously meaningless. By convention, the
above means: "get the number of positions from R0". So if R0 contains 5,
this will be the same as `SRL R1,5`

Now what if R0 contains 0? It is understood as "shift by 16".

Finally any value greater than 15 is just rounded up to a number between
0 and 15. For instance `SRC R5,0` with R0 containing 17 is the same as
if R0 contained 1, i.e. the same as `SRC R5,1`

####  The ANDI instruction

There are other ways to test bits:

          ANDI R0,>2000
          JEQ  NOKEY

Here I'm using the ANDI instruction, which performs a bitwise logical
**AND** between R0 and the **I**mmediate argument \>2000 (it's called
immediate, because the value \>2000 immediately follows the instruction
in memory). The result of an AND operation is 1 only if both operands
were 1:

    0 AND 0 = 0
    0 AND 1 = 0
    1 AND 0 = 0
    1 AND 1 = 1

So, if R0 contains \>2541 as above, "ANDing" it with \>2000 will result
in:

      0010 0101 0100 0001     This is R0: >2541
      0010 0000 0000 0000     This is >2000
      -------------------
      0010 0000 0000 0000     This is R0 AND >2000

all other bits were wipped out by the ANDI instruction.

Like many assembly instructions, ANDI automatically compares its result
to zero and sets the corresponding bits in the microprocessor status
register. Which means that we can test for zero with a simple JEQ
(**J**ump if **EQ**ual zero) or JNE (**J**ump if **N**ot **E**qual zero)
without the need for a special comparison instruction.

Note that ANDI can test for more than one bit:

           ANDI R0,>2040
           JEQ  THERE

The jump will only be taken if both bit \>2000 and bit \>0040 are zero
in R0.

####  The ORI instruction

An instruction similar to ANDI is ORI. It performs a bitwise **OR**
between a register and an **I**mmediate value. The result of an OR is 1
if either one of the operands was 1:

    0 OR 0 = 0
    0 OR 1 = 1
    1 OR 0 = 1
    1 OR 1 = 1

    So if R0 contains >2541, ORI R0,>2212 will result in:

      0010 0101 0100 0001     This is R0: >2541
      0010 0010 0001 0010     This is >2212
      -------------------
      0010 0111 0101 0011     This is the result: >2753

Note that this is NOT the same as an addition, because bit \>20 is set
in both operands. An addition would thus cause a carry to bit \>40 and
the result would be \>4753. Logical instructions like AND and OR are
purely bitwise and do not carry to the next bit.

####  Bitwise comparisons

Yet another way to test bits is:

           COC  @H2000,R0
           JEQ  NOKEY

Where H2000 is defined elsewhere in your program as

    H2000  DATA >2000

COC stands for **C**ompare **O**nes **C**orresponding. It tests some
bits in a register, according a mask specified in the first argument.
Note that the second argument (the testee) must be a register, while the
other (the mask) can be either a register or a general memory addres

Here we are saying: "test those bits in R0 that are 1 in H2000. See if
they are all one". If all these bits are '1', the equal flag will be
set, so we can test the result with JEQ or JNE.

           LI   R1,>1204    Our mask
           COC  R1,R0       Assume R0 contains >3695
           JEQ  THERE

     0001 0010 0000 0100    This is R1. It specifies the bits to test.
     0011 0110 1001 0101    This is R0 (>3695)
        ^   ^        ^      Bits tested: all are '1' so the JEQ will be taken.

There is also a CZC instruction: **C**ompare **Z**eros
**C**orresponding. It tests bits just like COC, except that now it
checks whether all the specified bits are zero.

           LI   R1,>4130    Our mask
           CZC  R1,R0       Assume R0 contains >3695
           JEQ  THERE

     0100 0001 0011 0000    This is R1. It now contains >4130
     0011 0110 1001 0101    This is R0 (still >3695)
      ^      ^   ^^         Bits tested: one of them is not '0'
    Cause of this-' bit, the JEQ is not taken.

In the keyboard example, I used a shift instruction because it takes
less space in memory than the other two solutions: one word versus two.
On the other hand, it only tests one bit at a time, whereas ANDI and COC
can be used to test several bits at once. When testing a single bit,
it's a matter of programming style: you are free to use whichever
solution you prefer.

###  Self-test

A few questions for you to make sure you understood the above:

           LI  R1,>0204
           SRC R1,5
    What does R1 contain now?

           LI   R3,>1356
           ANDI R3,>F11F
           ORI  R3,>8041
    What does R3 contain now?

           LI   R1,>0123
           LI   R2,>0111
           COC  R2,R1
           JEQ  YES
    Is the jump taken or not?

           LI   R1,>0123
           LI   R2,>1818
           CZC  R2,R1
           JEQ  YES
    What about here?

I'm not giving you the answers: just assemble the above and see if the
results match your predictions...

------------------------------------------------------------------------

### 3. Ditto, but do not use KSCAN

Solution: call the keyboard scanning routine in the console ROMs
directly. If you disassemble the KSCAN subroutine you will realize that
it's nothing more than a thin wrapper around the keyboard scanning
routine in the console ROMs:

    KSCAN  DATA KEYWR,KSC1             workspace and address for use by BLWP
    *
    KSC1   LWPI >83E0                  use GPL workspace
           MOV  11,@SAVR11             save R11
           BL   @>000E                 call keyboard scanning routine
           MOV  @SAVR11,R11            restore R11
           LWPI KEYWR                  back to our workspace
           RTWP                        return to caller
    *
    KEYWR  BSS  32                     somewhere: 32 bytes for our workspace

So we could include such code directly in our program. A few notes of
caution though.

We MUST change the workspace to \>83E0, the GPL workspace, because of
the way the scanning routine was written.

This is annoying because R11 currently contains the return address for
the GPL opcode XML that called your program. If you intend to return to
the cartridge, you must preserve R11. If you intend to exit with
`BLWP @>0000`, i.e. to reset the TI-99/4A, then you don't care too much
about GPL's R11.

####  Branch tables

If you look up address \>000E you will see that it contains a branch to
the actual location of the keyboard scanning routine, at \>02B2. You may
be tempted to do a `BL @>02B2` directly. Don't. It's always possible
that a different version of the ROMs would have the routine at a
different adddress and your program would crash. On the other hand,
\>000E is guaranteed to be always correct.

This method of implementing branch tables for routines called with BL is
very good programming practice and should be followed each time you
create routines meant to be called by other programs. For routines
called with BLWP it's even simpler: just place the two vectors inside
the table.

    THIS  B    @THIS1          For routines called with BL
    THAT  DATA MYWR,THAT1      For routines called with BLWP
    *
    THIS1 ...
          B    *R11
    *
    THAT1 ...
          RTWP

####  Changing the workspace

So why do we need to change the workspace to \>83E0 before we call
\>000E? Because the TI programmers fell to a very common temptation and
called their registers as addresses. See, suppose you want to transfer
only one byte in R0:

           MOVB R1,R0

This moves the byte into R0's Most Significant Byte (MSB).

But now suppose you want to move it into the least signigifant byte
(LSB) of R0. If you know that your workspace it at \>83E0, you could
write:

          LWPI >83E0           let's use this workspace
          MOVB R1,@>83E1       since >83E1 is the second byte of R0

Similarly, you would use \>83E3 for R1's LSB, \>83E5 for R2's LSB, etc.

But this means that, if you change your workspace, your code will be all
wrong! To some extent you can correct for this by using a label:

    MYWR  EQU  >83E0
    *
          LWPI MYWR
          MOVB R1,@MYWR+1

Then all you have to do to change your workspace is to modify the EQUate
and re-assemble your program.

BUT... the code will only run properly with this workspace. By contrast,
if you do:

           MOVB R1,R0           move into R0's MSB
           SRL  R0,8            transfer to LSB

you achieved the same thing, and the code will run properly with any
workspace!

Ah, but we lost R0's MSB in the process... If you wanted to preserve it,
you could do something like:

           SWPB R0             swap MSB and LSB
           MOVB R1,R0          move into R0'MSB
           SWPB R0             swap again

Now we have modified R0's LSB while leaving the MSB intact. Only, it
took us three instructions instead of one, and three words of memory
instead of two. So our code will be bulkier and slower.

Yet this situation occurs rarely enough so that we can afford this
expense and use only register-addressing so our routines can be called
with any workspace. But then again, it's a matter of taste and
programming style...

###  4. Ditto but detect the \ key

The problem here is that your program was likely launched by pressing
the \ key, so when it starts running, chances are that this key
is still down. So the Cnd bit in \>837C will not be set again, unless
\ is released and pressed again. How could we detect if it was
held down?

We have touched on this earlier. The Cnd bit in \>837C is only set if a
new key is pressed, but the key code in \>8375 is updated even if the
key was held down. So you could skip the test on \>837C and just check
\>8375: if it contains \>FF, no key is being pressed.

#### Extra info

How does this feature work? The keyboard scanning routines saves the
scancode (not the ascii value) for the last key that was pressed in byte
\>83C8 and in \>83C9 or \>83CA for keyboard layout 1 and 2 respectively.
It also saves the keyboard layout code in \>83C6 and the last loop
number in \>83C7. When a key is detected, it is compared with the saved
scancode. The Cnd bit will only be set if the new code is different.

Take home lesson: do not use bytes \>83C6 through \>83CA for your data
if you plan to call the keyboard scanning routine.

###  5. Detect the \ key (fctn-4) using a routine faster than KSCAN

This could be done by calling the little-known routine at \>0020:

           BL  @>0020        test  key
           JEQ CLEAR         pressed
           JNE NOCLR         not pressed

Again, \>0020 only contains a branch to the actual location of the
routine.

For this one, we can use any workspace as the routine does not access
them by address. It only affects R12. If the \ key was pressed,
the routine returns with the Equal flag set in the status register. So
we can use JEQ and JNE to react to it.

------------------------------------------------------------------------

The keyboard is accessed via the CRU. If you know what the CRU is, you
may just jump ahead to the paragraph entitled 'Accessing the keyboard'.
If you know how the keyboard is laid out, you may [jump ahead](#A6) to
the answers section. Otherwise, keep reading...

#### What is the CRU ?

The Communication Register Unit (aka CRU) is used by the microprocessor
to control hardware otherwise than through memory addresses. The CRU can
be viewed as a bundle of 8192 wires: 4096 for input and 4096 for output.
The TMS9900 microprocessor can set any output wire high (to about 5
volts) or low (to about 0 volts). It can check whether any input wire is
high or low.

Of course, there aren't actually that many wires. In fact, there are
only three: one for input, one for output, and one to indicate output
operations. The microcontroller places the number of the 'wire' it wants
on the memory address bus and it's up to the peripheral hardware to
check it. If the peripheral sees the number of a wire it's suppose to
react to, it will input or output data using the two dedicated wires.
But as far as programming is concerned, it's easier to consider the CRU
as a bunch of wires.

To access a 'wire', you place its number in the register R12, for the
microprocessor to put it on the address bus. There is a little problem
here: because the TMS9900 is a 16-bit microprocessor, all addresses are
even and there is no address line corresponding to bit \>0001. Because
of this, the value in R12 should be shifted one position to the left,
i.e. multiplied by two. In other words, the CRU address in R12 is TWICE
the number of the desired wire.

There are five assembly instruction that control the CRU, three affect
only one wire, two affect groups of wires.

           SBO  x      Set Bit to One, sets an output wire high
           SBZ  x      Set Bit to Zero, sets an output wire low

           TB   x      Test Bit, checks an input wire
           JEQ  HIGH   if the wire is high, the bit value is '1' and JEQ is taken
           JNE  LOW    if the line is low, the bit value is '0' and JNE is taken

Where 'x' is the number of the wire, relative to the address in R12. It
can be any number between -128 and +127. This is a very nifty feature
because it means that you can access many wires without having to modify
R12 each time. For instance:

            SBO  0        accesses the wire specified in R12
            SBO  1        accesses the next one,
            SBO  -1       accesses the previous one, etc.

To manipulate several wires at a time, you can use:

           LDCR  source,n   LoaD CRu, ouputs n bits from 'source' to the CRU
           STCR dest,n      STore CRu, inputs n bits from the CRU to 'dest'

Where 'source' and 'dest' can be registers or memory addresses and 'n'
is the number of bits to be transfered.

An LDCR transfer begins with the least significant bit of 'source' to
the wire specified in R12: if the bit is '1' the wire will be set high,
if it's '0' the wire will be set low. LDCR then uses the next bit on the
left to control the next wire, and so on till 'n' bits have been
transfered.

Similarly, STCR checks wires via the CRU, starting with the wire
specified in R12, and sets a bit in 'dest' to '1' if the wire is high
and to '0' if it is low. Again, the transfer starts with the least
significant bit and proceeds toward the left by testing successive
wires.

There is one little subtility here: if n is 8 or less, the transfer will
only affect one byte. If n is more than 8, it affects one word. (If n is
0, it is understood at 16 and affects one word). So, for instance:

           LI   R1,>0902
           LDCR  R1,8    transfers >09  (8 bits, from the right of R1's MSB)
           LDCR  R1,12   transfers >902 (12 bits, from the right of R1)

####  CRU addresses

The first 32 CRU addresses are reserved for the TMS9901 co-processor in
the console. This chip decodes the CRU address and converts it into
enough physical wires to control the keyboard, the cassettes, the
joysticks and the interrupts.

N.B. Due to a design flaw, all CRU addresses from \>0000 to \>0400 are
actually answered by the TMS9901.

Addresses \>1000 and higher are meant for peripheral cards. The
convention is that each card starts at a \>0100 boundary and uses upto
127 bits. For instance: - the disk controller uses \>1100 trough
\>1107 - the RS232 card uses \>1300 through \>13BE - the second RS232
card uses \>1500 through \>15BE - etc.

A further convention is that the first bit in each card (address \>1x00)
turns on the card ROM and makes it appear at \>4000-5FFF. This is used
by several console routines that search for DSRs, CALLs, etc.

Quite often, a peripheral will echo an output line to the corresponding
input line, so you can read back what you wrote. But this is by no means
an absolute rule: some CRU bits are write-only, others are read-only.

If you would like to learn more about the CRU, have a look at [this
page](cru.md).

####  Accessing the keyboard

The keyboard is arranged as a matrix of 8 rows by 6 columns:

        Column: 0   1     2     3     4     5
     R12  |
    >0006 |   =     .     ,     M     N     /
    >0008 | space   L     K     J     H     ;
    >000A | enter   O     I     U     Y     P
    >000C |         9     8     7     6     0
    >000E | fctn    2     3     4     5     1   A-lock
    >0010 | shift   S     D     F     G     A
    >0012 | ctrl    W     E     R     T     Q
    >0014 |         X     C     V     B     Z

Each key controls a switch that connects a row and a column. To check if
a key is pressed, the microprocessor sends electrons into a given column
and checks if they come out of any row. To test the whole keyboard, you
just scan all columns one by one.

The rows are directly connected to the input pins of the TMS9901 and can
threfore be tested as eight independent CRU bits.

The columns, on the other hand, are controlled by a 74LS151 decoder. A
decoder is a chip that converts a number into a bunch of lines. It
brings low one line only, the others being held high:

                             A B C | Y0 Y1 Y2 Y3 Y4 Y5 Y6 Y7
    in +-------+ out         ------+------------------------
    ---|A    Y0|---          0 0 0 | 0  1  1  1  1  1  1  1
    ---|B    Y1|---          0 0 1 | 1  0  1  1  1  1  1  1
    ---|C    Y2|---          0 1 0 | 1  1  0  1  1  1  1  1
       |     Y3|---          0 1 1 | 1  1  1  0  1  1  1  1
       |     Y4|---          1 0 0 | 1  1  1  1  0  1  1  1
       |     Y5|---          1 0 1 | 1  1  1  1  1  0  1  1
       |     Y6|---          1 1 0 | 1  1  1  1  1  1  0  1
       |     Y7|---          1 1 1 | 1  1  1  1  1  1  1  0
       +-------+

This allows the TMS9901 to control the six keyboard columns (plus the
two joysticks) with only three lines. It is possible because only one
column at a time is being tested.

Because of the nature of the decoder that brings columns low, when a key
is pressed the corresponding row wire will be low and the CRU bit will
be '0'. If no key is pressed on that column, all rows will read as '1'
bits.

###  6. Detect the spacebar by accessing the keyboard directly

To check if the keyboard, you place a 3-bit column number at CRU address
\>0024 and read 8 CRU bits at address \>0006. If any of these is low,
the corresponding key was pressed.

           LI   R1,>0000    keyboard column 0
           LI   R12,>0024   CRU address of the decoder
           LDCR R1,3        select the column
           LI   R12,>0006   address of the first row
    LOOP   STCR R2,8        read 8 rows
           ANDI R2,>0200    test spacebar
           JNE  LOOP        not pressed

Remember that bits are transfered from right to left, so '=' is bit
\>0100, 'space' is bit \>0200, 'enter' is bit \>0400, etc.

###  7. Ditto, but react if any key is pressed, not just the spacebar

This just means that we have to test all six columns. It can easily be
done by bumping up the value in R1. We'll also need to test R2 for any
bit set as '0'.

    LP2    LI   R1,>0000    start with keyboard column 0
    LP1    LI   R12,>0024
           LDCR R1,3
           LI   R12,>0006
           STCR R2,8
           CI   R2,>FF00    value if all bits are '1'
           JL   KEY         if a bit is '0' the value is lower than >FF00
           AI   R1,>0100    next column
           CI   R1,>0500    did we do them all?
           JLE  LP1         not yet
           JMP  LP2         no key: restart from column 0

    KEY    B    *R11        a key was pressed

###  8. Also check the joystick fire buttons

Joysticks are controlled by the decoder's output Y6 and Y7, so they can
be accessed as column 6 for joystick 1, and column 7 for joystick 2.
Their return wires are connected to the same lines than the first five
keyboard rows.

       Column 6      7
     R12  |
    >0006 | Fire    Fire
    >0008 | Left    Left
    >000A | Right   Right
    >000C | Down    Down
    >000E | Up      Up
    >0010 |
    >0012 |
    >0014 |

To test the joystick fire buttons, we could insert the following code
after 'not yet':

           JNE  LP1           not yet
           LI   R12,>0024     address of the decoder
           LI   R1,>0600      column 6 for joystick 1
           LDCR R1,>0300      set it
           TB   -15           test bit at address >0006
           JNE  KEY           bit is '0' = fire button pressed
           SBO  0             changes the 6 into a 7
           TB   -15           test fire button for second joystick
           JNE  KEY           pressed
           JMP  LP2

In this example, I tried to show how to use TB to test single bits
without having to change the value in R12. If you remember that R12
contains the wire number times two, you will see that \>0006 (i.e. 6,
wire \#3) lies 15 bits before \>0024 (i.e. 36, wire \#18).

Similarly, SBO 0 is used to modify the column number without having to
change R1 and to LDCR it again (6 is 110 in binary, and 7 is 111, so we
just have to set the least significant bit).

###  9. Also check the alpha-lock key

The alpha-lock key is not controlled by the 74LS151 decoder. It has its
own 'column' wire controled by CRU bit \>002A. Its 'row' wire is the
same as the fifth keyboard row.

So to test that key:

          LI    R12,>002A    address of alpha-lock
          SBZ   0            send electrons to it
          LI    R12,>000E    fifth keyboard row
          TB    0            read it
          SBO   0            finish alpha-lock test
          JNE   KEY          react to TB

Note that SBO does not affect the status register, which is why we can
postpone the JNE test and still have it react to the results of TB. The
alternative would be to have two `SBO 0`, one if the jump is taken, one
if it's not taken.

####  Extra info

The drag with the alpha-lock key is that it's connected to the same row
than the joysticks "up" return line. When the alpha-lock key is down,
and bit \>002A is high (i.e. when not testing the alpha-lock key) it
sends enough current on the fifth row to mess up the joystick return.
This could easily have been avoided, had TI picked the sixth row as a
return line for the alpha-lock key!

###  Self-test

          CLR R12
          SB0 18
          SBZ 19
          SBZ 20
          TB  3
          JNE OK
    What am I testing for, here?

          LI   R12,>1100
          SBO  0
    What did I just do?

NB Don't forget to do SBZ 0 before you exit the latter program!

------------------------------------------------------------------------

### 10. Expert challenge

You wish! This one you'll have to answer by yourself. But here's a
couple of hints:

Yes, it's done via interrupts, although not VDP interrupts.

An almost complete answer is hidden somewhere in the TI-99/4A Tech
Pages...
Revision 1. 8/19/02. Ok to release.
[Back to the TI-99/4A Tech Pages](titechpages.md)
