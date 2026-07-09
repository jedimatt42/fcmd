# The keyboard

The keyboard is internally organised as 6 columns of 8 keys. Note that
both shift keys are connected together and that the alpha-lock key has
its own "column" connection. Presing a key closes the connection between
a column and a row, thus the CPU can detect a key by energizing columns
one at a time and reading the status of the rows.

**Hardware  **
[Internal connector
](#hardware)[Circuitery
](#circuit)[CRU map
](#CRU%20map)[Phantom keys](#phantom)

**Software  **
[Built-in routines
](#Built-in)[Quick scanning routines
](#quick%20scan)[Auto-repeat keys
](#auto-repeat)[Interrupt-activating keys](#int%20driven)

[Keyboard tutorial and challenge](tutor1.md)

## Hardware

### Connector

The keyboard in connected to the console mother board by a 16-pin
connector that is only appearent when opening the console. Pin number 1
should be labelled as such on the motherboard. Wire number 15 on the
keyboard cable is generally red, whereas other wires are green.

     1 Keyboard row 2 (enter O I U Y P joystick_right)
     2 Keyboard row 3 (  9 8 7 6 0 joystick_down)
     3 Keyboard row 5 (shift S D F G A)
     4 Keyboard row 1 (space L K J H ; joystick_left)
     5 Keyboard row 0 ( =    . , m n / joystick_fire)
     6 Keyboard alpha-lock key
     7 Keyboard row 4 (fctn  2 3 4 5 1 joystick_up alpha_lock)
     8 Keyboard column 5 ( / ; P 0 1 A Q Z)
     9 Keyboard column 4 ( N H Y 6 5 G T B)
    10 Keyboard row 6 (ctrl W E R T Q)
    11 Keyboard row 7 ( X C V B Z)
    12 Keyboard column 0 (= space enter  fctn shift ctrl )
    13 Keyboard column 1 (. L O 9 2 S W X)
    14 Keyboard column 2 (, K I 8 3 D E C)
    15 Keyboard column 3 (M J U 7 4 F R V)
    16

### Internal circuitery

The keyboard is organised as 6 columns of 8 keys. The columns are
connected to six of the open collector outputs of a 74LS156 and pulled
up to +5V via 1K resistors (the remaining two outputs are used for
joysticks). The keyboard rows (and joystick inputs) are connected to the
inputs of the TMS9901 programmable system interface and pulled up at +5V
via 10K resistors. The TMS9901 also controls the decoder in charge of
the row selection. Pressing a key results in physically connecting the
wires connected to this column and that row.

------+              74LS156     +5V---WWW---+---||---Gnd
      |           +-----------+              |
   P2 |-----------| S0    1Y0*|--------------+---UUU------&gt; Keyboard connector
   P3 |-----------| S1    1Y1*|--- Ditto        6.8 uH
   P4 |-------+---| 1GA*  2Y0*|--- Ditto         1K
      |       &#39;---| 2GA*  2Y1*|--- Ditto    ,---WWW---+----+5V
      |           |       2Y2*|--- Ditto    |    ,----&#39;
      |       ,---| 1GB*  2Y3*|--- Ditto    |  |/
      |       +---| 2GB*  1Y2*|-------------+--|
      |       |   |       1Y3*|--- Ditto       |\      270 Ohm   10 Ohm
      |      Gnd  |           |          1.5K    V-+---WWW---+---WWW---&gt; Joystick
      |           |           |     -5V---WWW------&#39;         &#39;---||---Gnd
      |           +-----------+                                 10 pF
      |
      |                              10K      10 pF
      |                        +5V---WWW---+---||---Gnd
      |                                    |
INT3* |------------------------------------+---WWW-----+---&lt; Keyboard connector
INT4* |--- Ditto                             470 Ohms  |
INT5* |--- Ditto                                       &#39;---&lt; Joystick connector
INT6* |--- Ditto
INT7* |--- Ditto
INT8* |--- Ditto
INT9* |--- Ditto
INT10*|--- Ditto
   P5 |--- Ditto (output to alpha-lock key)
------+

To scan the keyboard, we must first activate a column by sending its
number to CRU bit 18-20 at R12-address \>0024. The bits are fed to the
74LS156 decoder that brings the corresponding keyboard column low. Then
we must read the status of each keyboard row in CRU bits 3-10 at
R12-address \>0006. If a key is pressed the row will read as 0,
otherwise it reads as 1 (non-intuituve!).

###  CRU map

Here is the keyboard layout in terms of CRU:
The alpha-lock key is activated by the TMS9901 output P5 (R12 address
\>002A) and reads together with keyboard row 4. This has the very
unfortunate effect of messing up the joystick "up" signal when the
alpha-lock key is engaged. Why the TI engineers did not pick row 7 (pin
\#11) instead, is a mystery to me.

###  Phantom keys

We can now explain the phenomenon known as "phantom key": when you press
three keys together, the console may detect a fourth key, even if it
isn't pressed. This occurs when the keys you pressed occupy the three
corners of a square in the above table: the phantom key is the one on
the fourth corner.

For instance, suppose you pressed Ctrl-Shift-S. When the processor tests
column 0, it brings pin 12 low. Since both \ and \ are
pressed, both pin 3 and pin 10 will be low and CRU bits 8 and 9 read as
0. So far, so good. But let's see what happens when testing column 1.
Pin 13 goes low and, since \ is pressed, so does pin 3 thus bit 8
reads as 0. BUT, since the \ key is placed on the same row,
column 0 will also go low, and since \ is down, pin 10 goes low
and bit 9 reads as 0. Which creates the illusion that \ was pressed.
See what I mean?

All this is due to the fact that electrons can flow back from a row to a
column. To prevent this from happening we would need to install a diode
at each key. That's something you should consider if you ever wanted to
create a piano keyboard for the TI-99/4A. On a piano it is very common
that 3 or more keys are pressed. On a computer on the other hand, it is
rare to press more than two keys, so we can live with the phantom key
problem. Also note that some multiple key combinations are ok, if the
three keys are aligned on the same column (Ctrl-Shift-=), on the same
row (1-2-3), or do not form a square (Ctrl-Shift-1).

##   Software

### Built-in keyboard scanning routines

This is the standard way to call the built-in keyboard scanning routine
in the console ROM. Note that we must change the workspace to \>83E0
using a `LWPI `instruction, and not a `BLWP`. This is because R13 to R15
in the \>83E0 workspace are used to hold special values that should not
be erased (GROM base, various flags, VDP write-address port). To save
space, I have placed my WREGS workspace in such a way that R0-R12 are
overwritten by the program, you don't have to do that if you don't want
to.

On the other hand, the test for Fctn-4 can be called with any workspace,
as it only uses R12.

* Calls the built-in keyboard scanning in console ROM
*--------------------------------------------
WREGS
KSCAN  DATA WREGS,KSCAN1
 KSCAN1 LWPI &gt;83E0           can&#39;t change WS with BLWP as R13-R15 are in use
       MOV  R11,@OLDR11     save GPL R11
       BL   @&gt;000E          call keyboard scanning routine
       MOV  @OLDR11,R11     restore GPL R11
       LWPI WREGS
       RTWP`
       BSS  6               R13-R15
OLDR11 DATA 0`
* Caller routine`
       BLWP @KSCAN
       MOVB @&gt;837C,R1       Check flag for key pressed
       SLA  R1,3            Flag value is &gt;20
       JNC  NOKEY           No key was pressed
       MOVB @&gt;8375,R1       Get key code
       ...`
*---------------------------------------------
* Calling the built-in Fctn-4 test in console ROM
*---------------------------------------------
TESTF4 BL   @&gt;0020          R12 will be modified
       JEQ  CLEAR           Fctn-4 was pressed
       JNE  NOCLR           It wasn&#39;t                 

###  Quick keyboard scanning routines

Now, some home-made routine. The first one checks whether a key is down.
If yes, it calls the standard KSCAN routine to get the ASCII code,
otherwise it just returns. This speeds up programs that needs to perform
a lot of operations while still watching the keyboard: no need to call
the slow KSCAN routine if no key is pressed. A similar routine could be
used to wait until the key has been released.

* Quick-and-dirty check to see if a key is pressed.
* If not return in a hurry,
* else call the standard scanning routine.
* Uses R1, R2 and R12.
*--------------------------------------------
KCHECK CLR  R1                 Start with column 0
LP1    LI   R12,&gt;0024          R12-address for column selection
       LDCR R1,3               Select a column
       LI   R12,&gt;0006          R12-address to read rows
       SETO R2                 Make sure all bits are 1
       STCR R2,8               Get 8 row values
       INV  R2                 Since pressed keys read as 0
       JNE  KPR                A key was pressed
       AI   R1,&gt;0100           Next column
       CI   R1,&gt;0600           Are we done
       JNE  LP1                Not yet
       B    *R11               No key pressed: return (with EQ bit set)
 KPR    BLWP @KSCAN             Call KSCAN routine
       INCT R11               (either) signal key by skipping a jump
*      MOV  R2,R2             (or) signal key by clearing the EQ bit
       B    *R11               Return


Depending on the signaling technique chosen, the main program calls the
routine either like this:

       BL   @KCHECK           Assuming the INCT R11 solution
       JMP  GOON              Keep going
       ...                    Process key

or like that (slightly faster since JNE is not taken when no key is
detected).

       BL @KCHECK            Assuming the MOV R2,R2 solution
       JNE KEY               Process key
       JEQ NOKEY             Keep going
Of course things are even easier if we only want to check for one key or
for a combination of keys located on the same column (and now you know
why \ is Fctn-=). No need for a loop, just test the appropriate
column:

* Routine to detect a key combination (in this case Fctn-=),
* returns with Eq bit set if this is the case.
* This routine alters R12 and R1
*--------------------------------------------
ISQUIT CLR  R1                  Test column 0
       LI   R12,&gt;0024           Address for column selection
       LDCR R1,3                Select column
       LI   R12,&gt;0006           Address to read rows
       STCR R1,8
       ANDI R1,&gt;1100            Mask all irrelevant bits
       B    *R11                Else return
 *--------------------------------------------
* Routine to detect a specific key (in this case &lt;enter&gt;),
* it returns with Eq bit set if &lt;enter&gt; was NOT pressed.
* Alters R12 only
*--------------------------------------------
ISENTR CLR  R1                  Test column 0
       LI   R12,&gt;0024           Address for column selection
       LDCR R12,3               Select column
       TB   -13                 Test R12-address &gt;000A, i.e &lt;enter&gt;
       B    *R11                &lt;enter&gt; was not pressed: return
 `
* Caller routine
       BL   @ISQUIT
       JNE  NOQUIT
       JEQ  QUIT
       BL   @ISENTR
       JNE  ENTER
       JEQ  NOENTR

Note that the "negative TB" trick would also
have worked for \ (using two tests of course).

###  Auto-repeat keys

In TI-Basic, if you hold a key down for a moment, it begins repeating
itself at high speed. This feature is not included in the console KSCAN
routine. Rather, KSCAN only detect changes, which requires a key to be
released (or another key pressed) before it can be repeated. Let's see
if we can get around that.

We all know the canonic addresses to be used with KSCAN:
\>8374 contains the keyboard argument.
\>8375 returns the key code, or \>FF if no key was pressed.
\>8376 returns the X-value for a joystick (0,4, or \>FC).
\>8377 returns the X-value for a joystick.
\>837C bit 2 (value \>20) is set if a key was pressed.
\>83E0 Workspace used by KSCAN.
\>000E Branch vector for the KSCAN routine

Now, how does KSCAN know that the key was pressed before? Well, it just
stores a key scan code (not the standard ascii code) in a given address.
In fact, there are three such addresses, one for each keyboard argument
0, 4 and 5. Another address is used to remember the current keyboard
argument, to be used when KSCAN is called with \>00 in byte \>8374.

\>83C6 Contains the default keyboard argument minus 3 (i.e. 0-2).
\>83C7 Contains keyboard column 0 (special keys).
\>83C8 Scan code of current key, whatever keyboard type.
\>83C9 Ditto for keyboard type 4 (Pascal).
\>83CA Ditto for keyboard type 5 (Standard).

So, by clearing the relevant byte, we ensure that KSCAN always returns
with the \>20 bit set in \>837C, even if the same key was held down. Of
course, we must now include a delay loop to avoid the key being repeated
at ultra-fast speed. In fact, we probably want two types of delay: how
long before the repeat operation begins and how long between each
repetition. The second delay is typically much smaller that the first.

* Keyboard scanning with auto-repeat.
* Put even values in R0 and R1 either at assembly or at run time.
* Put an odd value in R0 to disable auto-repeat.
*--------------------------------------------
MYSCAN DATA WREGS1,MSK1
 MSK1   C    R2,R1             Are we repeating?
       JEQ  MSK2              Yes
       MOVB R4,@&gt;8374         Set keyboard argument
       MOV  R0,R2             Initially, use long delay
 MSK2   MOV  R2,R6             Reload delay
MSL1   SETO @&gt;83C8            Erase previous key&#39;s scan code
       MOVB @&gt;83C8,@&gt;83CA     Ditto for keyboard 5
       LWPI &gt;83E0
       BL   @&gt;000E            Call scanning routine
       LWPI WREGS1
       MOVB @&gt;8375,R11        Get key&#39;s ascii code
       CB   R11,R3            Same as before?
       JNE  NEWKEY            No
       DECT R6                Yes: wait. Never zero if odd delay
       JNE  MSL1              Keep scanning (as key could change)
       MOV  R1,R2             Done with waiting: load repeat delay
       JMP  USEKEY            Do first repetition
 NEWKEY MOVB @&gt;8375,R3         Memorize current key (will be &gt;FF if no key)
       MOV  R0,R2             Load initial delay
USEKEY MOVB R3,*R13           Pass key to caller, in R0
       MOVB @&gt;837C,11         Signal key pressed, if any
       ANDI 11,&gt;2000          Keep only &quot;key pressed&quot; flag
       SOCB 11,15             Transfer it into Eq bit of caller
       RTWP                   Return to caller
*
WREGS1 DATA &gt;0080             R0: Time before auto-repeat kicks in
       DATA &gt;0020             R1: Delay between repeats
       DATA &gt;0080             R2: Current delay
       DATA &gt;FF00             R3: Key buffer
       DATA &gt;0500             R4: Keyboard argument
       BSS  24                R4-R15

###  An interrupt-driven keyboard?

Given the way the keyboard is wired, it is not possible to set it in
such a way that pressing any key would generate an interrupt. On the
other hand, it is possible to set it so that a subset of keys (a column)
trigger an interrupt when pressed. This provides us with an alternative
to the quick-scan routines described above.

For a description of this feature, check the page dealing with the
[TMS9901](tms9901.htm#Fun).

Revision 1. 2/13/99 OK to release

Revision 2. 3/31/99 Polishing
Revision 3. 5/30/99 Tested & debugged examples
[Back to the TI-99/4A Tech Pages](titechpages.md)
