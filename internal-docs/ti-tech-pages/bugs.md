# Known TI-99/4A bugs

This page describes the few bugs that are known to plague the TI-99/4A
console, the peripheral card DSRs or the cartridges. In short, anything
that we have to live with, because it is engraved in ROM.

**Console**
[RESTORE command](#Restore)
[GPL MOVE opcode](#Move)
[Extended GPL opcodes](#XGPL)
[Timer interrupt \#1](#Timer%20#1), [\#2](#Timer%20#2)
**Disk Controller card**
[DSK.diskname](#DSK.)
[Subprograms \>10 and \>11](#Sub10-11)
**RS232 card**
[Unrecognized interrupts](#Foreign%20ints)
[Timer interrupts](#Timer%20ints)
**Mini-memory cartridge**
[RAM-files with CALL INIT](#call%20init)
**Extended Basic cartridge**
[LINPUT \#0](#LINPUT)
[Recursive calls](#Recursive)
[GOSUB before ELSE](#ELSEGOSUB)
 
 

## Console

#### RESTORE command

**Description**: The RESTORE command in the Basic interpreter (in
association with DATA, not with files) will only work if the program is
placed in the VDP memory. It won't work for a program placed in
GRAM/GROM.

**Cause**: This is because it imports data from the VDP directly instead
of calling the dedicated routine that checks whether the program lies in
VDP RAM or in GROM.

**Fix**: None that I can think of.

#### GPL opcode MOVE

**Description**: The GPL interpreter crashes if a VDP register is used
as a source operand for MOVE.

**Cause**: because VDP registers are write-only, the MOVE opcode is not
supposed to accept them as source operand. However, the routine that
checks for the operand type does not exclude VDP registers. Instead it
branches at a crazy address in the interpreter.

**Fix**: None available. However, a good GPL assembler won't let you
generate such a MOVE statement, so the problem should only occur when
the interpreter went bananas and is reading data as if it were code.
Which means that we are in big trouble anyhow...

#### Extended GPL opcodes

**Description**: GPL opcodes that do not belong to the original set
crash the system.

**Cause**: Provision was made for these opcode to be handled in a
peripheral card, with CRU address \>1B00. When one is encountered, the
GPL interpreter turns this card on and branches to it... without making
sure that the card is effectively here. And of course, such a card was
never released.

**Fix**: The HSGPL card has a CRU address of \>1B00 and Flash-EEPROM
memory in its DSR space. So it would be possible to implement an
extended GPL interpreter to take care of these extra opcodes. At the
very least, a routine that skips the proper number of operand bytes and
returns to the interpreter.

#### Timer interrupts

**Description**: The TMS9901 interrupt-handler comprises an internal
decrementer that can be used to generate timer interrupts. However, the
interrupt service routine (ISR) does not recognizes them and mistakes
them for peripheral card interrupt. It is possible to set a flag to
indicate that we are expecting timer interrupts, but then the ISR
assumes that all interrupts come from the timer. If one actually comes
from the VDP or a peripheral card, it will not be acknowledged and the
computer is locked into an interrupt loop.

**Cause**: Poor design. The ISR is assuming that only the cassette DSRs
will ever make use of the timer interrupts.

**Fix**: Disable VDP and peripheral interrupts before you set the flag
that enables timer interrupts. (Poor man's fix).

#### Timer interrupts \#2

**Description**: When a timer interrupt occurs, the ISR gives you two
options:

1.  Return to the main program, skipping one word (used by the cassette
    routines to jump out of a JMP \$ waiting loop).
2.  Branch at an address of your choice, loosing the return point!

**Cause**: Poor design again. This part of the ISR was clearly written
with only the cassette DSRs in mind. The return address is lost because
branching is achieved by placing the desired address in R14 (thereby
overwriting the return point) and performing a RTWP.

**Fix**: Enable interrupts at only one place in your program, with a
LIMI 2 LIMI 0 pair of statements. This way you know where to come back:
at the LIMI 2 statement.

##  Disk controller

#### DSK.diskname DSR

**Description**: If the DSK.diskname DSR does not find the required disk
it retuns with an error, instead of letting another card have a go to
it. Which means that we cannot have another controller card making use
of DSK.diskname.

**Cause**: Poor design. It would have been a trivial matter to just
return as if the DSR was not found. I guess the designers wanted a "File
error" rather than a "Device error".

**Fix**: If you do have another card that handles DSK.disname properly
(such as the Horizon Ramdisk), install it at CRU address \>1000 so that
it is scanned before the disk controller (whose CRU address is
\>1100).
Or: write your own DSRLNK routine, and make sure it checks CRU \>1100
last. This won't work with Basic though, because it uses the GPL DSRLNK
routine that's in the console ROM.
 

#### Subprograms \>10 and  \>11
**Description**: Just as above. These subprograms expect the drive
number to be passed in byte \>834C. If this number does not correspond
to one of its drives (i.e. \>01 to \>03) the TI disk controller returns
a file error.

**Cause**: Poor design again. TI never envisionnned that there could be
other controllers around, that may support additional drive numbers.

**Fix**: If you have another card that handles subprograms \>10 (sector
read/write) and \>11 (format disk), make sure it is installed at CRU
\>1000.
Or, write your own DSRLNK routine, that scans the TI controller last.

##  RS232 card

#### Unrecognized interrupts

**Description**: The RS232 card contains two TMS9902 serial controllers,
each of which can be set to generate interrupts upon various conditions.
The card DSRs only make use of the reception interrupt, to signal that a
new byte was received. However, if you enable interrupts for only one of
these chips, the RS232 card will automatically enable the second as soon
as it sees an interrupt coming from another card.

**Cause**: When it determines that an interrupt did not come from
itself, the RS232 card resets the reception interrupt on both chips.
However, it does not remember on which chip interrupts were enabled and
enables either both or none.

**Fix**: Have another card whatch for such situations and restore the
proper interrupt conditions in the TMS9902. Tricky. And requires a card
with RAM in the DSR space and a CRU address higher than that of the
RS232 card (so it is called after it).

#### Timer interrupts

**Description**: Only reception interrupts are handled by the RS232 card
(albeit quite poorly as we just saw). If another type of interrupt
occurs, the TMS9902 chip is reset, thereby disabling interrupts. This is
a pain because the TMS9902 contains a nice timer that we could have
enjoyed.

**Cause**: Poor design.

**Fix**: Trap this kind of interrupts within another card (such as the
Horizon Ramdisk) and prevent the RS232 card from seeing them. Easier
said than done...

##  Mini-memory cartridge

#### RAM-files and CALL INIT

! Top winner bug !

**Description**: The two RAM-files EXPMEM1 and EXPMEM2 become basically
unavailable once a CALL INIT is performed, or if the Mini-memory is
initialized.

**Cause**: Provision was made that EXPMEM1 and EXPMEM2 would not
conflict with assembly language loaded by CALL LOAD. This was done with
two 10-lines routines, one called by OPEN and SAVE, the other by DELETE.
Well, this was obviously a last-minute hack because the sloppy
programmers managed to leave two deadly bugs within the first routine.

Bug \#1 causes the file type to be saved as Dis/Fix no matter what it
actually was. Which means that any file you create cannot be read back
unless it is Dis/Fix (or program, because SAVE does not check the file
type).

Bug \#2 increments the wrong pointer! Which means that any attempt to
create an EXPMEM1 file prevents CALL LOAD from loading into the high
memory expansion (instead of the low memory expansion). Furthermore, the
file we just created cannot be accessed anymore because its
characteristics are not placed at the right location (\>2004 instead of
\>2000).

**Fix**: Temporarily "revert" the CALL INIT before opening a file with a
CALL LOAD to address \>7000. We'll also need to patch the loader's
pointers at \>7026 (if we are using EXPMEM1) or \>7022 (for EXPMEM2)
since this won't be done automatically. Finally, we need to restore the
content of \>7000 before any CALL LINK, otherwise an error will be
issued:0

    100 CALL LOAD(28672,0)            ! Clear initialization flag
    110 OPEN #1:"EXPMEM1",whatever  
    120 CALL LOAD(28672,165)          ! Restore it (could wait until next CALL LINK)
    130 CALL LOAD(28710,0,0,0,0)      ! Prevent CALL LOAD(filename) into the low memory expansion

For EXPMEM2. use address 28706 to prevent CALL LOAD from loading
assembly programs into the high memory expansion.

For more details, see my [Mini-memory page](mm_mod.htm#Bug).

##  Extended Basic

#### LINPUT \#0

**Description**: LINPUT \#X where X=0 gives a "Syntax Error" instead of
taking input from the keyboard.

*(Contributed by Ben Yates)*

**Cause**: ?

**Fix**: None that I know of.

#### Recursive subprogram calls

**Description**: If a subprogram is allowed to call its caller, you may
end up in a forever recursive loop. For example:

    100 CALL A 
    110 SUB A 
    120 CALL B 
    130 SUBEND 
    140 SUB B 
    150 CALL A 
    160 SUBEND

With the black Extended Basic version 110 cartridge, you get:

    * MEMORY FULL IN 110 
    CALLED FROM A 
    CALLED FROM B 
    CALLED FROM A . .

The computer keeps displaying these "`CALLED FROM`" for a coons life.

By contrast, a beige style, newer 110 Extended BASIC detects nested
calls and gives the following error message:

    * RECURSIVE SUBPROGRAM CALL

*(Contributed by Casey Vanlandingham)*

**Cause**: Actually, this is not a bug. Recursivity can be a very nice
feature. It's just up to you to include the proper controls in your
subprograms so that you don't get locked in a forever loop. The only
buggy feature is that the error handling routine should not display all
these `CALLED FROM` messages.

**Fix**: Only some of the old black modules do have this problem. It was
"fixed" in later versions...which actually was a problem for people who
were making proper use of the recursivity feature!

" There is an interesting fact related to that bug: Do you know the
software-house APESOFT ? (was from Austria, a family business from
family Peschetz). They wrote and sold lots of graphic programs for XB,
but they had this special 'recursive' Basic and USED THAT
FEATURE!!!!!!!
...so most of their users got the above error code....and they had to
modify their programs. "

*(Note by Michael Becker)*

Oh, so that's what the "APE" in APESOFT stands for: Austria, PEschetz.

####  GOSUB before ELSE

**Description**: If you place a GOSUB before an ELSE in a coumpound
statement, and the ELSE is not taken, execution skips the next statment.
Here is an example:

    60 D=0 
    70 IF D=0 THEN GOSUB 90 ELSE Y=0 :: P=3 
    80 GOTO 100 
    90 P=1 :: RETURN 
    100 PRINT "P = "&STR$(P)

This program should print "P=1" right? At least it does so if you place
aything else than a GOSUB before the ELSE. But with a GOSUB it actually
prints P=3.

*(Contributed by Carsten, Denmark)*

Cause: This "bug" is probably due to the fact that GOSUB returns to the
next statement, not to the next line number. So the compound statment
Y=0 :: P=3 (which is normally part of the ELSE branch, since IF does
jump to the next line number) is split in two, and GOSUB returns at P=3.

Fix: ?

Note: The error is replicated in the following XBASIC cartridges:

TI Extended BASIC version 110
XB Vn 2.5 by Tony Knerr
Super Extended BASIC
Mechatronic Extended BASIC
Rich XB Vn 2001

*(Note by Bob Carmany)*

Revision 1. 7/17/01

Revision 2. 7/7/03
 

 
 

[Back to the TI-99/4A Tech Pages](titechpages.md)
