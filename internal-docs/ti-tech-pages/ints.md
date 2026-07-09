# Interrupts

An interrupt is a signal that tells the CPU to interrupt execution of
the current program and to call a special subroutine: the **ISR**
(interrupt service routine). Once the ISR returns, execution of the
current program will resume where it was. This is extremely usefull for
many reasons: you can automatically perform a task at given intervals,
or you can have a peripheral asking for the attention of the CPU. The
alternative is for the CPU to constantly check all peripherals to see if
one of them needs attention, obviously a very inneficient way (however,
that's how the TI-99/4A scans the keyboard. Wouldn't it be nice if the
keyboard could send interrupts just like a PC? More on this later).

The TMS9900 accepts two kind of interrupts:16 different maskable
interrupts that are signaled by activating the INTREQ\* pin (and whose
number is expected on the IC0-IC3 pins) and a single non-maskable
interrupt signalled by the LOAD\* pin.
**Non-maskable interrupt  **
[Triggering LOAD\*
](#load%20triggering)[Service routine](#UISR)

**Maskable interrupts  **
[In the TI-99/4A](#maskable)

**Interrupt service routine  **
[Peripheral interrupts
](#ISR)[VDP interrupts
](#VDP%20ISR)[\_\_Sprites
](#sprites)[\_\_Sounds
](#sounds)[\_\_Quit key
](#quit)[\_\_Blank screen
](#blank)[\_\_ISR hook
](#hook)[Timer interrupts](#Timer%20ISR)

##   Non-maskable, LOAD\* interrupt

When the LOAD\* line becomes active (low) the TMS9900 executes the
equivalent of a `BLWP @>FFFC`, i.e. it fetches a new workspace pointer
at \>FFFC, saves the old workspace, program counter and status in the
new R13, R14 and R15 respectively, then branches to the address found in
\>FFFE. Since \>FFFC-FFFE are in the high memory expansion, we can place
any vectors we want in there.

###  Activating the LOAD\* line

This line is present in the side-port of the TI-99/4A, on pin \#13 (7th
from the right, at the bottom of the connector when looking inside the
console). However, the flex cable connector does not carry this signal
to the PE-Box. If we want to use it we must intercept it at the level of
the connector.

I have a mouse (WiPo mouse 99) that works according to this principle: a
small printed circuit board is placed inbetween the side port and the
flex cable. The circuitery on it drives power from the side port and
issues non-maskable interrupts when the mouse is moved. A piece of cable
cable feeds all remaining informations (which way did it move, wich
buttons are down) to the joystick port.

You can easily build a small circuit to connect the LOAD\* pin to the
ground and trigger non-maskable interrupts. However, make sure your
circuit is a "one-shot", since interrupts will be issued as long as pin
\#13 is low. That kind of interrupt is called "level-triggered" as
opposed to "edge-triggered" (an interrupt that would only occur when the
pin changes from high to low).

The main advantage of level trigerred interrupts, apart from being less
sensitive to transients (i.e. noise), is that several devices can share
the same interrupt line. Suppose a first device brings the line down
but, before the ISR cleared the interrupt and brought the line up again,
another interrupt occurs. Once the ISR is done with the first interrupts
and clears it, the line stays down which triggered a new interrupt (it
wouldn't with level-triggering since the line does not change from high
to low, which is the triggering event). Of course, there is another side
to that coin...

###  Unmaskable interrupt service routine (UISR)

The drag with an unmaskable interrupt (especially level triggered) is
that another interrupt could occur before we have processed the current
one. We may thus run into two problems when the UISR is entered for the
second time:

1.  Wherever we saved the return address, it will now be overwritten by
    a new return address (pointing inside the UISR itself). Thus the
    UISR will never return to the main program.
2.  Re-entering the UISR before its completion is likely to mess up all
    variables used by the first instance of it.

Here is a simple, but imperfect solution:

UISR   MOV  @NEWWR,@&gt;FFFC        Change workspace for futur interrupts
       MOV  @NEWPC,@&gt;FFFE        Change PC: now points at RTWP
       LWPI UREGS1               In case we-retered after line 1
       ...                       Handle interrupt
INSTAL MOV  @UISRPC,@&gt;FFFE       Restore pointer to UISR entry point
       MOV  @UISRWR,@&gt;FFFC       Restore workspace
TORTWP RTWP                      Resume program execution`
UISRPC DATA UISR                 Address of UISR
UISRWR DATA UREGS1               UISR workspace
NEWPC  DATA TORTWP               Address of a RTWP
NEWWR  DATA UREGS2               Alternate workspace: provides one level of recurence `
* This routine installs the UISR hook
HOOKUI DATA UREGS1,INSTAL        Call it by BLWP @HOOKUI

See how it works? The first `MOV `instruction changes the workspace to
be used in case of a new interrupt. This way, the return values are
perfectly safe: no interrupt will occur before this first instruction is
completed. And after that, new return values will be saved in R13-R15 of
workspace UREGS2, thus won't overwrite those in UREGS1.

This solves problem 1, but what about problem 2 (re-entrancy)? The
second `MOV `instruction takes care of it: all subsequent interrupts
will transfer control to a `RTWP `instruction, that return immediately
to where we were in the UISR. Of course an interrupt could occur in
between the first two `MOV `instructions, but we don't care: this will
just repeat the first `MOV`, which is fine. We just have to make sure
we've got the right workspace: the `LWPI UREGS1` instruction takes care
of that.

Things are a bit more tricky at the end of the routine, when we must
restore the two vectors. If an interrupt occurs after we have restored
\>FFFE we will repeat the whole UISR from the beginning. This may or may
not be OK, depending on what we are doing inside this routine. If
however an interrupt occurs between the second `MOV `and the `RTWP`, the
return address will be overwritten and our UISR will loop forever!

I struggled with this problem for quite a time before a found a
solution. It's fairly intricate, so study it carefully.

UISR   MOV  @NEWWR,@&gt;FFFC        Change workspace for futur interrupts
       MOV  @NEWPC,@&gt;FFFE        Change PC: now points at RTWP
       CI   R13,UREGS2           Check for rentrancy
       JEQ  ENDUI                We came from the end point (before TORTWP)
       CI   R13,UREGS3           Double re-entrancy (after ENDUI + after UISR)
       JEQ  TORTWP               Return from the second one
       CI   R13,UREGS1           From next instruction after UISR or after ENDUI
       JNE  UIOK                 From outside UISR: no re-entrancy
       CI   R14,ENDUI            We re-entered, but from where?
       JL   TORTWP               From line 2, just after we changed &gt;FFFC
       JMP  ENDUI                From ENDUI, after we changed &gt;FFFC or &gt;FFFE
 UIOK   MOV  R13,@UREGS2+26       OK, we did not re-enter
       MOV  R14,@UREGS2+28       Save return parameters in R13-R15 of UREGS2
       MOV  R15,@UREGS2+30
 *      ...                       Do what we want to do in this UISR
        MOV  @OLDUWR,R13          (Optional) Link to previously installed  UISR
       JEQ  ENDUI                None
       BLWP @OLDUWR              Call previously installed UISR
 ENDUI  MOV  @UISRPC,@&gt;FFFE       Restore pointer to UISR entry point
       MOV  @UISRWR,@&gt;FFFC       Restore initial UISR workspace
       LWPI UREGS2               Switch to worskspace that contains saved values
TORTWP RTWP                      Return to caller`
UISRWR DATA UREGS1               Normal workspace for UISR
UISRPC DATA UISR                 UISR entry point
NEWWR  DATA UREGS3               Alternate workspace, if re-entered
NEWPC  DATA TORTWP               Points to a RTWP
 OLDUWR DATA 0                    To save WS of previous UISR, if any
OLDUPC DATA 0                    To save PC of previous UISR
 UREGS1 BSS  6                    Regular UISR worskpace
UREGS2 BSS  6                    Worskpace used to save R13-R15
UREGS3 BSS  32                   Alternate worksapce (uses only R13-R15)
 
* These routines install the UISR vectors in memory
HOOKUI DATA UREGS2,ENDUI         Just install our UISR
 LINKUI DATA UREGS2,LNK0          Check is another one is here, link to it
 LNK0   CLR  @OLDUWR              Flag: no link to previous UISR
       MOV  @&gt;FFFC,R1            Get current UISR worskpace
       C    R1,@UISRWR           Is it ours?
       JEQ  ENDUI                Yes: don&#39;t link
       CI   R1,&gt;2000             Check if valid
       JL   ENDUI                A workspace can&#39;t be in ROM
       CI   R1,&gt;8400
       JL   LNK1
       CI   R1,&gt;A000
       JL   ENDUI                Can&#39;t be in mapped area either
       SRL  R1,1
       JOC  ENDUI                Shouldn&#39;t be an uneven address
LNK1   MOV  @&gt;FFFE,R1            Get current UISR vector
       JEQ  ENDUI                None
       C    R1,@UISRPC           Is it our UISR?
       JEQ  ENDUI                Yes: then don&#39;t link
       CI   R1,&gt;8400             Check if valid
       JL   LNK2
       CI   R1,&gt;A000
       JL   ENDUI                Can&#39;t be in mapped area
LNK2   SRL  R1,1
       JOC  ENDUI                Shouldn&#39;t be an uneven address
       MOV  @&gt;FFFC,@OLDUWR       Ok, we may have a valid hook: save its workspace
       MOV  @&gt;FFFE,@OLDUPC       Save its vector
       JMP  ENDUI                And install ours instead
 *This routine unhooks the UISR
UNHOKU MOV  @OLDUWR,@&gt;FFFC       Restore previous workspace (or &gt;0000)
       MOV  @OLDUPC,@&gt;FFFE       Restore previous vector
       B    *R11                 Called with BL for simplicity

As you can see, the UISR tests the workspace of the caller. If it's not
its own, it knows there was no re-entrancy and it saves the return
vectors R13-R15 in an alternate workspace (so they can be loaded with a
single instruction). If the caller's worskpace is that of the UISR, it
means we re-entered. This can only happen at two places: before the
first `MOV @NEWPC,@>FFFE` or after the last `MOV @UISRPC,@>FFFE`. The
action to be taken is different in each case, but we can distinguish
them by checking R14 (return point).

Now let's see what happen if the UISR is re-entered:

- After the first line (`MOV @NEWWS,@>FFFC`). We're going to repeat this
  instruction upon re-entrancy: no problem, it can be repeated as many
  times as necessary. Once we can go on, the UISR will detect the
  re-entrancy thanks to R13, and immediately perform a `RTWP`, since R14
  points to the beginning of the UISR.
- After the second line (`MOV @NEWPC,@>FFFE`). From that point on, an
  immediate `RTWP `occurs. We can just ignore the re-entrancy problem
  from now on.
- After ENDUI (`MOV @UISRPC,@>FFFE`). We're going to start the UISR from
  the beginning again. However, it will detect the re-entrancy thanks to
  R13 and branch to ENDUI again, after checking R14. But wait, what if
  another interrupt occurs just after the first line? This will repeat
  the first line (which is ok) and change R14! But that's no problem
  either: thanks to the value of R13 (UREGS3), a RTWP will be executed
  and we'll be returned to line 2 as if nothing had happened.
- After the last `MOV @UISRWR,@>FFFC`. We re-enter and overwrite the
  return address! Fortunately, it was saved in another workspace by the
  three instructions at UISROK. The UISR will detect the re-entrancy and
  branch to the exit again.
- After the `LWPI UREGS2`. We re-enter with a different workspace. But
  this one also is detected, and the UISR attempts to exit again.
- After the `RTWP`. Well, we were back to the caller. So there is no
  re-entrancy problem here.

Another refinement I introduced is the possibility to link to a
previously installed UISR. This is slightly dangerous to do, as there is
no guaranty that the values found in \>FFFC and \>FFFE represent valid
UISR vectors. First of all, some programs just ignore unmaskable
interrupts and use the whole range of addresses to store their data. To
try to detect this possibility, the installation routine LINKUI performs
several checks to ensure the vectors are valid. Of course, another
possibility is that there was a valid UISR that was erased or
overwritten, but whose vectors remained intact. In this case, the
computer will most probably crash! Therefore, it is probably safer to
use HOOKUI to install your UISR unless you know for sure there is
another program running, with a valid UISR loaded (such as a mouse
driver).

Finally, just to avoid the above problem, you should call `BL @UNHOKU`
to clear the unmaskable interrupt vectors, once you are done.

##   Maskable interrupts

When the INTREQ\* line is low, the TMS9900 starts reading the number
present on lines IC0-IC3 after each instruction. It compares this number
to the value of the interrupt mask stored in the status register and if
the interrupt level is lower or equal to the mask it performs a
`BLWP `at an address that depend on the interrupt level.
Level 0 performs `BLWP @>0000`
Level 1 performs `BLWP @>0004`
Level 2 performs `BLWP @>0008`
...
Level 15 performs `BLWP @>003C`

The TMS9900 also automatically decreases the value of the interrupt mask
(provided it's not 0), so that interrupts with higher priority can
interrupt the ISR of the current one. You can programmatically change
the value of the interrupt mask with the `LIMI `instruction (Load
Interrupt Mask Immediate): `LIMI 0` through `LIMI 15`. Note that
interrupt 0 cannot be masked since `LIMI -1` is not allowed. But this
does not matter since interrupt 0 performs the same `BLWP` as the reset
signal, and would thus be useless as an interrupt anyway.

###  Maskable interrupts in the TI-99/4A

Texas Instruments obviously decided to make things simple with the
TI-99/4A: pins IC0 through IC3 are hardwired so that every interrupt has
a level of 1. This means that there are only two relevant
`LIMI `instructions:

       LIMI 0       Mask out interrupts

The vectors for interrupt 1 in the console ROM (at address \>0004-0007)
contain the values \>83C0, \>0900 therefore they branch to an interrupt
service routine that is also located in the console ROM. Which means we
have no control on maskable interrupts. What a pain in the neck!

##    The TI-99/4A interrupt service routine

The TI-99/4A unique ISR is located at address\>0900 and is entered with
worskpace \>83C0. The first thing it does is to disable further
interrupt with a `LIMI 0` instruction, thus getting rid of the
reentrancy problem. Then it changes the workspace to \>83E0, which is
the main workspace used by the GPL interpreter.

It then checks whether the interrupt was generated by the timer in the
TMS9901 chip. This timer is used for cassette operations, and the
routines that initializes it also loads a flag in bit 2 (value \>20) of
word \>83FD. If this bit is 1 the ISR branches to the timer subroutines.
Note that these routines never check whether the interrupt really came
from the timer, which means any other interrupt will be mistaken for a
timer interrupt if the flag bit is set in \>83FD!

If the timer flag is not set, the ISR tests bit 2 of the CRU: this asks
the interrupt controller TMS9901 whether the interrupt was generated by
the videoprocessor. If this is the case, the ISR executes VDP
subroutines.

If the interrupt did not come from the VDP, the ISR turns on peripherals
cards one at a time, from CRU address \>1000 to \>1F00. It then calls
each and every ISR is can find in these cards, untils either all cards
have been called or one of the cards stopped the search process.

The ISR then restores workspace \>83C0, and performs a `RTWP `to return
to the main program.

###  Peripheral cards interrupts

To generate an interrupt, a peripheral card must have some piece of
hardware that brings the EXTINT\* line low. This line is present on the
side port of the TI-99/4A (pin \#4) and is fed to the PE-Box by the flex
cable connector. In the PE-Box slots it is pin \# 17.

To implement an ISR, the peripheral card must have onboard ROM (or RAM)
that will be turned on by CRU bit 0 in its CRU address space (i.e
\>1000, \>1100, \>1200,... \>1F00). The first byte in the ROM must be
\>AA to indicate a standard header. Then word \>400C-400D must contain a
pointer to a chain of ISRs (which may consist in only one ISR). Each
link in the chain is made of two words: the address of the next link,
and the address of the ISR to be branched at. Here is an example,
featuring two ISRs:

    Address  Value      Meaning
    >4000    >AA        Signals a standard header
    >400C    >4020      Points to first link in chain of ISRs

    >4020    >4028      Points to next link
    >4024    >4100      Address of first ISR
    >4028    >0000      No more links
    >402C    >4200      Address of second ISR

    >4100     ...       First ISR starts here
    >4200     ...       Second ISR starts here

Note that multiple ISRs are kind of a luxury. Most of the time, you
won't need more than one ISR per card.
The peripheral ISR is in charge of checking whether the interrupt came
from that card or not. If it determines that the interrupt was indeed
issued by that card it should clear it by reseting line INTREQ\* to high
(this may be done automatically by the electronics) . It then performs
whatever action the interrupt is meant to trigger.

In any case, ISR should return with:
Theoretically, the ISR should also reset the "peripheral interrupt" bit
in the TMS9901:

        SBO   1 

However, this is generally not necessary because the TMS9901 only
latches interrupts for one clock cycle. If the peripheral card resets
its own interrupt-generating circuitery, CRU bit 1 will become inactive
at the next clock cycle.

Note that the main ISR will keep scanning peripheral cards and call
their ISRs until all CRU addresses are checked. There is a way to
prevent this after you dealt with an interrupt comming from your card.
See [ISRs](headers.htm#ISR) in the page on standard headers.

I am only aware of one card that handles interrupts: the RS232 card (and
it's quite buggy: see my [RS232 page](rs232c.htm#RomISR)). The Horizon
Ramdisk however, has RAM at \>4000-5FFF and thus offers you the
possibility to write your own peripheral ISR. The ramdisk won't trigger
interrupts, but if you install it at a low CRU address (e.g. \>1000)
you'll be able to intercept any interrupt issued by other cards.

###  VDP interrupts

The videoprocessor TMS9918 can be [programmed](tms9918a.htm#Registers)
to issue an interrupt each time it refreshes the screen, which occurs 60
times per second (50 times per second for the European model TMS9929A).
This interrupt is routed to the interrupt controller TMS9901 that echos
it on CRU bit 2 and triggers the INTREQ\* line.

Once the ISR has determined that the interrupt came from the VDP it does
the following:

- Move sprites in auto-motion, if any.
- Play sound list, if any.
- Test \ key, reset the TI-99/4A if it's pressed.
- Read VDP status and place it in byte \>837B.
- Increment by two a timeout counter and turn the screen off when it
  reaches zero.
- Call a user defined ISR, if any.

Four bits in byte \>83C2 are used to enable/disable the first 3
functions:
If the first bit (weight \>80) is set, the ISR jumps directly to point
4.
If the second bit (\>40) is set, the ISR won't handle sprites.
If the third bit (\>20) is set, the ISR won't process the sound list.
If the fourth bit (\>10) is set, the ISR won't test the \ key.

####  Sprite auto-motion

The ISR expects byte \>837A to contain the number of the highest sprite
in automotion. It also expects the sprite descriptor table to be at
address \>0300 in the VDP memory (VDP register 5 must contain \>06) and
a sprite motion table at \>0780 in the VDP memory. This motion table
comprises 4 bytes for each sprite in auto-motion: the first two must be
initialised with the desired speed, the next two are used by the ISR as
internal buffers.

(if needed)

Positive speeds (\>01 to \>7F) move the sprite to the right (or down).
The larger the number, the faster the sprite. Negative speeds (\>80 to
\>FF) move the sprite to the left (or up). \>FF is the slowest, \>80 the
fastest. If speed is zero the sprite does not move in this dimension. If
both speeds are zero that sprite does not move at all.

####  Sound list

The ISR expects the address of the sound list in the word \>83CC-83CD.
As it processes the list, it will update this word so as to constantly
point to the next bar to be processed. The sound list can be located
either in VDP memory or in GROM, the last bit of byte \>83FD is used to
determine which memory it is in: 0 for GROM, 1 for VDP. Finally \>83CE
serves as a buffer for the duration counter: nothing will be played if
it contains zero, so it should be initialized as \>01.

In summary, the sound-processing subroutine in the ISR does the
following:

- Check the byte at \>83CE, if it's zero don't do anything (end of list
  reached).
- Decrement \>83CE, if it does not become zero don't do anything (sound
  is being played).
- Get the size byte from the relevant memory. If it's a branch order
  (\>00 or \>FF) put the new address in \>83CC but do not change sounds
  yet.
- Else pass all sound bytes to the sound chip at address \>8400.
- Place new duration byte in \>83CE.
Each bar in the sound list begins with a mandatory size byte, followed
by several data bytes to be passed to the TMS9919 sound chip, and ends
with a duration byte. A given bar does not need to access all 4
generators: any generator that is not specified will continue to play
the same sound (if any).

     >ss                    Number of data bytes (not counting size nor duration)
     >8z >xy                Set frequency >xyz on generator 1 (watch the nibble order!)
     >9x                    Set attenuation >x on generator 1 (>0=max volume >F=off)
     >Az >xy                Set frequency >xyz on generator 2
     >Bx                    Set attenuation >x on generator 2
     >Cz >xy                Set frequency >xyz on generator 3
     >Dx                    Set attenuation >x on generator 3
     >Ex                    Set noise type+frequency
     >Fx                    Set attenuation x on noise generator
     >tt                    Duration in 60th of a second (50th in Europe)

     ...                    Next bar(s)

     >ss >9F >BF >DF >FF    Turn all generators off (wise but optional)
     >00                    Duration zero: end of list
Number of bytes
This specifies the number of sound bytes to be passed to the sound
generator, therefore it does not incluse the duration (nor the \# of
bytes itself)

There are two special values for this byte, \>00 and \>FF, that allow to
jump from one sound list to another or to create a loop inside the
current list (to repeat a tune forever). The syntax is:

     >00        Fetch next bar at address wxyz in the current memory
     >wx >yz    (i.e. place >wxyz in >83CC)

     >FF        Ditto, but change memory: go to VDP if we were in GROM and conversely
     >wx >yz    (i.e. invert bit 7 at >83FD)

Attenuation
The attenuation is 2 decibel (100 times less energy!) for each
increment by 1. Specifying an attenuation of \>F turns that generator
off.

Frequency
For tone generators 1 to 3, the frequency in Hertz can be calculated
as:

    F = 111860.8
          >xyz

For the noise generator, the noise characteristics are determined by the
last 3 bits of the frequency byte:

    1110 0wrr
          |||
          |00: 6691 Hz
          |01: 3496 Hz
          |10: 1748 Hz
          |11: Pick up frequency from tone generator 3 (whether it's on or not)
          |
          0: Periodic noise
          1: White noise

Duration
The duration byte specifies how many times the ISR must be called
before it processes the next bar.

####  Testing the \ key

The ISR scans column zero of the keyboard (=, space, enter, Fctn, Shift,
and Ctrl keys). If it matches the value found at \>004C in the console
ROM (\>11 for Fctn =), it immediately performs a `BLWP @>0000`,
effectively reseting the TI-99/4A. This is one of the rare cases when
the ISR does not return to the calling program.

####  Saving the VDP status

The ISR just reads the VDP status byte from \>8802 and stores it in byte
\>837B. This also clears the interrupt condition in the VDP.

####  Blanking the screen

The ISR increments by two the word at \>83D6. If it becomes zero, the
ISR uses a copy of VDP register 1 stored in byte \>83D4, sets the INT
bit (\>20) to ensure that further VDP interupts will be generated,
clears the SCR bit (\>40) and writes that byte to VDP register 1, which
results in turning the screen off.

Note that the word at \>83D6 is incremented by two, therefore if it
contains an odd value it will never reach zero and the screen will never
be blanked. Placing \>0000 it that word ensures the longer delay before
blanking the screen (it requires 32768 calls to the ISR, which takes
about 9 minutes), whereas \>FFFE turns the screen off at the next
interrupt.

####  Calling a user-define ISR

Now here comes the best part! The ISR checks the word at \>83C4 (the
interrupt hook), if it contains a non-zero value the ISR uses this value
as a pointer to a user-defined routine. It branches to this routine via
a `BL `instruction, with \>83E0 as the workspace. This means that we can
install an ISR of our own by placing its address in this word. In
general, you should first check whether another program has already
hooked it before to install your hook: this way you can chain the call
to the other program and don't disturb anything.

Example:

       LI   R0,MYISR
       MOV  R0,@&gt;83C4            Now hook the ISR
       ...
       LIMI 2                    Interrupts must be enabled somewhere
       LIMI 0                    in the main program
       ...`
MYISR ...                        Do what I want to do in our hooked routine
       MOV  @OLDHOK,R2           Chain to old hook
       B    *R2                  R11 still contains return address to the ISR
The anti-virus bug
Note that reseting the TI-99/4A clears the whole scratch-pad
including \>83C4, therefore a hook will not survive a reset. This was
intentional from Texas Instruments, to prevent a virus from remaining
active by just hooking the interrupt routine.

BUT... the code that clears \>83C4 is written in GPL... and the
GPL-interpreter allows interrupts! Thus, it is possible that an
interrupt occurs after a reset, before \>83C4 has been cleared, which
would leave the hooking program in control. This never occurs in case of
a hardware reset (e.g. turn the TI-99/4A off, then on) since the VDP
chip takes a long time to reset itself before it generates the first
interrupt. However, a software reset, such as pressing the \ key
does not reset the VDP, so an interrupt may occur in time for the hook
to survive the reset...

###   Timer interrupts

You can generate timed interrupts by loading a delay value in the
TMS9901 chip, via the CRU. When the timer fires, an interrupt is
generated. The TI-99/4A uses this feature for cassete operations: it
times the cassette player by reading a long string of leading zeros on
the tape, then uses this value to detect incoming data.

Unfortunately, it never occured to the TI engineers that somebody may
want to use this timer for anything else than cassette operations. As a
consequence, they wrote this part of the ISR in a way that makes it very
hard to use for us.

####  Timer ISR subroutine

As mentioned above, the ISR branches to the timer ISR subroutine if bit
3 (\>20) of byte \>83FC is set. The timer ISR then behaves as follows:

- It resets the interrupt flags in the TMS9901 (whithout checking it).
- Then it checks the word at. \>83E2-83E3,
- If \>83E2 is positive or zero, the ISR checks the instruction it is
  supposed to return to,
- If that instruction is a `JMP $-2` it returns by skipping it. (This
  allows to wait for an interrupt in a forever loop).
- If \>83E2 is negative, or if the main program is not in a forever
  loop, the ISR fetches an address from \>83EC and places it in R14,
  thereby overwritting the return address to the main program!!!
- It then performs a `RTWP, `which effectively results in branching to
  the address found in \>83EC with whatever workspace and status the
  main program had when the interrupt occured.

####  Hooking the timer interrupt

Now, how can we make use of that routine?

What we want to do is to place a delay value in the TMS9901 and start
the countdown. But first we must set the timer interrupt flag and
disable all other interrupts as the main ISR would mistake them for a
timer interrupt (remember, it only checks the flag to determine where
the interrupt came from).

* It expects a delay value in R0
* and a branch vector in R1 (or &gt;0000 to use a forever loop)
TIMEON SOCB @H20,@&gt;83FD        Set timer interrupt flag bit
       MOV  R12,@OLDR12        Preserve caller&#39;s R12
       CLR  R12                CRU base address &gt;0000
       SBZ  1                  Disable peripheral interrupts
       SBZ  2                  Disable VDP interrupts
       SBO  3                  Enable timer interrupts

Now we have to decide what to do when the interrupt occurs:

       JEQ  EVERLP
       SETO @&gt;83E2             Flag: we intend to branch elsewhere
       MOV  R1,@&gt;83EC          Set address where to go

Now, we can initialize the timer:

       INC  R0                 Set the clock bit to put TMS9901 in clock mode
       LDCR R0,15              Load the clock bit + the delay
       SBZ  0                  Back to normal mode: start timer
       MOV  @OLDR12,R12        Restore caller&#39;s R12
*      LIMI 2                  (optional, see below) Enable interrupts now
       B    *R11

If we decide to wait for the timer to fire, the caller should soon go
into a forever loop. If the timer fires before we do, the ISR will try
to branch to the address found in \>83EC (which may be a usefull
feature, by the way).

       LI   R0,&gt;1254           Value for 100 msec
       CLR  R1                 Let&#39;s use a forever loop
       BL   @TIMEON            Hook the timer interrupt
       LIMI 2                  Enable interrupts (if not done in TIMEON)
EVER   JMP  EVER               Stays here until timer fires
       BL   @TIMOFF            Escape loop: clean up timer setup
       ...

Alternatively, we could do something else until the timer interrupts us.

       LI   R0,&gt;1254           Value for 100 msec
       LI   R1,COMEBY          That&#39;s where we want to come
       BL   @TIMEON            Hook the timer interrupt
       LIMI 2                  Enable interupts (if not done by TIMEON)
HERE   INC  R0
       JMP  HERE               Do something (in this case, wait forever)`
* We&#39;ll land here when the timer fires
COMEBY LWPI WREGS             As we don&#39;t know for sure what the WS will be
       ...                    Do our stuff... and never return!

The only problem with the second solution is that the return address is
lost by the main ISR. Which means we cannot resume execution of the main
program after a timer interrupt. Well, we don't know the return address
for sure, but we can sort of guess it since interrupts must be enabled
by a `LIMI `instruction. Most programs place a couple of such
instructions is a frequenly visited loop:

RETRN  LIMI 0

If an interrupt occurs, it will be processed just after the `LIMI 2` is
executed. Thus, we know where to return: to the `LIMI 0` instruction. Of
course, this won't work if we have placed the `LIMI 2` in our
initialisation sequence: in this case the interrupt could occur
anywhere. Curse the stupid TI programmer who coded that horror of an
ISR!

Oh yes, and we should not forget to clean up our mess once we are done:

TIMOFF SZCB @H20,@&gt;83FD        Clear timer interrupt flag bit
       MOV  R12,@OLDR12        Preserve caller&#39;s R12
       CLR  R12                CRU base address &gt;0000
       SBO  1                  Enables peripheral interrupts
       SBO  2                  Enables VDP interrupts
       SBZ  3                  Disables timer interrupts
       MOV  @OLDR12,R12        Restore caller&#39;s R12
       B    *R11`
OLDR12 DATA 0                  Temporary buffer for caller&#39;s R12

See the disk controller page for an example on how to use the timer
interrupts to [check the rotation speed](disks.htm#Check%20speed) of a
disk drive.

See my page on the [TMS9901](tms9901.md) for more examples on how to
program this chip and play with interrupts.

Revision 2. 3/25/99 Polishing
Revision 3. 5/29/99 Tested & debugged examples
Revision 4. 5/30/99 Solved the UISR re-entrancy problem

Revision 5. 5/12/00. Corrected a few typos

Revision 6. 8/28/00. Discussion of CRU bit 1 reset by ISR.
Revision 7. 1/20/06. Corrected bug: flag bit is at \>83FD, not \>83FC.
[Back to the TI-99/4A Tech Pages](titechpages.md)
