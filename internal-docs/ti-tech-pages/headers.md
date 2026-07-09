# Standard headers

[Introduction
](#header%20intro)[Structure summary](#header%20summary)

**Power-up routines  **
[Introduction
](#power-up)[Calling
](#call%20power-up)[Returning](#return%20power-up)

**Programs  **
[Introduction
](#Programs)[Calling
](#Call%20programs)[Exiting](#Exit%20program)

**Subprograms  **
[Introduction
](#Subprograms)[Calling
](#call%20subprogram)[Return
](#Return%20subprogram)[Parameter passing
](#Subprogram%20parameters)[Returning
values](#Subprograms%20return%20values)

**DSRs  **
[Introduction
](#DSRs)[Calling
](#DSR%20call)[Returning
](#DSR%20return)[Parameter passing: PABs](#PAB)

**[ISRs](#ISR)**

**[Foreign language translators](#Foreign%20language%20translators)**

**Programming examples  **
[XML \>19](xml191a.txt): the card ROM scanner
[XML \>1A](xml191a.txt): the GROM scanner
[Calling XMLs](#crude%20way): the crude way
[Calling GPL and XML](#GPLLNK): the smooth way
[String allocation routine
](#string%20alloc)[Passing parameters from Basic
](#passing%20params)[Returning values to Basic](#returning%20values)

##  Introduction

Standard headers can be located either in peripheral cards ROM, in GROMs
or in cartridge ROM. In peripheral cards, the header always appear at
address \>4000. In cartridge ROM it maps at \>6000 and can only contain
programs. In GROMs there can be a header at the following addresses:
\>0000, \>2000, \>4000, \>6000, \>8000, \>A000, \>C000, and \>E000 in
GROM memory. If a header is present at any of these addresses, the first
byte must be \>AA to to tell the scanning routines there is a header
here.

Headers contains lists of programs, subprograms, power-up routines, DSRs
and ISRs. Lists are organised as chains of linked items: each item
begins with a word pointing to the next item in the list. The word is
\>0000 for the last item in the list.

**Power-up** routines are executed when you reset the TI-99/4A, before
the user can press a key to leave the main title screen. They can be
found in GROMs or peripheral card ROMs, but not in cartridge ROM.

**Programs** are what appear on the main menu, after the user left the
title screen: "Press 1 for TI Basic", etc. Programs can be located in
GROMs or in cartridge ROMs, but not in peripheral cards. At some point
in time, Texas Instruments modified the system so that programs in ROM
won't appear on the main menu: according to which version you have, your
console may or may not pick them up.

**Subprograms** are routines that provide a service to the user. For
instance, all Basic CALLs are subprograms. But subprograms can also be
called from assembly languages. They can be found in GROMs or peripheral
card ROMs, but not in cartridge ROM.

**DSRs** (Device Service Routines) are special subprograms that deal
with files. In Basic, they are called by file operations, and can also
be called from assembly. They can be found in GROMs or peripheral card
ROMs, but not in cartridge ROM.

**ISRs** (Interrupt Service Routines) are called when an interrupt was
received that did not come from the video-processor. They can only be
located in peripheral card ROMs.

Finally, the GROM header at \>6000 in base \>9800 can have two **foreign
language translators**, used to translate the title screen and the main
menu, respectively.

In summary:
Peripheral cards ROMs cannot contain programs, nor translators.
GROMs cannot contain ISRs.
Cartridge ROMs can only contain programs.

###  Standard header summary

Content

&gt;AA indicates a standard header

Version number

Number of programs (optional)

Not used

Pointer to power-up list (&gt;0000 if
none)
Pointer to program list (&gt;0000 if
none)
Pointer the DSR list (&gt;0000 if none)
Pointer to subprogram list (&gt;0000 if
none)

Program, subprogram and DSR lists:

    Link to next item --+
    Address             |
    Name length         |
    Name                |
    +-------------------+
    |
    V
    Link to next item: >0000
    Address
    Name length
    Name

Power-up and ISR lists:

    Link to next item --+
    Address             |
    +-------------------+
    |
    V
    Link to next item: >0000
    Address

#### Exemple

&gt;4001: &gt;01          Version 1
&gt;4002: &gt;00          No programs allowed in peripheral card ROMs
&gt;4003: &gt;00
&gt;4004: &gt;4010        Ptr to power-up list
&gt;4006: &gt;0000        Ptr to program list (none)
&gt;4008: &gt;4018        Ptr to subprogram list
&gt;400A: &gt;4030        Ptr to DSR list
&gt;400C: &gt;4040        Ptr to ISR list`
&gt;4010: &gt;0000        Link to next power-up (no more)
&gt;4012: &gt;4100        Address of power-up routine`
&gt;4018: &gt;4022        Link to next subprogram
&gt;401A: &gt;4200        Address of this subprogram
&gt;401C: &gt;04          Name length
&gt;401D: &quot;SUB1&quot;       Subprogram name
&gt;4021: &gt;00          Align on even boundary
&gt;4022: &gt;0000        No more subprograms
&gt;4024: &gt;4220        Address of this subprogram
&gt;4026: &gt;01          Name length
&gt;4027: &gt;B3          Subprogram name`
&gt;4030: &gt;403A        Link to next DSR
&gt;4032: &gt;4300        Address of this DSR
&gt;4034: &gt;05          Name length
&gt;4035: &quot;MYDSR&quot;      DSR name
&gt;403A: &gt;4044        Link to next DSR
&gt;403C: &gt;4300        Address of this DSR (same as above)
&gt;403E: &gt;05          Name length
&gt;403F: &quot;DUMMY&quot;      DSR name
&gt;4044: &gt;4022        Trick: subprogram &gt;B3 can also be accessed as a DSR
&gt;4046: &gt;4440        Address of this DSR
&gt;4048: &gt;01          Name length
&gt;4049: &gt;04          DSR name`
&gt;4050: &gt;0000        Link to next ISR (no more)
&gt;4012: &gt;4800        Address of ISR routine`
&gt;4100: ...          Power-up routine
       B   *R11     Must return like that`
&gt;4200: ...          SUB1 subprogram
       INCT R11     Found and executed
       B    *R11`
&gt;4220: ...           &gt;B3 subprogram / DSR
       INCT R11
       B    *R11`
&gt;4300: ...           MYDSR and DUMMY DSRs
       INCT R11
       B    *R11 `
&gt;4440: ...          &gt;04 DSR
       INCT R11
       B    *R11`
&gt;4800  ...          ISR
       TB   2       Check if interrupt came from this card (e.g using CRU bit 2)
       JEQ  NOTME   Nope: return immediately
       ...          Yep: process interrupt
NOTME  B    *R11    Return

##  Power-up routines

As the name indicates, a power-up routine is executed at power-up time,
i.e. just after the TI copyright screen has been displayed, before
checking if the user has pressed a key. These routines are meant to
initialize the card: the one on the interface card sets the PIO port as
output, the one on the disk controller reserves room in the VDP memory,
etc. They could also be used to take over control of the TI and prevent
the user to go to the main menu. Some third party disk controllers have
this annoying habit.

The main power-up program is located in the console GROMs, at address
\>0020. It performs the following actions:

- Empty the sound list and stop all sound generators.
- Load default values (found in GROM) into VDP registers.
- Clear scratch-pad addresses \>8300-8371 and \>8382-83CA.
- Reset CRU bits 3 to 10 in the TMS9901 (no interrupts for keyboard
  input lines), as well as bit 16 (unconnected output) and 24 (audio
  gate off). Set CRU bits 1 and 2 (VDP interrupts and external
  interrupts allowed) and bits 22 and 23 (cassette motors).
- Generate a beep.
- Determine the size of the VDP memory (4K, 8K or 16K). Set the 16K flag
  accordingly in VDP register 1 and byte \>83FD.
- Clears VDP addresses 0 to \>1000. Loads a color table at \>0380-039F,
  the standard character set (chars 32-96) at \>0900-0AFF, and the TI
  logo in characters 1 to 11 at \>0808-0857.
- Clear the screen.
- Scan the keyboard in all possible modes (5 down to 1). Ignore the
  results.
- Build the title screen.
- Search for power-up routines and calls each and everyone of them.

###  Calling power-up routines

The power-up searching routine uses XML \>19 to scan ROMs and XML \>1A
to scan GROMs. It branches to each power up routine it finds, whether in
assembly or in GPL. Routines are called in the following order:

- Power up routines in peripheral card ROMs are executed first, in the
  order of their CRU base, from \>1000 to \>1F00. If there is more than
  one routine per card, they are called in the order of their appearance
  in the routine list.
- Then routines in GROMs installed at base \>9800 are called, starting
  with GROM \>0000 (none) and walking byte steps of \>2000 to GROM
  \>E000. If there is more than one routine in a GROM, they are called
  in the reverse order of their appearance in the list.
- Finally, if GROM \>6000 at base \>9800 has a negative version number
  (bit \>80 set in byte \>6001), the GPL routine at \>6010 is called.
  This is meant to install a foreign language translator for the title
  screen ("Texas Instruments Home computer. Ready - Press any key to
  begin"). Some cartridges use that trick to auto-start without waiting
  for the user to press a key and enter the main menu. Others just use a
  power-up routine for that purpose.

###  Returning from a power-up routine

Assembly language power-up routines should return with B \*R11 in
assembly.

It is also possible to return with INCT R11, B \*R11 but this will
prevent further scanning of peripheral cards for power-up routines. It
will not however prevent GPL power-up routines from being called.

Things are a little more complicated in GPL as the main program does not
branch to power-up routines directly. Instead the addresses of all
power-up routine located are saved on the data stack (whose base is not
at its usual location, but at \>8300, to allow for more storage space).
To branch to the next power-up routine on the list, one must therefore
transfer its address to the subroutine stack:

       DECT  @&gt;8372               Update pointer
       RTN                        This will branch to the next routine

If you wanted to shortcut the process and prevent the main program from
calling other power-up routines, you should keep searching the data
stack until you find an address that is in GROM 0 (i.e. with an address
smaller than \>1800). This will be the address at which the main routine
continues (normally \>019F, but it may change according to the GROM
version), just branch to it as described above.

L1     DCH   &gt;1800,*&gt;8372        Is address in GROM 0?
       BS    DONE                Yes: branch to it
       DECT  *&gt;8372              Point to next address
       BR    L1                  and try again

##   Programs

A program is what appears on the main menu, after you left the TI
copyright screen: "Press 1 for TI-Basic, 2 for Whatever".

There is no limit to the length of a program name, except that it should
fit on one screen line, if you want the main menu to look nice. This
being said, nothing prevents you to call your program
"`PROTECTING YOUR WORLD FROM THE ATTACK OF THE GOULISH DEMONS OF THE LEGIONS OF CHAOS`"
(except good taste, may be...). However, if another program is found
after yours, its name will overwrite the third line of your program
name.

Note that at this point only the standard character set has been loaded
in VDP memory (unless you loaded another one with a power-up routine).
Therefore you can only use characters 32 to 96 in your program name,
i.e. no lower case.

###  Calling a program

The main power-up program lists the programs it finds in the following
order:

- Program in catridge ROMs (at \>6000 in CPU memory) are listed first.
  If there is more than one program in the ROM header, they are listed
  in the reverse order of their appearance in the list. Note: at some
  point Texas Instruments disabled this feature to prevent third parties
  from selling pure ROM cartridges (and how mean is that?). Depending on
  the GROM version in your console, it may or may not be present:
  version 2.2 draws the line.
- Programs in GROMs at base \>9800 are then listed in the order of the
  GROMs: \>0000 (none), \>2000 (`TI BASIC`), \>4000 (no header), \>6000,
  etc upto \>E000. If there is more than one program in a header, they
  are listed in the reverse order of their appearance in the program
  list.
- If the GROM at \>6000 in the current base has a valid header with a
  negative version number (bit \>80 set in \>6001), the program
  immediately branches at GROM address \>6013. This is meant to
  implement a foreign language translation routine that can translate
  the menu that was just built (most probably, it will just change
  "PRESS" and "FOR" as translating program names would be quite tricky).
  Some cartridges use that trick to auto-start without letting the user
  select a program from the main menu.
- The main program compares the content of GROM addresses \>6000-601D
  using base \>9800 and base \>9804. If a difference is detected, it
  adds a program called "REVIEW MODULE LIBRARY" at the end of the list.
  In fact, this is not a real program: it just re-enters the menu
  building routine. If the user selects this option, it will list the
  ROM programs again, then the GROM programs but this time using base
  \>9804. The next time it will use base \>9808, etc upto \>983C. If a
  base does not yield valid headers, it is just skipped. Note that
  theoretically there could be GROM ports that map higher than that
  (upto \>9BFC), but they won't be scanned for programs, DSR, or
  subprograms.
- If no program at all was detected, you'll see a message saying "Insert
  cartridge". In this case, any key you may press is considered as
  invalid: you must insert a cartridge, which will reset the TI-99/4A.
  Note that this situation should normally never occur since the
  "`TI BASIC`" program is always present in the console GROMs (that map
  under any base).
- If there are more than 9 programs, the names of program 10 and above
  will not appear on screen. Instead they'll be written in the color
  table and pattern table, which is likely to mess up the display...
  However, the programs can still be selected using the keys that follow
  "9" in the ASCII list (: ; \< = \> ? @ A B C etc).

Once the user has selected a program, the system performs the following
actions:

- Beep.
- Clear the screen.
- Clear the VDP from address \>1000 to the address found in word
  \>8370-8371 (highest free address in VDP mem).
- Clear the scratch-pad addresses \>8300-836F, \>8374, and \>8382-83C0.
- Load the same color in the whole color table (\>17: black on cyan).
- Branch to the program using `XML >F0` (vector in \>8300) for assembly
  programs in cartridge ROMs and a dummy `RTN `instruction for GPL
  programs in GROM.

###  Program termination

A program is not supposed to return to the caller: upon completion it
typically resets the TI-99/4A. This is achieved by `BLWP @>0000` in
assembly and by `EXIT `in GPL.

##   Subprograms

A subprogram is what you call with the Basic instruction CALL. They can
also be called from assembly or any other language. Theoretically there
is not limit to the length of a subprogram name, but TI-Basic and the
console GROM DSRLNK routine (G@\>0010) set a limit of 7 characters
(probably because they save the name into the scratch-pad area at
\>834A-8351).

###  Calling a subprogram

The calling convention is that the subprogram name (with a leading
lenght byte) should be placed in VDP memory, with word \>8356-57
pointing at the length byte. Then DSRLNK can be called from GPL with
`CALL G@>0010`.

This routine first scans the peripheral card ROMs in the order if their
CRU addresses, by calling XML \>19.

It then repeatedly calls XML \>1A to scan the GROMs at base \>9800,
starting with GROM \>0000, then \>2000, etc upto \>E000.

It then repeats the process with bases \>9804, \>9808, upto \>983C.

If you're curious, here is a [disassembly](xml191a.txt) of XML \>19 and
XML \>1A

Unfortunately XML \>19 and XML \>1A cannot be called from assembly since
they return to the GPL interpreter. Therefore most assembly language
package provide a custom DSRLNK routine that will search the card ROMs.
The editor/assembler cartridge also provides an XMLLNK routine and a
GPLLNK routine that will allow the programmer to call G@\>0010. But
that's because it's a GPL cartridge and it contains an XML instruction
to return to the assembly caller.

There are ways around this drawback though.
Here is a fairly crude one:

* so that it can call XML &gt;19
* It expects: the sub name in &gt;834A-&gt;8351, name lenght in &gt;8354-55
* &gt;836D should contain &gt;0A for subprograms, &gt;08 for DSRs
* It returns by skipping a word if the sub/DSR was found.`
TXML19 MOV  @&gt;83FA,R9      Save GROM base
       LI   R0,&gt;0041       Points to a byte containg &gt;0A
       MOV  R0,@&gt;83FA      Screw up GROM base`
       MOV  @&gt;83C4,R12     Save interrupts hook
       LI   R0,RETPT       Our return point
       MOV  R0,@&gt;83C4      Set it as ISR hook`
       MOVB  @&gt;8373,R7      Save sub stack ptr`
       CLR  @&gt;83D0         To search all cards
*      LI   R0,&gt;1700       (Alternatively: start from that card)
*      MOV  R0,@&gt;83D0
*      LI   R0,&gt;xxxx       (and that link in sub/DSR list)
*      MOV  R0,@&gt;83D2      (should point at &quot;next link&quot; word of first sub) `
       LWPI &gt;83E0          GPL workspace
       B    @&gt;0AC0         Call XML &gt;19
*      MOV  @&gt;0CFC,R1      (Safer ?) Get address of XML table 1
*      MOV  @&gt;0012(R1),R1  Get vector for XML &gt;19
*      B    *R1            Call it`
RETPT  LWPI WREGS          Back to our workspace
       MOV  R9,@&gt;83FA      Restore GROM base
       MOV  R12,@&gt;83C4     Restore ISR hook
       CB   @&gt;8373,R7      XML &gt;19 does DECT @&gt;8373 if sub was found
       JEQ  NOTFND         Not found
       MOVB R7,@&gt;8373      Restore initial value
       INCT R11            Flag: we found it
NOTFND B    *R11           Return to caller

What this routine does is to change the GROM base used by the GPL
interpreter to fetch the next opcode. It changes it to an address in ROM
that contain byte \>0A, which corresponds to the GPL instruction GT (it
could also be \>09, \>0C or \>0D. Just make sure the byte address is
lower than \>9800).

Then it installs an ISR hook and calls XML \>19 which will scan card
ROMs for the subprogram or the DSR we want to call. Whether it was found
or not, XML \>19 returns to the GPL interpreter. Since the "GROM base"
now points at a \>0A byte, the interpreter will endlessly execute a GT
instructions. I chose GT because it's pretty harmless and 1-byte long.

As the interpreter enables interrupts between each instruction, our hook
is prone to be branched at sooner or later. We then perform some clean
up to restore the interpreter in its normal state. Finally we check
whether the subprogram (or the DSR) was found.
Now, here is a more subtle way. It implements a
GPLLNK routine that allow an assembly language program to call a GPL
routine. It makes use of carefully selected bytes in the console GROMs
to simulate an XML instruction and regain control once the GPL routine
returns.

DUMXML DATA &gt;36B8           This address may vary: check your GROMs
ADRXML DATA &gt;3FF6           and adjust this value accordingly
BYTE00 DATA &gt;0003           Address of a &gt;00 byte in GROM (should be OK)`
* This routine calls a GPL subroutine that returns with RTN or RTNC
* It expects the subroutine address in a data word following the call
GPLLNK DATA WREGS,GPLNK1   `
GPLNK1 LI   R1,&gt;8373        Subroutine stack ptr
       MOVB *R1,@WREGS+3    Put it in R1: now pointing at top of stack
       INCT R1              Next free spot
       MOV  @DUMXML,*R1+    Push &quot;return&quot; address (dummy XML in GROMs)
       MOV  *R14+,*R1       Push desired address
       JMP  SK1             Common part with GPLG10`
* This routine emulates a CALL G@&gt;0010.
* It is used to call a GPL subprogram/DSR that returns with CALL G@&gt;0012
* It expects the GROM base in a first data word (if 0, it gets it from &gt;83FA)
* and a GROM address in a second data word
GPLG10 DATA WREGS,GPLNK2    `
GPLNK2 LI   R1,&gt;8373        Subroutine stack ptr
       MOVB *R1,@WREGS+3    Put it in R1: now pointing at top of stack
       INCT R1              Next free spot
       MOV  @DUMXML,*R1+    Push &quot;return&quot; address (dummy XML in GROMs)
       MOV  *R14+,*R1       Push GROM base
       JNE  SK4             Is it &gt;0000?
       MOV  @&gt;83FA,*R1      Yes: use current base (from GPL R9)
SK4    C    *R1+,*R1+       Skip a word
       MOV  *R14+,*R1       Then push address: this emulates G@&gt;0010
SK1    MOVB @WREGS+3,@&gt;8373 Update stack ptr
       MOV  @ADRXML,R1      Address of the dummy XML vector
       MOV  *R1,R2          Save its content
       LI   R11,GPLRT       Our real return point
       MOV  R11,*R1         Place it in the XML table
       LWPI &gt;83E0           Switch to GPL workspace
       S    R9,R9           Opcode &gt;00 (i.e. RTN)
       B    @&gt;0016          To GPL interpreter with opcode in R9, msb`
* This routine calls an XML that returns to the GPL interpreter
* It expects the XML number is a data word following the call
* Alternatively, that word may contain the address to branch at
GPLXML DATA WREGS,XMLNK2   Calls an XML that returns to the GPL interpreter `
XMLNK2 LI   R1,&gt;8373        Subroutine stack ptr
       MOVB *R1,@WREGS+3    Put it in R1: now pointing at top of stack
       INCT R1              Next free spot
       MOV  @DUMXML,*R1     Push &quot;return&quot; address (dummy XML in GROMs)
       MOVB @WREGS+3,@&gt;8373 Update stack ptr
       MOV  @&gt;83FA,R2       Get current GROM base
       AI   R2,&gt;0402        Make it GROM set address port
       MOVB @BYTE00,*R2     Address of a &gt;00 byte in GROM
       MOVB @BYTE00+1,*R2   Now, next instruction will be a RTN
       MOV  @ADRXML,R1      Address of the dummy XML vector
       MOV  *R1,R2          Save its content
       LI   R11,GPLRT       Our real return point
       MOV  R11,*R1         Place it in the XML table
       JMP  SK2             Common part with XMLLNK
* This routine calls an XML that returns to the caller
* It expects the XML number is a data word following the call
* Alternatively, that word may contain the address to branch at
XMLLNK DATA WREGS,XMLNK1`
XMLNK1 CLR  R1              No need to restore anything
SK2    MOV  *R14+,@&gt;83E4    Put data word in GPL R2
       LWPI &gt;83E0           Switch to GPL workspace
       MOVB R2,R2           Is first byte of data &gt;00
       JNE  SK3             No: direct branch to an address
       MOV  R2,R1           Yes: get address from XML tables
       SRL  R1,4
       SLA  R1,1            Isolate table number
       SLA  R2,12
       SRL  R2,11           Isolate routine number
       A    @&gt;0CFA(R1),R2   Address in table
       MOV  *R2,R2          Get vector
SK3    BL   *R2             Call XML
GPLRT  LWPI WREGS           GPL will return here: switch to our workspace
       MOV  R2,*R1          Restore the data we modified
       RTWP                 And that&#39;s it`
* Calling examples
      BLWP @GPLLNK
      DATA &gt;002A           Call the LOG subroutine`
      BLWP @GPLG10
      DATA &gt;0000,&gt;37BF     Call SCREEN`
      BLWP @XMLLNK
      DATA &gt;0012           Convert floting point to integer`
      BLWP @GPLXML
      DATA &gt;0019           Scan cards for a subprogram (returns to GPL)`
      BLWP @GPLXML
      DATA &gt;0AC2           Alternate entry point into XML &gt;19 (skip CLR R1)

GPLLNK first places a dummy return address on the subroutine stack. This
is the address of a \>0F byte in the console GROMs. It will be
interpreted as `XML `by the GPL interpreter, and the next byte will be
used to find the address where to branch. This may vary according to the
GROM version, that's why I put these values in data words. In my console
GROM address \>36B8 contains \>0F4B which corresponds to XML \>4B and
will get the vector from address \>3FF6 (the XML table 4 starts at
\>3FE0). GPLLNK replaces this word with its return point.

It also places the desired GPL address on the subroutine stack and
finally branches to the GPL interpreter using the special entry point
\>0016 which expects R9 msb to contain an opcode. In our case, this
opcode will be \>00, which codes for a `RTN `instruction. The GPL
interpreter will therefore "return" to the address we just placed on the
stack, which is the GPL routine we want to call. (I used `S R9,R9`
instead of `CLR R9` because entry point \>0016 branches to a `JLT` that
must not be taken for RTN).

If this routine returns with `RTN`, the GPL interpreter will just branch
to the pseudo `XML >4B` instruction in GROM, and this will return
control to our GPLLNK routine.

An alternate subroutine, GPLG10 is provided to emulate a call to
G@\>0010, which the the GPL equivalent of DSRLNK. It saves the GROM base
on the stack together with the return address. Subprograms called in
this way return with `CALL G@>0012` which fetches the base and the
address from the stack (all TI-Basic subprograms and DSR do so).

XMLLNK emulates an XML instruction and calls an assembly language
subroutine. It is assumed that this routine will return to the caller.
Unfortunately many XMLs in the console ROM don't bother to save the
return address and just branch to the GPL interpreter.

GPLXML it therefore provided to call an XML that returns to the GPL
intepreter, and nevertheless regain control afterwards. As it modifies
the current GROM/GRAM address, you may want to save this address first,
and restore it afterwards.
And now that we can call GPL routines, let's scan for subprograms the
way TI-Basic does: by calling DSRLNK at \>0010 in the console GROM.
There is only one problem: DSRLNK expects a byte of data after the call:
\>08 for DSR and \>0A for subroutines. Since we are not calling it with
a `CALL `from GPL, the `FETC @>836D` instruction will return a dummy
value. Therefore, OURLNK enters DSRLNK by skipping the
`FETC `instruction, with the proper value already placed in \>836D.
Unfortunately address \>0010 only contains a branch instruction to the
real start point of the routine. We'll have to get the branch first, to
be able to change the entry point.

* all card ROMs and all GROMs for the subroutine/DSR we want
* It expects the name of the sub/DSR in VDP memory (with leading lenght byte)
* Word &gt;8356 should point at the length byte
* Byte &gt;836D should contain &gt;08 for DSRs and &gt;0A for subroutines
OURLNK LI   R9,&gt;9C02      GROM set address port
       LI   R0,&gt;0010      Address of the GROM DSRLNK routine
       MOVB R0,*R9        Set this address
       SWPB R0
       MOVB R0,*R9
       AI   R9,&gt;FBFE      Now it becomes the GROM base
       MOVB *R9,R0        Get first byte of address
       SWPB R0
       MOVB *R9,R0        Get second byte
       SWPB R0
       ANDI R0,&gt;1FFF      Remove the BR opcode (&gt;40)
       INCT R0            Skips FETC @&gt;836D (since we can&#39;t put data in GROM)
       MOV  R0,@WHERE
       MOV  R9,@GBASE
       BLWP @GPLG10       Because subprograms return with CALL G@&gt;0012
GBASE  DATA 0
WHERE  DATA 0             Enter G@&gt;0010 directly (cause we can&#39;t FETC @&gt;836D)`
       MOVB @&gt;837C,R1     Upon return: See if subprogram was found
       SLA  R1,3          by testing Cnd bit
       JOC  NOFND
       INCT R11           Skip word if it was found
NOFND  B    *R11

####  Situation upon entry in a subprogram

Assembly subprogram called with XML \>19 or an equivalent routine are
entered with the GPL worskpace \>83E0. The following values can be
expected:

R1: Number of times the subprogram was called (normally 1).
R9: Address of the subprogram
R11: Return address (to keep scanning)
R12: CRU base address of the card.
\>83D0-D1: CRU base address of the card
\>83D2-D3: link to next subprogram in the header

The most usefull are R1 and R12. They allow the possiblility of
installing several identical cards in the PE-Box, each with a different
CRU address (generally DIP-switch selected). The subprogram can use R12
to know the CRU of the card it is in and R1 to know how many cards were
found before. The [RS232/PIO card](rs232c.htm#RomDSR) works this way.

GPL subprograms will find similar informations in the scratch-pad:

\>836C: Number of times the subprogram was called
\>83D0-D1: GROM base (normally \>9800)
\>83D2-D3: link to next subprogram in the header

###  Returning from a subprogram

Assembly language subprogram typically return with::

       B    *R11            

This prevents the scanning routine (XML \>19 or a clone of it) from
scanning further cards for the same subprogram. Unless of course this is
precisely what you want to do. In this case, just return with:
GPL subprogram often return with:
since they can expect to have been called by the GPL DSRLNK routine
located at \>0010 in the console GROM.

###  Passing parameters to a subprogram

Contrarily to DSRs, there is no conventions as how to pass parameters to
a subprogram. It depends on the language from which it is called. As a
consequence, you'll probably have to write several versions of your
subprogram, one for each language it can be called from (or at least
have different entry points). For instance, the disk controller card
contains a subprogram called "FILES" to be called from TI-Basic and an
almost equivalent one called \>16 to be called from assembly (the only
difference is that you can have upto 16 files in assembly, but only 9 in
Basic).

####  Subprograms called from assembly

If the caller is assembly language, the parameters are generally passed
on the scratch-pad (so as not to depend from the presence of a memory
expansion card). Convenient addresses for that purpose are \>834A-8353.
If that's not enough space, you could pass a pointer to a VDP memory
address in these bytes. Byte \>8350 is often used to return an error
code: \>00 means no error, other values indicate an error.

Which means you should provide the user with a good doc, so that he/she
knows how to call your subprogram. Don't forget to explain the error
codes, if any.

Example:

The disk controller card subroutine \>12 (file protect/unprotect)
expects the following parameters.
\>834C: drive number, from \>01 to \>03
\>834D: protection code: \>00 = unprotect, \>FF = protect
\>834E-834F: an address in VDP memory where to find a 10-characters
filename.
It returns an error code in \>8350 if something went wrong.

####  Subprograms called from Basic

Things get really messy when parameters must be passed from Basic or
Extended basic. That's because the CALL statement does not perform any
parameter passing. Therefore, your call must parse the Basic statement
to extract the parameter specifications. If these are variables you must
then get their values from the Basic symbol table. That's not easy to
do, to say the least.

Let me first mention a simple trick that makes the programer's life
easier, but puts some burden on the user. Just tell the user to pass
parameters after the subprogram name, separating them with a dot. For
instance:
CALL MYSUB.A=0.TEST2.UNLOCK

That's somewhat annoying for the user because CALLs cannot be placed in
a variable: CALL A\$ is not allowed. Therefore the user must enter a
different line for each desired value of a parameter. In the above
example, it A can have 16 different values, it means a ON A GOTO to 16
different CALL MYSUB.A=, each with a different value of A...

The user will certainly prefer this kind of syntax:
CALL MYSUB(A,"TEST2",U\$,512)
But now the burden is on you!

First you must parse the basic statement, looking for parameters. Basic
codewords are not stored as such in memory: to save place they are
converted to 1-byte tokens. For instance, the above statement is encoded
as:

    VDP address  Token/bytes  Meaning
    >37B4        >17          Line size
    >37B5        >9D          CALL
    >37B6        >C8          Unquoted string
    >37B7        >05          String length
    >37B8        MYSUB        The name is of course not encoded
    >37BD        >B7          (
    >37BE        A            Variable names are not encoded
    >37BF        >B3          ,
    >37C0        >C7          Quoted string "..."
    >37C1        >05          String length
    >37C2        TEST2        Content of the string
    >37C7        >B3          ,
    >37C8        U$           Another variable
    >37CA        >B3          ,
    >37CB        >C8          Unquoted string
    >37CC        512          Numeric constants are passed as strings
    >37CF        >B6          )
    >37D0        >00          End-of-line mark

You'll find a pointer to the current token in word \>832C, and the last
processed token is copied in byte \>8342. In this example, it will be
the \>C8 token following the CALL and \>832C will contain \>37B7.

TI-Basic programs can be stored in VDP memory or in GRAM. To know where
it is, test byte \>8389: if it's \>00 the program is in VDP memory,
otherwise it's in GRAM. Extended basic programs can't be in GRAM but
they can be stored in the high memory expansion, if it's present. In
this case, a non-zero value in byte \>8389 means that the program is in
the memory expansion.

A convenient way to avoid all this hassle is to use XML \>1B: it will
get the next token (or character) from the adequate memory, put it in
\>8342 and increment \>832C accordingly.

But first, you must skip the name of your subprogram (easy: you know
it's 5 chars long, so just add 5 to \>832C). Then make sure there is a
parenthesis, fetch each parameter, check for commas and for the closed
parenthesis. If you're using GPL, a` PARS >B6` instruction comes handy
for this job. Finally you must return with \>00 in \>8342 otherwise
Basic will complain.

But wait, when I said "fetch parameters" I kind of overlooked several
problems.

If the parameter is a **string constant** ("TEST2" in the example
above), no problems: just use it as it is. Almost, that is: the user may
have the bad idea to concatenate two string constants with the &
operator (e.g. "TEST"&"2"). You must therefore test for this situation:
the token for & is \>B8.

If its a **numeric constant** (512 in the example), you'll have to
convert it to a number. That can be easy if your subprogram accept only
a limited range of values (e.g. 0 to 9: just fetch the first character
and substract \>30 from it). If that's not the case, you have to use a
string-to-number conversion routine. Fortunately, there is one the the
console ROM: XML \>11 and that one can be called from assembly. Place
the address of the string in \>8356 and call XML \>11. You'll find the
result in floating point format in \>834A-8352. If an error occurs, it
will be announced in byte \>8354. You'll also find the sign in \>8375
(\>00 positive, \>FF negative) and the exponent in \>8376, if you have a
need for these.

And what about **variables**? Oh boy, that's where it becomes really
complicated. You must first call XML \>13 that will search the Basic
symbol table for the variable whose name is pointed at by \>832C. If it
finds the variable, it places its address in the Basic value stack in
word \>834A-834B. However, if it does not find it, it returns to the GPL
interpreter to announce an error in Basic!

Now you must get the value of that variable.
This is achieved by XML \>14: it dumps the value stack entry for the
variable pointed at by \>834A-834B into bytes \>834A-8351. When it
returns, you'll find a pointer to the value (in VDP memory) in word
\>834E. But first you must test byte \>834C: it will be \>00 for a
numeric variable and \>65 for a string variable. You may also test
\>834D: it contains the number of dimensions for array variables (\>00
of its not an array). For non-array string variables, you'll find the
length of the string in word \>8350. I'm not sure it's the case for
string arrays. [Anyone](feedback.htm#questions)?

Oh, and don't forget that here also the user may have used the
concatenation operator & to combine several string variables: you'll
have to get each and everyone of them using the above method. Of course,
you could just forbid the use of this operator and return with a syntax
error...

Finally, you may want to **convert floating point** numbers to integers.
This can be done with XML \>12. It will place the result in word
\>834A-834B and the error code in \>8354 (\>03 if the number is not in
the range -32768 to +32767). A quick trick though: if a floating point
number is zero, the first word (in \>834A) will be \>0000, so no
conversion is necessary.

Ok, will you remember all this? Well, I warned you: parameter passing is
a mess with subprograms.

And just to depress you a little more: I'm not sure XML \>13 and XML
\>14 apply to Extended Basic (in fact I think they don't...).

Ok, here is an example of parameter
passing to a GPL subprogram called from Basic

* It is supposed to be called with CALL TEST(123,A$), the A$ being optional`
MYSUB  CLR   @&gt;834A             Skip our subprogram name in the basic program
       ST    V*&gt;832C,@&gt;834B     Get size byte, make it a word
       DINC  @&gt;832C             Skip size byte
       DADD  @&gt;834A,@&gt;832C      Skip subprogram name
       XML   &gt;1B                Get next token
       CEQ   &gt;B7,@&gt;8342         Is it ( ?
       BR    ERRIS              No: incorrect statement`
       XML   &gt;1B                Get next token
       PARS  &gt;B6                Parse to next param or the &gt;B6 (closed parenthesis)
       CEQ   &gt;65,@&gt;834C         Is it a string
       BS    ERRSN              Yes: string-number mismatch
       XML   &gt;12                Make it an integer
       DST   @&gt;834A,....        Use it`
       CEQ   &gt;B3,@&gt;8342         Is next token a , ?
       BR    NOCOMA
       XML   &gt;1B                Get next token (we could also have used PARS &gt;B6)
       XML   &gt;13                Get symbol address in value stack
       XML   &gt;14                Get entry from value stack
       CEQ   &gt;65,@&gt;834C         Is it a string ?
       BR    ERRSN              No: string number mismatch
       MOVE  @&gt;8350,V*&gt;834E,... Get string, use it`
NOCOMA CEQ   &gt;B6,@&gt;8342         Is there a ) ?
       BR    ERRIS              No: &quot;Incorrect statement&quot; error
       XML   &gt;1B                Get next token
       CZ    @&gt;8342             Is it &gt;00 (end-of-statement)
       BR    ERR1               No: error (optional: CALL does it anyway)
       CALL  G@&gt;0012            Return to caller (i.e. Basic) `
ERRSN  CALL  G@&gt;001C            Call error routine (returns to Basic)
       BYTE  &gt;20,&gt;7D            &quot;String-number mismatch&quot; in console GROM     `
ERRIS  CALL  G@&gt;001C            Call error routine (returns to Basic)
       BYTE  &gt;20,&gt;2C            &quot;Incorrect statement&quot; in console GROM    

###  Subprograms returning values

Here again it's easy for subprograms called from assembly: just place
the value (or a pointer to it) in the scratch-pad.

Things are more complicated in Basic, mainly because of string
variables. If you want to return a string, you cannot be sure that it
will fit in the memory currently allocated for the string variable. Most
probably it won't, as the user will call your subprogram with an empty
string variable. You must therefore use XML \>15 to assign a value to a
variable, whether string or number.

To call XML \>15 you first make a copy of the variable entry in the
value stack (as returned by XML \>14) at the top of the value stack.
This is achieved by calling XML \>17.

Then for **numeric variables**, just place the new floating point value
on \>834A-8352 and call XML \>15: it will transfer the value in the
proper memory location. Uh-oh, we have a problem here: there is no XML
to convert an integer to a floating point number so you'll have to do it
yourself!

For **string variables**, you must first
get enough room for your string in the Basic string space. There is a
GPL routine that does this: place the required number of bytes in
\>830C-830D and `CALL G@>0038`, the address of the allocated space will
be returned in \>831C. You can easily call it from GPL, but if your
subprogram is written in assembly language, this may be a problem. You
could either use the [GPLLNK](#GPLLNK) routine outlined above, or
simulate G@\>0038 with an assembly routine.

* It allocates space for a string in the Basic string space
* It is less efficient than the GPL version as it does not
* perform garbage collection if there is not enough memory.
* The number of bytes to allocate should be in R2.
* The allocated address in VDP will be returned in R0.
* A memory full error can be detected with a JEQ.`
STRSPA DATA WREGS,STRSP1`
STRAP1 ORI  R15,&gt;2000     Set Eq bit
       MOV  @4(R13),R2    Get required string size
       MOV  R2,R1         Save it
       C    *R2,*R2+      We&#39;ll need 4 more bytes
       MOV  @&gt;831A,R4     String space address
       MOV  R4,R0         Save it for later
       S    R2,R4         The space is growing downwards
       MOV  @&gt;836E,R3     Basic value stack pointer
       AI   R3,&gt;0040      8*8 bytes must be free
       C    R4,R3         Do we have room enough?
       JLE  MFULL         No: memory full (or collect garbage and try again)
              SWPB R1            Put size byte in msb
       BLWP @VSBW         Put it in string space
       SWPB R1
       S    R1,R0         Adjust string space pointer
       MOV  R0,*R13       Return value into R0
       AI   R0,-4         Four more bytes needed
       MOV  R0,@&gt;831A     Save new pointer
       BLWP @VSBW         Write dummy value in first byte
       MOVB R1,@&gt;8C00     Clear next 2 bytes
       MOVB R1,@&gt;8C00
       SWPB R1
       MOVB R1,@&gt;8C00     Write size in 3rd byte
       ANDI R15,&gt;DFFF     Clear Equ bit to indicate success
MFULL  RTWP               Return with Equ bit set to indicate error
      

Now you can place your string in the VDP memory at the designated
address.

Then get the Basic variable entry by calling XML \>13 and XML \>14
successively and push it again on the Basic value stack by calling XML
\>17.

Then modify it so that it points at your string in the string space.
Finally call XML \>15 and that's it (wow).

In summary, this is how to return
variables from a GPL subprogram. All these XMLs can be called from
assembly, so you should not have too many problems to come up with an
equivalent version in assembly language. I'll leave it to you as an
exercise.

SENDST DST   &gt;000E,@&gt;830C    Put string size here
       CALL  G@&gt;0038         Will return to Basic if a &quot;memory full&quot; error occurs`
       MOVE  @&gt;830C,G@TESTRI,V*&gt;831C  Now put a string into that space `
       XML   &gt;1B             Get first char of variable name (required by XML &gt;13)
       XML   &gt;13             Get next symbol (in Basic statement) address
       XML   &gt;14             Get its entry in the value stack
       XML   &gt;17             Make a new copy of it
       CEQ   &gt;65,@&gt;834C      (Optional) Make sure it&#39;s a string variable
       BR    ERRSN           If not, call &quot;string-number mismatch&quot; error`
       DST   &gt;001C,@&gt;834A    New entry will be a string expression (flag &gt;001C)
       DST   @&gt;831C,@&gt;834E   Address of our string in the string space
       DST   &gt;000E,@&gt;8350    Put string size here
       XML   &gt;15             Assign string to string variable
       RTN`
TESTRI TEXT &#39;THIS IS A TEST&#39;`
* This GPL routine demonstrates how to return a value in a Basic numeric variable `
SENDNM XML   &gt;1B             Get first char of variable name
       XML   &gt;13             Get next symbol (in Basic statement) address
       XML   &gt;14             Get its entry in the value stack
       XML   &gt;17             Make a new copy of it
       CEQ   &gt;65,@&gt;834C      (Optional) Make sure it&#39;s not a string variable
       BS    ERRSN           If it is, call &quot;string-number mismatch&quot; error`
       DST   &gt;4101,@&gt;834A    Now put a floating point number on &gt;834A-8451
       DST   &gt;2345,@&gt;834C
       DCLR  @&gt;834E
       DCLR  @&gt;8350`
       XML   &gt;15             Assign it to the variable
       RTN`
ERRSN  CALL  G@&gt;001C         Call error routine (returns to Basic)
       BYTE  &gt;20,&gt;7D         &quot;String-number mismatch&quot; in console GROM  `
ERRIS  CALL  G@&gt;001C         Call error routine (returns to Basic)
       BYTE  &gt;20,&gt;2C         &quot;Incorrect statement&quot; in console GROM        `
* This is a sample GPL subprogram using the two routines above.
* Call it from Basic with CALL TEST(A$,X), then print A$ and X.
TEST   DADD  &gt;0005,@&gt;831C    Length of &quot;TEST&quot;, plus size byte
       XML   &gt;1B             Get next token
       CEQ   &gt;B7,@&gt;8342      Make sure there is a (
       BR    ERRIS
       CALL  SENDST          Send a test string
       CALL  SENDNM          Send a test number
       XML   &gt;1B             Get last token. Basic will make sure it&#39;s &gt;00
       CALL  G@&gt;0012         Return to Basic

##  DSRs

DSR stands for Device Service Routine. They are special subprograms
meant to access custom functions provided by the card. You could call
them "drivers" to follow the PC lingo. The main difference between DSRs
and subprograms is that DSRs use the concept of files as a standard way
of passing parameters. Therefore, in Basic DSRs are called by file
operations (OPEN, CLOSE, PRINT, INPUT, DELETE, OLD, and SAVE). They can
also be called from assembly, using a PAB to pass parameters to/from the
DSR. DSR names can have from 1 to 7 characters.

### Calling a DSR

DSRs are called the same way as subprograms. Most of the time, the same
scanning routine performs handles both types of calls: all it needs is a
parameter to get the first link in a header. This parameter is \>0008
for DSRs and \>000A for subprograms (see [structure of a
header](#header%20summary), above).

The situation upon entry in a DSR is therefore just the same as for
subprograms.

Most of the time, a DSR will be able to execute a variety of operations
(open a file, close it, etc). The first byte in the PAB is used to pass
an opcode, that specifies which operation is to be executed. DSRs are
thus more complex that most regular subprograms, but paradoxically
easier to use with Basic.

###  Returning from a DSR

Just like subprograms, assembly language DSRs typically return with::

       B    *R11            

This prevents the scanning routine from scanning further cards for the
same subprogram. Unless of course this is precisely what you want to do.
In this case, just return with:
GROM DSRs, written in GPL, generally return with:
since they can expect to have been called by CALL G@\>0010 or the
assembly language equivalent of it.

###  PAB: Peripheral Access Blocks

The convention to pass parameters to a DSR is to use a peripheral access
block (PAB). This is a bunch of data, generally stored in the VDP
memory, that is used to pass parameters to and from the called DSR or
subprogram. A PAB has the following structure:

Function.

Opcode

Err flag + File type

VDP buffer address
Record length

Number of chars

Rec # / file size
Bias / status

Name size

DSR name


#### Byte 0: Opcode

This byte is used to define the type of operation to be performed by the
DSR. The standard opcodes for file operations are the following:

\>00 Open. Opens the file, reserves room in VDP memory to store data in.

\>01 Close. Closes the file, frees the dedicated VDP memory area.

\>02 Read. Input data from the file/peripheral.

\>03 Write. Writes data to the file/peripheral.

\>04 Rewind. Points at a given record (or at top of file).

\>05 Load. Loads a memory image ("program") file.

\>06 Save. Saves a memory image file.

\>07 Delete. Removes a file from the storage unit.

\>08 Scratch. Removes a given record from a file.

\>09 Status. Returns informations about a file.

Some DSR accept other custom opcodes. For instance, the RS232 DSR
accepts:

\>80 Interrupts. A variant of Open that allows for interrupts upon
reception of a char on a serial port.

The Horizon Ramdisk supports several additional opcodes:

\>0A Assembly. Loads and runs a memory-image assembly file.
\>0B Basic. Loads and runs an extended basic file.
\>0C Cartridge. Loads a memory image GPL file.
\>B0 Rambo. Selects a memory bank to be mapped at \>6000-7FFF.
\>80 to \>8C: same as \>00 to \>0C, but uses a buffer in CPU memory
rather than in VDP memory.

#### Byte 1: Err/Type

The last 5 bits are used by the caller to define the file type.
Typically used by Open only.


Error code
&gt;00: Update &gt;04: Input
&gt;02: Output &gt;06: Append

The first 3 bits are used by the DSR to return an error code:

**0** (\>00): No error. I/O error 0 occurs when the calling subroutine
could not find a matching DSR anywhere.

**1** (\>20): Write protection. Attempt to write to a write-protected
medium.

**2** (\>40): Bad attribute. File attributes in the PAB (Dis/Int,
Var/Fix, Seq/Rel, rec lenght) don't match attributes on disk.

**3** (\>60): Illegal opcode. Unsupported or unknown opcode.

**4** (\>80): Memory full. There is no room to store the data or the
file definition (e.g more than 127 files).

**5** (\>A0): Past end-of-file. Attempt to read past the end of the
file.

**6** (\>C0): Device error. Something went wrong with the hardware.

**7** (\>E0): File error. Anything that do not fit in the above.

#### Bytes 2-3: Buffer address

These two bytes point to a data buffer, generally in VDP memory.

Read: write data from the file/peripheral into this buffer, record by
record.
Write: take data from this buffer to write it on file, record by
record.
Load: write data from the whole file/peripheral into this buffer in a
single operation.
Save: take data from this buffer to write it on file in a single
operation.

#### Byte 4: Record length

For "fixed" files, this is the absolute record length, in bytes. For
"variable" files, it is the maximum allowable record length. This byte
is not used for "program" files.

Open: When this byte is \>00, replace it with the default record length.
Else check if length is ok with that peripheral.
Read: Maximum number of bytes to transfer.

#### Byte 5: Number of chars

Read: how many bytes have been transfered from the peripheral to the
memory buffer.
Write: how many bytes must be transfered to the peripheral.

#### Bytes 6-7: Rec \# / File size

Rewind: point on that record, for next operation (must be Relative,
unless record \# is 0)
Scratch: delete that record.
Load: maximum number of bytes that can be loaded into the memory
buffer.
Save: number of bytes to write to the file/peripheral.

#### Byte 8: Bias / Status

Basic uses this byte to specify a screen bias, i.e. a value to be added
to each character in case the DSR wants to display an error message on
screen (the Basic bias is \>60). This is typically not used as DSRs do
not write directly on screen (the CS1 and CS2 DSRs are exceptions to
this rule)..

Status: Return the file status in that byte:


not used

N.B. Some DSR return the status after each and every opcode (The Horizon
Ramdisk does so), but that's not required by the TI specs.

#### Byte 9: Name length

The size in bytes of the DSR name, including file name and modifiers.

#### Byte 10+: DSR name

The DSR name should start from that byte. Valid DSR names must be 1-7
characters in lenght and should not contain decimal points. Exemples:
DSK1, RS232/1.

Additional parameters can be appended to the DSR name, using decimal
points as separators. E.g. a filename DSK1.MYFILE or transmission
parameters RS232.BA=4800.DA=8.CR.CH

What comes after the first decimal point depends on the DSR and is not
part of the PAB convention.

##   ISRs

Interrupt service routines (ISRs) only exist in peripheral card ROMs,
since the GPL interpreter allows interrupts: a GPL ISR would thus almost
certainly be interrupted by a VDP interrupt!. When the main ISR in the
console ROM determines that an interrupt did not come from the VDP, nor
from the TMS9901 timer, it assumes it came from a peripheral card. It
then scans all CRU addresses from \>1000 to \>1F00 and branches to each
and every ISR it finds in any card ROM.

It is the responsability of the card's ISR to determine whether the
interrupt came from that card or not.

An ISR always returns with B \*R11, so that the main ISR can scan other
cards. If however you would like to make sure no other card will be
scanned, here is how to return:

       LI   R0,&gt;1E00        This means SBZ 0
       LI   R1,&gt;045B        This means B *R11
       B    R0              Caution: not B *R0

We can easily jump out of the ISR scanning loop by incrementing R11 by
4. However, this skips an `SBZ 0` instruction and results in leaving our
card ROM "on", which most probably will crash the system. We must
therefore execute this `SBZ 0 `ourselves. But this instruction cannot be
placed in the card ROM since we won't be able to execute a program in
there once the ROM is turned off. Therefore, the `SBZ 0` and a `B *R11`
are copied into the scratch-pad into R0 and R1. We then continue
execution of the program in our worspace, and execute these two
instructions.

##  Foreign language translators

There is one more feature for GROMs: if GROM \>6000 in base \>9800 has a
negative version numbe (i.e. bit 0 is \>80), then the power-up routine
will branch at \>6010 just after is finished drawing the TI title
screen. This was means to implement a foreign language translator, that
would replace titles in english with titles in whatever language the
cartridge provided. Some game cartridges use this trick to take over
control of the TI-99/4A, just like some do with the power-up routines.

After the user pressed a key and the main program menu is displayed,
another translator kicks in. Its address is \>6013 in base \>9800. This
leaves you only 3 bytes to install the first translator, but that's all
you need for a `B G@>xxxx` statement. This second translator is in
charge of translating the words "PRESS" and "FOR" in the menu (it could
possibly translate program name, but that would be quite a task...).
Revision 1. 3/19/99 OK to release

Revision 2. 5/28/99 Added "Headers" and "PAB" chapters.
Revision 3. 6/6/99 Completely modified. Added chapter on subprograms
with calling examples, etc.

[Back to the TI-99/4A Tech Pages](titechpages.md)
