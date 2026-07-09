# Writing DSRs for the USB-SM card

One problem when writing DSRs for the USB card is that there are a lot
of different USB devices, hence many different DSRs, possibly written by
many different programmers. The end user will want to load the DSRs
dealing which these devices he owns, in whatever order he fancies.
Therefore, a big one-for-all DSR program won't do it.

The solution I retained is to write DSRs in tagged-object format, i.e.
the DF80 files produced by the TI assembler. A special loader, modeled
on the one in the E/A cartridge, will be available for the user to load
the required DF80 file into the DSR memory. Yes it's slower, but so is
burning EEPROMs, and it's something the user won't do too often anyhow.

Another problem is that, even though the Flash-Memory is 8 megabytes in
size (or 4 megs if you were cheap) , it maps at \>4000-4FFF, so a page
is only 4K. If a programmer needs more space, there should be a way to
split the DSRs into several pages. However, the page numbers won't be
available at programming time, because they depend on the order in which
the user will load his DSRs.

The solution is easy: segment programming. The TI assembler carries all
the necessary commands to this end, but we never used them because the
TI loader doesn't handle them. Our special DSR loader does these
commands and can dispatch segments between pages and link them together.

The present page explains how to write DSRs so that they can be handled
by a common USB-DSR loader. It also describes several sets of low-level
routines used to access the most basic features: Smartmedia, USB
controller and memory. For specifics on how to program the various chips
on the USB-SM card, refer to [this page](usb2.md).

[Writing DSRs
](#writing%20DSRs)[Using the SRAM
](#Using%20the%20SRAM)[ISP1161 access routines
](#ISP1161%20routines)[SmartMedia access routines
](#SM%20routines)[Standard memory routines](#STDMEM)

###  Writing DSRs

First, you need the assembler that comes with the Editor/Assembler
cartridge, or an equivalent program that supports segment commands.

One usefull thing to do is to include a "COPY DSK1.USB/EQU" at the top
of your file. This will include a definition file in your program, which
contains a whole buch of EQUates and DEfines that will be usefull later.
It will save you the burden of typing these DEFs on top of each of your
programs.

To write distinct segments with the TI assembler, all you have to do is
to enclosed your code in between CSEG and CEND:

       ...           your code
       CEND

In the example above, MYSEG is the name of the segment.

There are a few special cases:

- CSEG with no name defaults to the name \$BLANK. I'm using it to load
  the DSRs entry points in page 0.
- Using DSEG and DEND instead, results in a segment called \$DATA. I'm
  using it to represent the SRAM memory.
- Using PSEG and PEND instead returns to the usual code segment, the
  assembler's default. It is not allowed here.

In addition to \$BLANK and \$DATA, the following segment names are
reserved:

- INFO This segment won't be loaded. It is used to provide the end user
  with some info about the DSRs contained in the file. It should appear
  at the top of your file.
- EXPORT This segment serves to make some of the routines in your DSRs
  available to other DSR files.
- MYSTUB This (small) segment will be loaded at the beginning of each
  page.
- STUB Same as above, except that your code will be added to the default
  stub segment, which contains the page switching routines.

So how do you jump from one segment to another? Well, that's what the
stub segment is here for: it contains a set of routines that allow you
to call routines across segments. For instance, to branch-and-link to
another segment, instead of using `BL @THERE`, you would call:

       DATA THERE,PG4SEG   

Where SGBL is the routine within the stub that switches pages and
performs a BL to the specified address. PG4SEG is a dummy label which
the loader will replace with the number of the page where THERE has been
loaded.

To return to the caller, rather than doing a simple RT, the routine
THERE should call:
Similarly, SGBLWP and SGRTWP are used to call and return from routines
requiring a context switch. Simple enough, no?

Lets now discuss the various segments one at a time:

[Entry segment
](#entry%20segment)[Data segment
](#Data%20segment)[Info segment
](#Info%20segment)[Export segment
](#Export%20segment)[Stub segment
](#Stub%20segment)[Your segment(s)](#Your%20segments)

[Segment order
](#Segment%20order)[EEPROM structure](#EEPROM%20structure)

###   The entry segment

All DSRs for all devices must have their entry points (i.e. their name)
in page 0. Since space is limited in page 0, the entry point should
consist in only a branching order to the page where your DSRs are to be
found.

To create an entry segment, just include its code in between a CSEG with
no segment name and a CEND. Note that the loader won't necessarily load
this segment as such, but may perform some code manipulation (e.g.
linking to other DSRs, etc).

Important: If the entry segment uses any REF label that is also
used by other segments, then the entry segment should appear before
those in your file.

There are four type of routines that can be found in typical DSR space:

- [Power-up routines](#Power-ups) are executed when the computer is
  reset.
- [Interrupt service routines](#ISR) (ISR) are executed when an
  interrupt occurs.
- [Device service routines](#DSRs) (DSR) are called by file operations
  (e.g. OPEN, CLOSE, SAVE, DELETE).
- [Subprograms](#Subprograms) can be called from (Extended) Basic with
  CALLs, or from assembly.

####  Device Service Routines

Strictly speaking, only these routines deserve the name "DSR": they let
the user interact with a device by the mean of file operations like
OPEN, SAVE or DELETE. All DSRs are called by name, some names you are
probably familier with: DSK1, PIO, CS1, etc.

Parameters are passed via a structure called PAB (peripheral access
block, see this page for details) located in VDP memory. The first byte
of the PAB indicates the type of operation desired: open, close, read,
write, restore, load, save, delete, scratch-record, or file status.
According to the operation, parameters such as file type, opening mode,
or file data will be passed to the DSR through the PAB. See [this
page](headers.md) for more information.

Another way to pass parameters to a DSR is to include them after the DSR
name, using a dot as a separator. A well known example is the RS232 DSR
which can be called with something like "RS232.BA=9600.DA=8.CR.LF". The
DSR name is "RS232", the rest are parameters interpreted by the DSR.

Be aware that each time a file operation occurs, all DSRs on all cards
will be scanned until the proper name is found. Thus, try to avoid
multiplying DSR names, since it may slow the system. Also, remember that
there is limited space in page 0 that should accomodate all DSR and
subprogram names for the USB-SM card. It is thus best to have a main DSR
and split it by the way of parameters. For instance, the USB controller
can handle 127 devices, so you may want to create 127 DSRs called "USB1"
through "USB127". This wouldn't be a good idea. It's much better to use
something like "USB.1" where there is only one DSR (called USB) and the
device number is passed after the dot.

You tell the loader that you wish to declare DSRs by defining the label
MYDSR. Since you can have more than one DSR, you should arrange them as
a chain and have MYDSR point to the first link in the chain. Each link
consists in two data words (aligned on word boundaries!): the address of
the next link, and the address of the entry point. These are followed
with the DSR name, which should be a string from 1 to 7 characters, with
a leading size byte. The last DSR in the chain will have \>0000 in its
link field.

To save space in page 0, a DSR entry point should consist only of a
branch order to the page where your DSRs are loaded. You can use the
routine SGB to this end, but not SGBL nor SGBLWP as these routines are
not present in page 0.Your target routinewill be entered with whatever
workspace the calling program is using (\>83E0 from Basic and Extended
Basic, anything from assembly). You can expect R12 to contain the card
DSR, and R11 to contain the return point. Make sure you preseve R11 when
returning from your DSR. In addition, R1 should contain the number of
times this DSR was already found, a feature used by DSRs that can be
present on multiple cards. You don't have to preserve R1 if your DSR
doesn't use this feature.

Once done, you should increment R11 by two and return by branching to
PG0RT (this is number zero, not letter o). This routine simply switches
back to page 0, restores R12, and performs a B \*R11. Optionally, you
can dispense with INCTing R11. In this case, the calling routine will
keep searching for DSRs with the same name. For this to work properly,
you should preserve R1 as well as R11.

Example of DSR entry points:

       REF   SGB,PG0RT,PG4SEG
*----------------------------------
* No-name segment: loaded in page 0
*----------------------------------
       CSEG
MYDSR  DATA  LK2        link to next DSR
       DATA  ENT1       entry point for this one
       BYTE  4          size byte
       TEXT  &#39;DSR1&#39;     DSR name
       EVEN             word-align (important!)
LK2    DATA  LK3
       DATA  ENT2
       BYTE  3
       TEXT  &#39;FOO&#39;
       EVEN
LK3    DATA  0          no more links
       DATA  ENT3
       BYTE  3
       TEXT  &#39;BAR&#39;
       EVEN
*
ENT1   BLWP  @SGB       entry point for &#39;DSR1&#39;
       DATA  MYDSR1     address
       DATA  PG4SEG     let loader provide the page #`
ENT2   BLWP  @SGB       entry point for &#39;FOO&#39;
       DATA  FOODSR,PG4SEG
ENT3   BLWP  @SGB       entry point for &#39;BAR&#39;
       DATA  BARDSR,PG4SEG
       CEND
*----------------------------------
* Our first segment: loaded wherever it fits
*----------------------------------
       CSEG  MYSEG1
MYDSR1 ...              code for &#39;DSR1&#39;
SK1    INCT R11         no more scanning
       BLWP @PG0RT      return to page 0 and do B *R11
FOODSR ...              code for &#39;FOO&#39;
       JMP SK1          return as above
BARDSR ...              code for &#39;BAR&#39;
       JMP SK1          return as above
       CEND
       END

####  Subprograms

Subprograms resemble DSRs, in that there are called by name. However,
they are not called by file operations, but with a dedicated
instruction, CALL in Basic. They can also be called from assembly with
DSRLNK, the same routine that calls DSRs.

A big difference with DSRs is in the way parameters are passed, as there
is no PAB. For subprograms called from Basic, you must parse the Basic
statement to find parameters in the parenthesis following CALL. This is
quite tricky to do, as numeric parameters can be passed via complicated
math expressions, and string parameters can include things like SEG\$
and CHR\$. The situation is much simpler in assembly, where parameters
are generally passed in CPU memory, at address \>834A and above.

It is possible to append parameters to the subprogram name (e.g. CALL
MYSUB.A=1.NOLOAD) but be aware that these long names may not be properly
saved within a Basic program, so it's only usefull for subprograms
called interactively, from the keyboard.

Your subprogram will be entered with the caller's workspace, and you can
expect R12 to contain the card's CRU. Do not loose R11, since it
contains the return point. Just like with DSRs, R1 can optionally
contain the number of times this particular subprogram name was found.

Once your subprogram is done, it should increment R11 by two and return
with B @PG0RT, which returns to page 0 and performs a B \*R11.
Optionally, you can dispense with incrementing R11, which will cause the
scanning routine to keep searching cards for subprogram with that name.
In this case, make sure you also preserve R1.

Since you might want to define more than one subprogram, you should
always arrange them in a chain (even if there is only one). Then define
the label MYCALL, and have it point to the first link in the chain. The
structure of the chain is the same as for DSRs.

Here is an example:

       REF   SGB,PG0RT,PG4SEG
*----------------------------------
* No-name segment: loaded in page 0
*----------------------------------
       CSEG
MYCALL DATA  LK2        link to next subprogram
       DATA  ENT1       entry point for this one
       BYTE  4          size byte
       TEXT  &#39;THIS&#39;     subprogram name
       EVEN             word-align (important!)
LK2    DATA  0          no more links
       DATA  ENT2
       BYTE  4
       TEXT  &#39;THAT&#39;
       EVEN
*
ENT1   BLWP  @SGB       entry point for &#39;THIS&#39;
       DATA  THIS1      address
       DATA  PG4SEG     let loader provide the page #
ENT2   BLWP  @SGB       entry point for &#39;THAT&#39;
       DATA  THAT1,PG4SEG
       CEND
*----------------------------------
* Our first segment: loaded wherever it fits
*----------------------------------
       CSEG  MYSEG1
THIS1  ...              code for &#39;THIS&#39;
SK1    INCT R11         no more scanning
       BLWP @PG0RT      return to page 0 and do B *R11
THAT1  ...             code for &#39;FOO&#39;
       JMP SK1         return as above
       CEND
       END  

####  Power-up routines

One can distinguish three types of power-up situations:

- Software reset: the computer reboots itself. E.g. the \ key was
  pressed, or BLWP @\>0000 was executed.
- Console power-up: the console power was turned off and back on. This
  sends a signal on the Reset\* line, which causes a harware reset for
  any chip hooked to it. For instance, all CRU bits will be reset to 0,
  and the microprocessor automatically executes a software reset.
  However, the RAM in the PE-box should remain unchanged, baring any
  unexpected glitch.
- PE-box power-up: the power was turned off and back on for the PE-box.
  This will wipe off the contents of the SRAM and scramble all CRU bits.
  A PE-box reset in itself will not call the power-up routine. However,
  since the PE-box is generally turned on just before the console, one
  can reasonnably expect that a console power-up (and thereby a software
  reset) will follow.

The master power-up routine in the TI-99/4A console does not distinguish
the three types of power-up. However, the power-up routine in the USB-SM
card DSRs does, and it's the one that will call your own routine. This
will let you write different initialisation routines according to the
type of power-up that occured.

Be carefull that, no matter what type of reset occured, only one type of
power-up routine will be called. Generally, you should write your
power-up routine in a cascading fashion, with three entry points:

    PEBOX  ...        Entry point for PE-Box power-up
           ...        Code unique to PE-box reset
    CONS   ...        Entry point for console power-up
           ...        Code unique to console reset
    SOFT   ...        Entry point for software reset
           ...        Code unique to software reset
           RT

To inform the loader that you intend to load a power-up routine, you
should define the label MYPWUP in your program, and have it point at the
first link of your power-up routine (even though there is no need for
more than one routine, you could have several, therefore a chain
structure is used, similar to the one we saw for DSRs, but without
names). From there, you can either branch directly to the page in which
your DSRs are loaded, or perform a quick test to determine which type of
reset occurred. Before it calls your routine, the card's power-up
routine will set a flag in R0 with the following values:

R0 = \>FFFF for PE-box reset (test with JLT)
R0 = \>0000 for console reset (test with JEQ)
R0 = \>0001 for software reset (test with JGT)

N.B. The flag in placed in R0 just before entering your routine, so you
don't need to add a MOV R0,R0 to test it.

Example of power-up entry point:

       REF   SGB,USBRT,PG4SEG
*----------------------------------
* No-name segment: loaded in page 0
*----------------------------------
       CSEG
MYPWUP DATA  0          no more links
       DATA  MYP1       entry point`
MYP1   JEQ   SK1        trap software reset (example)
       BLWP  @SGB       call our routine for others power-up types
       DATA  MYPW1      address of our routine
       DATA  PG4SEG     let loader fill in page #
SK1    B     *R11       return doing nothing
       CEND
*----------------------------------
* Our first segment: loaded wherever it fits
*----------------------------------
       CSEG  MYSEG1
MYPW1  JLT   SK2        trap console reset
       ...              code for PE-box reset
SK2    ...              code for console reset
       BLWP  @PG0RT     return to page 0
       CEND
       END

In this example, a software reset necessitates no special action, so
that kind of power-up is trapped immediately. The other two types of
reset require special handling, so we are branching to our routine in
another segment. Note the branching is performed by calling the routine
SGB with two data words: the address and the page number. Since we don't
know what the page number will be, we used the pseudo-label PG4SEG wich
instructs the loader to provide the page number for the address found in
the previous word (here MYPW1).

The routine is entered with the GPL workspace: \>83E0. You can expect to
find your card CRU in R12. The entry routine has saved R12 through R15
and will restore them once our routine returns.

Once done, your routine must call PG0RT, which switches back to page 0,
and performs a RT. This returns to the USB entry routine, which will
call power-up routines for others DSRs, if any.

####  Interrupt service routine

These routines are called every time an interrupt occurs. An entry
routine will make sure that the interrupt indeed comes from the USB-SM
card, then call the ISR for every DSR loaded.

To provide an ISR, you should define the label MYISR and have it point
to the first link in your ISR chain. Again, even though there is
generally only one ISR, a chain structure is provided in case you wanted
to use more than one routine. Normally, the entry point will immediately
branch to your ISR routine in another segment. However, you may want to
first test CRU input bits 0 and 1, to determine whether the interrupt
came from the USB host controller or from the USB device controller.

It is your responsability to determine whether the interrupt was
generated by your device or by another one. If it's not your device,
your routine should branch immediately to PG0RT, which will return to
page 0 and call the ISRs for other DSRs. If the interrupt did come from
your device, you must clear the interrupt condition and increment R11 by
two before branching to PG0RT. This will prevent the stubs from calling
further ISRs.

Again, the ISR is entered with workspace \>83E0, and R12 should contain
the card's CRU. The card's ISR routine has saved R12 through R15 and
will restore them upon return from your ISR.

Example of ISR entry point:

       REF   SGB,USBRT,PG4SEG
*----------------------------------
* No-name segment: loaded in page 0
*----------------------------------
       CSEG
MYISR  DATA  0          next link: no more
       DATA MYISR1      entry point
MYISR1 TB    0          host controller interrupt?
       JNE   SK1        no: don&#39;t do anything
       BLWP  @SGB       call our routine for host controller ints
       DATA  MYISR2     address of our routine
       DATA  PG4SEG     let loader fill in page #
SK1    B     *R11       return doing nothing
       CEND
*----------------------------------
* Our first segment: loaded wherever it fits
*----------------------------------
       CSEG  MYSEG1
MYISR2 ...              check if interrupt came from our device
       JNE  NOTUS       nope
       ...              yes: handle it, clear interrupt
       INCT R11         no more ISR scanning
NOTUS  BLWP @PG0RT      return to page 0
       CEND
       END  

###  The information segment

As mentionned above, the segment named 'INFO' will not be loaded. It is
meant for directory listing programs, so the user can have an idea of
what the DSR is used for. For faster listing, it is better if the
information segment appears at the top of your file. The INFO segment
can contain upto 4 information fields, separated by \>00 bytes.

1.  The first field should consists in a maximum of 16 characters. It
    describes the device this DSR is meant for. Currently, the device
    name should be 'USB-SM 1.5'.
2.  The second field is meant for directory listing with one line per
    file: it should contain a 28-char description of the DSRs.
3.  The third field should consist in a maximum of 24 lines of 32
    characters. You can use byte \>0D to move to the next line, or just
    pad each line with spaces. This field is meant to provide one full
    screen of information about your DSR.
4.  The last field is optional. It is free for you to use and has no
    required format.

Example of information segment:

       TEXT &#39;USB-SM 1.5&#39;                      device name
       BYTE &gt;00                               field separator
       TEXT &#39;Sample DSR. No real use.&#39;        one-line description
       BYTE &gt;00
       TEXT &#39;       Sample DSR&#39;               full-screen description
       BYTE &gt;0D                               next line
       TEXT &#39;       ----------&#39;               underlining the title
       BYTE &gt;0D,&gt;0D                           skip 2 lines
       TEXT &#39;This is not really a DSR. It`s  &#39;
       TEXT &#39;here to demonstrate the use of  &#39;
       TEXT &#39;the INFO segment.               &#39;
       BYTE 0
       CEND                                    no third field, here 

###  The export segment

This segment is here so that you can declare some of your routines for
use by other DSRs (i.e. files loaded at a different time). The loader
will enter these into a symbol table in the EEPROM. Once this is done,
any DSR can refer to your routines by means of a REF statement. When the
loader encounters a REF, it first checks if the label is one of the
predefined symbols (e.g. PG4SEG). If not, it searches the export table,
in the DSR EEPROM.

Each entry in the export segment has the following format:

- A six-characters label name, padded with spaces if necessary.
- The address of the routine.
- The page number where the routine is to be found (use PG4SEG to let
  the loader fill this in)

Important: You must stick to this format very strictly and not
add anything else to the export segment, otherwise you may confuse the
loader and totally disable the REF resolution mechanism!

Here is an example of such a segment:

       TEXT &#39;VSBW  &#39;
       DATA VSBW1
       DATA PG4SEG
       TEXT &#39;DUMBOS&#39;
       DATA DUMBOS
       DATA PG4SEG
       CEND

Note that the DSR file exporting the routines must be loaded before the
one referencing to them. So there is no way you can have two DSR files
cross-referencing each other: you would get a 'label not found' error
when loading the first one.

####  Workspace issues

Most of the time, the exported routine will have its workspace within
the SRAM, generally in page 0. Thus there is a danger that it would
conflict with part of your DSEG segment. To prevent this kind of
problem, follow these rules:

- Exported routines that do not call other exports (a.k.a. level 1
  routines) should use L1WS as a workspace. L1WS is defined as \>5FD0 in
  the file DSK1.USB/EQU.
- Exported routines that call only level 1 routines are level 2 routines
  and should use L2WS (\>5FB0) as a workspace.
- Exported routines that call one or more level 2 routines, should use
  L3WS as a workspace (\>5F90).
- Etc, upto L9WS (\>5ED0).

###  The stub segment(s)

A stub is a short segment that is loaded at the beginning of every
EEPROM page. By default, the loader will install a minimal stub
containing the page switching routines. If you so wish, you can append
your own code to the existing stub, replace it with a stub or yours, or
not use any stub at all.

To append code to the existing stub, just include it within a segment
called STUB. The loader will add your code at the end of the default
stub. To create your own stub, make up a segment called MYSTUB. You can
have both a STUB and a MYSTUB segment in your program.

So that the loader knows which stub is to be used (if any) with each
segment, you should set the proper segment flags (see
[below](#Segment%20flags)):

- MYSTUB causes the segment to be loaded into a page beginning with the
  MYSTUB segment.
- NOSTUB frees the segment from stub requirements: it can be loaded in a
  page containing any stub, or none at all.
- If neither flag is set, the segment will be loaded into a page
  containing the STUB segment.

If you are not using the default STUB segment, you can still branch to
your segments from another page, by using the page switching routines
described [below](#Jumping%20pages). However, to return to the caller,
or to call other segments, you will need to implement your own page
switching routines.

Important note: If your stub segment(s) contain any REF label
shared by another segment, the stub segment should appear last in your
file. If you're using both MYTSUB and STUB, then STUB should appear
last.

###  Writing your own segment(s)

[Segment flags
](#Segment%20flags)[Jumping pages
](#Jumping%20pages)[Fetching inline data
](#Fetching%20data)[Shorter syntax](#Short%20syntax)

#### Segment flags

You can (but don't have to) set several loading flags for each of your
segment. To pass the flags to the loader, DEFine a label with the same
name as the segment. Then EQUate this label to the sum of all flag
values that you wish to set. If you find the values hard to remember,
you may want to COPY the file USB/EQU at the top of your file. It
contains several DEF and EQU needed to communicate with the loader. In
particular, each segment flag has a name associated with its values:

    NEWPG   EQU >0001
    ENDPG   EQU >0002
    FULLPG  EQU >0003
    MYPG    EQU >0004
    NOSTUB  EQU >0008
    MYSTUB  EQU >0010
    NOLINK  EQU >0080

**NEWPG** This flag causes the segment to be loaded at a fresh ROM page,
which does not contain any other segment yet. If room is left after the
segment is loaded, another segment (from you file, or from another DSR)
may be loaded in the same page later on.

**ENDPG** This flag reserves the remainder of the page for your segment.
It ensures nothing will be loaded after it, neither from your file, nor
from subsequent loading sessions.

**FULLPG** This flag reserves the full page for your segment. As you may
have noticed, it's actually a combination of NEWPG and ENDPG: so it
starts with a fresh page, and no other segment will be loaded in the
same page.

**MYPG** This is a somewhat mitigated version of NEWPG: your segment
will be loaded into a page that is currently empty (from previous
loading sessions) but into which other segments of yours may have been
assigned. In other words, your segment won't mingle with segments from
another DSR, but may cohabitate with some other segment of yours.

**NOSTUB** This flag relieves the loader from the obligation inserting a
stub at the beginning of the page your segment is loaded in. Such a
segment can be loaded in a page containing any stub, or no stub at all
(e.g. if it's an empty page).

**MYSTUB** This flag informs the loader that it should use your own stub
instead of the default one. Your stub must be in a segment called
MYSTUB. If this segment is not found, the loader will use its default
stub, or no stub a all if the NOSTUB flag is also set (MYSTUB has
priority over NOSTUB). Again, only segments with the MYSTUB flag will be
loaded together in the same page.

**NOLINK** prevents the loader from incorporating the segments within a
chained list. Normally, each segment is preceded with one word of data,
a pointer to the next segment (or free space). To find available space,
the loader walks this chain starting at \>4000. If you don't want such a
structure to be installed, e.g. because you need to place your own data
at \>4000, or because your segment is exactly 4K, you can set the NOLINK
flag. Be aware that it will make that page unavailable for loading
further segments, from other DSR files.

####  Jumping pages

If your DSRs are smaller than 4K, they can fit within a single page. If
you need more than one page, you will need a way to call a routine
located in a different page. This is done by calling the page-switching
routines SGB, SGBL and SGBLWP. We have discussed SGB above, SGBL and
SGBLWP have a similar syntax and are used to call procedures with BL and
BLWP respectively.

To call a routine that you would normally enter with BL, do:

       DATA THERE
       DATA PG4SEG           

The first data word contains the address of the routine you want to call
.

The second data word should contain the desired page number. As you
don't know this at assembly time, it is better to replace it with the
pseudo-label PG4SEG, which instructs the loader to use the page number
for the routine that immediately follows.

SGBL saves the return address in R11, and the current page number in
R10. You must preserve both registers to properly return to the caller.
The return is achieved by calling:
which performs the equivalent of an RT (i.e. B \*R11), after switching
back to the page specified in R10.
To call a routine that you would normally enter with BLWP, do:

       DATA FOOBAR
       DATA PG4SEG  

The first data word should be the address (in the destination page) of
the BLWP vectors: workspace and start address.

The second data word should contain the desired page number. As you
don't know this at assembly time, it is better to replace it with the
pseudo-label PG4SEG, which instructs the loader to use the page number
for the routine that immediately follows.

SGBLWP saves the workspace in R13, the return address in R14, the status
in R15 and the current page number in R10. To properly return to the
caller, you must also preserve R10. Then simply call the SGRTWP routine,
which performs the moral equivalent of a RTWP, after switching to the
page found in R10:

The trouble with BL is that you must preserve R11 before you can take
another BL from within the called procedure. If you find this annoying,
you're probably not too entranced at the idea of having to preserve yet
another register, R10. In this case, SGBLX is for you!

The SGBLX routine works just like SGBL, except that it saves R10 and R11
in an internal stack, so you don't have to worry about preserving these
registers. To return to the caller, just use SGRTX instead of SGRT. In
fact, unpon entering the target routine, R11 will point at a BLWP @SGRTX
instruction, so you can return with B \*R11 if you want to.

N.B. If you need to retrieve inline parameters, use ATR11X instead of
ATR11.

       DATA THAT,PG4SEG
       JMP  THERE`
* In another segment:
THERE ...
      BLWP @ATR11X         skip the JMP upon return
      BLWP @SGRTX          or B *R11, if R11 was conserved 

Please note that the internal stack is circular and 8-level deep. Which
means that if you nest 9 calls, unlikely as it is, the return parameters
for the nineth call will ovewrite those of the first call. Also, the
stack is located in the current SRAM page: if you switch RAM pages you
will loose all return points.

Because R11 does not contain the return address, you cannot manipulate
it to change the return point. If all you need to do is to skip a jump
upon return, you can use ATR11X as illustrated above. Otherwise, you'd
need to change the return point inside the stack, which is a bit
complicated. Your best bet is probably to pop the return parameters off
the satck, and to return directly to the desired address with SGB. This
strategy is illustrated below:

THERE ...
      DEC  @STAKPT          decrement stack pointer
      SZCB @HFFE3,@STAKPT   round to lowest multiple of 4, circularize
      BLWP @SGB             &quot;return&quot; by branching to the point we want
      DATA ALTERN,PG4SEG  `
HFFE3 DATA &gt;FFE3                    

STAKPT is a number from \>0000 to \>001C which keeps track of the
current entry in the stack. When it reaches zero, it rolls over to
\>001C and conversely. Thus the SZCB @HFFE3, which is the same as an
ANDI \>001C.

####  Fetching inline data

It's common practice to pass parameters to a routine by inserting data
words after the call:

       DATA &gt;1234,THAT      

The routine THIS can then recover the data with two `MOV *R11+,R0`
instructions. A routine called with BLWP would do the same with
`MOV *R14+,R0.` Obviously, this is not going to work with routines
located in different pages! To solve this problem, you can use the
routines ATR11 and ATR14.

       MOV  R0,...            MOV  R0,...      

Each routine returns with R0 containing one word of data fetched from
the page specified in R10, at the address specified in R11 or R14,
respectively. The relevant register is incremented by two, so you can
call the routine again, if more than one word of data needs to be
fetched. Make sure you fetch all data words before returning to the
caller.

A more general routine, is GETDAT (as a matter of fact, ATR11 and ATR14
are just different entry point into GETDAT). It is probably of little
use since most of your data will be in SRAM anyhow, but it may be used
to fetch a constant, for instance an entry from a pointer table.

       MOV  @MYPAGE,R2
       BLWP @GETDAT
       MOV  R0,... 

GETDAT retrieves data from the EEPROM page specified in R2, at the
address specified in R1. One word of data is fetched and returned in R0.

A convenient way to obtain the page number for R2, is to take it from a
data structure containing the pseudo-label PG4SEG:
The loader will replace PG4SEG with the number of the page where the
label MYDATA is located. Note that you don't need to use the very same
label as for the call to GETDAT, as long as it's located in the same
segment.

####  Shortened syntax

If you get tired with typing all these calls to SGBLWP, followed with a
data statement containing the routine name and a PG4SEG label, you can
use the following trick:

TOSLT1 DATA L1WR,TOSLT2            vectors for BLWP
TOSLT2 BLWP @SGB                   call the routine
       DATA TOSLOT,PG4SEG`
* Now, instead of writing...
       BLWP @SGBLWP
       DATA TOSLOT,PG4SEG
       DATA MYSLOT`
* ...you can use the shorter form:
       BLWP @TOSLT1
       DATA MYSLOT

This saves you 2 memory words per call, with a one-time overhead of 5
words. So it's worth using if you have more than 2 calls to the same
procedure. It also makes your program easier to read...

###  The data segment

Since DSRs reside in ROM, you should not include any data within your
program, except for constants. For variables, you can use the workspace
with which the routine is entered. Be aware however, that R11 (and
sometimes R1 and R12) must be preserved for proper return, and so are
R13-R15 if the workspace is \>83E0 (they are used by the system).
Therefore, it's generally better to immediately perform a BLWP to your
own workspace, possibly residing in the SRAM.

If you need more space for your data, you can of course use the SRAM
chips on board the card. These map at \>5000-5FFF, as 256 pages of 4
kbytes (although the USB controller maps at \>5FF0-5FFF when CRU bit 1
is set). Remember that the SRAM is not battery backed, so the its
content will be lost upon power-up. Incidently, this means that you
cannot load pre-initialized data. If you need to initialize variables,
have a copy of the initial state in the EEPROM and copy it to the SRAM
when needed (e.g. upon power-up, or when first entering your DSR).

To tell the loader that you mean to use data inside the SRAM space, just
include any data between a DSEG and a DEND instruction. These won't
actually be loaded (since, once again, the SRAM won't retain it beyond
the next power off), but the loader will assign them an address and
patch these addresses within your program.

Here is an example, in which variables are initialized from a copy in
EEPROM.

* Code segment: in EEPROM
*----------------------------------
       CSEG &#39;FOOBAR&#39;
INIVAR LI   R0,ROMVAR        optional: init variables
       LI   R1,HITS
       LI   R2,VAREND-ROMVAR
LP1    MOVB *R0+,*R1+        copy ROM to RAM
       DEC  R2
       JNE  LP1
       ...
DUMMY  MOVB @HERE,@THERE     use variables (dummy example)
       DEC  @HITS
       LI   R1,TITLE
       MOV  @SIZE,R2
       BLWP @VMBW
       ...
ROMVAR DATA 5                initial values to be copied in RAM
       BYTE &gt;AA
       BYTE &gt;55
       DATA 14
       TEXT &#39;This is a test&#39;
VAREND
       CDEND
*
*----------------------------------
* Data segment (assigned to SRAM)
*----------------------------------
       DSEG
HITS   DATA 0                our variables
HERE   BYTE 0
THERE  BYTE 0
SIZE   DATA 0
TITLE  BSS  14
       DEND
  

The DSEG segment will always use RAM page 0, which is shared by all
other DSRs. Thus, you should not expect data in your DSEG segment to
remain intact between two calls of your DSRs. If you need a private
memory area (e.g. to store opened file information), use the MALLOC
subroutine: the memory it assigns will remain yours until you free it,
or until the computer is reset. You can save the address obtained from
MALLOC into a private location using TOSLOT. See
[below](#private%20RAM%20slot) for details.

###  Segment order in the file

I recommend that your segments appear in the following order, within
your DF80 file:

- INFO
- no-name (i.e. entry segment)
- All other segments (including EXPORT)
- MYSTUB
- STUB
- The data segment can be anywhere, even chopped into pieces throughout
  the source file(s).

This is needed for proper resolution of the REF chains. If your segment
does not share any REF with another segment, then you can load it
anywhere.

For the same reason, it's better if each segment is kept as a single
block. You could chop them into pieces and interlace them, but that may
prevent the loader from resolving all REF labels. The key here is that,
if two segments uses the same REF label, then all occurences of this
label should appear in the same order as the segment declarations.

#### Technical explanation

This is due to the way the assembler organises the REFed labels: as a
chain starting at the end of the file (even though the REF statement
itself may appear at the beginning of the source file). Each occurence
of a REFed label points to the previous one, and the chain ends with a
null link at the first occurence. Segment structure is ignored for that
purpose. Here are three examples:

       CSEG 'SEG1'             CSEG 'SEGA'             CSEG 'MYSEG1'
       DATA ANYREF<-,          DATA ANYREF<-,          CEND
    ,->DATA ANYREF--'          CEND         |          CSEG 'MYSEG2'
    |  CEND                    CSEG 'SEGB'  |          DATA  ANYREF<-,
    |  CSEG 'SEG2'          ,->DATA ANYREF--'       ,->DATA  ANYREF--'
    '--DATA ANYREF<-,       |  CEND                 |  CEND
       CEND         |       |  CSEG 'SEGA'          |  CSEG 'MYSEG1'
       CSEG 'SEG1'  |       '--DATA ANYREF<-,       '--DATA ANYREF<-,
       CEND         |          CEND         |          CEND         |
       REF  ANYREF--'          REF  ANYREF--'          REF  ANYREF--'
    *  This is ok           * This won't work       * Neither will this

Because data within an EEPROM cannot be modified, the loader must build
segments one at a time in the memory expansion, starting with the last
one. More precisely: first STUB, then MYSTUB, then any other segments in
reverse order of apparition, then the no-name entry segment (INFO and
the data segments are never loaded).

It ensues that the loader cannot walk a REF chain if it goes through a
segment that hasn't been loaded yet. In the second example above, the
loader processes SEGB first, so it cannot walk the ANYREF chain through
SEGA. And when SEGA is built, the loader cannot walk "through" SEGB to
follow the links. A similar, less obvious, situation occurs in the
rightmost example, where MYSEG2 is process first (because its
CSEG appears after MYSEG1's), at a time when the REF chain hasn't yet
been walked through MYSEG1.

In conclusion, to avoid that kind of problem, don't split your segments
and arrange them as described above.

###  Internal EEPROM structure

You don't need to read this chapter, unless you are curious to know how
the loader organizes segments within the EEPROM.

Each segment is preceeded with a pointer, i.e. a data word containing
the next free address after the segment. This address may in turn
contain a pointer to the next one, etc.

    A4000  DATA PT1                 Address >4000
           ....                     First segment (generally STUB)
    PT1    DATA PT2
            ...                     Second segment
    PT2    DATA >FFFF               End-of-chain mark
           ...                      Third segment
           DATA >FFFF, FFFF,...     Free space

The exception to this rule is page 0, in which address \>4000 is needed
for the standard ROM header. Thus, the first link in page 0 is located
at \>400E instead.

Other important locations in page 0 are:

- \>4014 Address of the internal list of power-up routines, to be called
  by the card's master power-up routine.
- \>4016 Address of the internal list of ISRs, to be called by the
  card's master ISR.
- \>4018 Number of the first EXPORT page.
- \>401A Number of the first removal information page.

The exports page begins with a pointer reserving the whole page (i.e.
pointing at \>4FFE) and the page number. Starting at \>4004, exports are
loaded exactly as defined in your EXPORT segment, with no separation
between subsequent loading sessions. The first \>FFFF word marks the end
of the list. If the page gets full, the list will end with a \>0000
word, followed with the number of the next exports page.

Removal information is saved in the same manner. It begins with four
data words: a pointer to the next chunk of removal info, the numbers of
the last export before yours, and the number of your last export (if
these numbers are identical, you defined no exports), and finally a
pointer to a list exports REFed by your program.

Then it lists the page number, address, and size for each segment in
your program. The list always begins with the data segment and the
no-name entry segment, even if they were not used in your program.

Finally, the loader lists the ordinal number of each export used by a
REF in your file. The list ends with a \>8000 endmark. It has a maximum
size of 256 entries, if your program REFed more exports than this, the
endmark will contain the number of unlisted REFs, in the form \>8xxx.

This information can be used by a DSR management program to safely
remove a DSR file from memory. By comparing your export numbers with the
REF lists of other DSRs, it can determine whether removing your DSR
would compromise others. The segment table lets us determine which
memory page can be reclaimed, and possibly erased, by removing your DSR.

Example of removal information:

    A4000 DATA >5000              Reserve full page
          DATA >0002              Page number

          DATA PT1                Info for next DSR loaded
          DATA >0012              Number of the last export before ours
          DATA >0014              Number of our last export (i.e. we had two)
          DATA XP1                Pointer to list of REFed exports

          DATA >0000              SRAM page for DSEG segment
          DATA >5100              Address of DSEG segment
          DATA >0124              Size of DSEG

          DATA >0000              ROM page for entry segment (always 0)
          DATA >4530              Address of entry segment
          DATA >0064              Size of entry segment

          DATA >0042              Page where one of our segments is
          DATA >4106              Address of this segment
          DATA >0AFE              Its size

          DATA >0025              Page number for another segment
          DATA >4804              Its address
          DATA >0106              Its size

    XP1   DATA >0003,>0017,>0025  List of exports REFed by our DSR
          DATA >8000              Endmark

    PT1   DATA >FFFF,>FFFF...     Free space (or >0000,next_page)

###  Using the SRAM

[Data segment DSEG
](#DSEG%20in%20SRAM)[Malloc, Free, Ramcpy
](#malloc&free)[Private memory slot
](#private%20RAM%20slot)[Switching pages
](#Switch%20SRAM%20page)[SRAM internal structure](#SRAM%20stubs)

#### Data segment

When your DSRs are entered, you can expect SRAM page 0 to be on. All
data in your data segment will map to this page. This is generally where
you will place your workspace, unless you elect to use the caller's
workspace, i.e. the one in use when your DSR is entered. Which is a bit
tricky because you don't know which registers the calling routine
expects to remain intact. So, as a general rule, it's safer use your own
workspace.

A simple way to switch workspace upon DSR entry is the following:

       ...               Your DSR code
       BL   @OLDWS       Trick to retrieve old workspace
       INCT R11          Return with no error (example)
       B    *R11         Exit DSR
*
MYWS   DATA WREGS,HERE   Vectors
HERE   B    *R14         Return with new workspace
*
OLDWS MOV   R11,R14      Copy return point
      RTWP               Return with old workspace           

The botom of SRAM page 0 is reserved for use by the page-switching
routines: they have their workspace at \>5004 (level 0 workspace),
followed with a stack for return addresses, and some reserved variables.
Thus, your data segment will generally begin at \>5040.

The end of SRAM page 0, from \>5ED0 on, is used by exported routines for
their workspaces. See [workspace issues](#WS%20issues), above. And of
course the USB controller mapsat \>5FF0-5FFF if CRU bit 1 is set. Thus,
the maximum size of your data segment is \>0E90, or 3728 bytes.

####  Malloc, Free, Ramcpy

If you need more data space, you can use the dedicated procedure MALLOC
to reserve it. Don't forget to free this space after use, with the
procedure FREE. This method is especially convenient to reserve large
chunks of memory.

**MALLOC** takes one argument in R0: the number of bytes you need. It
returns with a page number in R0 (so you can select the page with RAMPG)
and with the address of the first byte in R1. The Eq bit will be set
upon return if the memory could be allocated. If the Eq is not set (test
it with JEQ or JNE) something went wrong. Very likely, you're running
out of RAM pages, or you asked for too big a chunk (max size is a bit
under 4 kbytes).

**FREE** takes two argument: the page number in R0 and in R1 the address
of the first byte to free, both as received from MALLOC. It will free
the number of bytes that were allocated when MALLOC was called.

Here's an example:

       LI   R0,1024        reserve 1024 bytes
       BLWP @SGBLWP        call routine in another segment
       DATA MALLOC,PG4SEG
       JNE  ERROR          trap errors
       MOV  R1,R6          save pointer
       ...                 use memory
       MOV  R6,R1          retrieve pointer
       BLWP @SGBLWP
       DATA FREE,PG4SEG    free the memory
       CEND

**RAMCPY** is a procedure that you can used once you obtained buffer
space with MALLOC, to copy data to and from it. Its parameters are the
following:

R0: source page
R1: source address
R2: number of bytes to copy
R3: destination page
R4: destination address

Obviously, R0 and R3 are only relevant if the corresponding address is
in the SRAM space, i.e. \>5000-5FFF. If it's elsewhere, the page number
is ignored. Optionally, you can use \>FFFF as a page number to specify
the current SRAM page.

      LI   R1,MYDATA      at this address
      LI   R2,512         512 bytes
      SETO R3             into current page
      LI   R4,MYBUF       at this address
      BLWP @SGBLWP
      DATA RAMCPY,PG4SEG  copy          

Technical note: All three procedures use a workspace located at \>5FD0,
then switch to another worskspace in the scratch-pad, at \>8300 so they
can switch pages. The content of the scratch-pad is saved, and will be
restored upon return. However, this means that you should not use RAMCPY
with an address in the range \>8300-831E, nor in the range \>5FD4-5FE2.

####  Private RAM

Sometimes, it is necessary to keep information in memory between two
calls of a DSR: for instance, you could store the current sector being
accessed by an open file. However, because page 0 is shared by all DSRs,
you cannot expect its contents to remain intect between two calls of
your DSR: another DSR may be called in the meantime, and could overwrite
your data.

If you need a permanent memory area, you can obtain one with MALLOC: the
allocated memory won't be touched unless you free it, or the computer is
rebooted. However, you still have a problem: how do you remember the
location of that memory area? This is where your private memory slot
comes in.

By REFerencing the label MYSLOT, you will cause the loader to assign you
a unique slot of 4 bytes in RAM page 1. The address of which will be
provided in MYSLOT. Four bytes is not much, but it's enough to save the
page number and address returned by MALLOC (actually, since the page
number is 2-255, you even have one spare byte). To this end, you can use
the dedicated routines TOSLOT and ATSLOT.

**TOSLOT** saves R0 and R1 into your private slot. It must be followed
with a data word containing a reference to MYSLOT.

**ATSLOT** retrieves the contents of your private slot into R0 and R1.
The call must also be followed with MYSLOT. Upon power-up, you can
expect your slot to contain \>0000, \>0000.

Here's an exemple of how to use this feature.

* This is performed upon power-up, or the first
* time your DSR is entered: it reserves some memory for later
      LI   0,&gt;0678        number of bytes required
      BLWP @SGBLWP
      DATA MALLOC,PG4SEG  get private memory area
      JNE  ERROR
      BLWP @SGBLWP
      DATA TOSLOT,PG4SEG  save its location
      DATA MYSLOT         data for TOSLOT  `
* Any time your DSR is entered, it can find out
* where the reserved memory is located
      BLWP @SGBLWP
      DATA ATSLOT,PG4SEG  retrieve R0 and R1
      DATA MYSLOT
      ...                use them, for instance with RAMCPY`
* This may be done once you don&#39;t need the memory any more
      BLWP @SGBLWP
      DATA ATSLOT,PG4SEG  retrieve R0 and R1
      DATA MYSLOT
      BLWP @SGBLWP
      DATA FREE,PG4SEG    free our memory area
      CLR  R0
      CLR  R1
      BLWP @SGBLWP
      DATA TOSLOT,PG4SEG  empty our slot
      DATA MYSLOT

MALLOC, FREE, RAMCPY, TOSLOT and ATSLOT are part of a separate segment
called STDMEM, to be found in the file MEMSUBS/O. This file should be
loaded with the DSR loader, and will define several exports that you can
REFered to within your program. Apart from the five aforementionned
routines, the segment contains the standard VDP access routines (VSBR,
VMBR, VSBW, VMBW, and VWTR), an extra VDP routine to write multiple
copies on a byte (VIBW) and a routine to write a word into the
Flash-EEPROM (BURNW). See [below](#STDMEM) for a complete description of
these.

####  Switching SRAM pages

We have touched on that above, when discussing the data segment. The
routine RAMPG lets you switch the current RAM page. It takes the number
of the desired page in R0, as a parameter.

       BLWP @RAMPG            

Be carefull that, after a call to RAMPG, the default page containing
your DSEG variables will be switched off. This means that you should
call RAMPG again to go back to RAM page 0 (or to your private RAM page)
when you are done with the buffer. This can be a little tricky if you
decided to place your workspace in the SRAM...when switching pages
you'll also switch workspaces! A way around this problem is to first
copy your workspace into the destination page with RAMCPY.

####  Internal SRAM structure

You might be tempted to access memory directly in a SRAM page, at
\>5000-5FFF. This is a bit dangerous to do, however, because you cannot
be sure what portion of this memory might be used by a different DSR. If
you decide to to it anyhow, you should be aware that the system expects
the SRAM to have a predefined internal structure.

All SRAM pages are structured in a linked-blocks chain. The first word
in the block points at the start of the next block, etc. If a block is
free, the pointer has its \>8000 bit set. Additionally, the first block
in each page contains a common stub installed by the default power-up
routine: it holds the workspace for page-switching routines, as well as
some necessary data.

For instance, every SRAM page should begin with the following stub:

    >5000  DATA ENDSTU                       Point to next block
           DATA >0001                        Page number
           DATA 0,1,2,3,4,5,6,7,8,9,10,11    Workspace for page switching routines (R0-R11)
           DATA >1D0A                        Address of CRU bit 5 (also R12)
           DATA 13,14,15                     End of workspace (R13-R15)
           BSS  26                           Reserved, 13 words
    ENDSTU DATA >5FFF+>8000                  Free upto end of page (>5040-5FFF)

N.B. In all pages, the area \>5FF0-5FFF is only accessible when CRU bit
1 is '0', when this bit is '1' the USB controller maps there. In page 0,
the area \>5ED0-5FEF is used for the workspaces of by exported routines.

If you were to reserve 512 bytes with MALLOC, the pointer at ENDSTU
would change as follows:

    ENDSTU DATA NEWEND                       Point to next block
    FORYOU BSS  512                          Space reserved for you
    NEWEND DATA >5FFF+>8000                  Free to end of page
In this example, the address labelled FORYOU would be returned by MALLOC
in the R1 register.

Calling FREE will result in merging the NEWEND and ENDSTU pointers,
reverting to the first example above.

------------------------------------------------------------------------

ISP1161 access routines

If you feel like using them, I have a set of low-level routines to
communicate with the USB chip, the ISP1161. To use them simply include
the file ISP1161/S inside your program with a "copy" statement:
`COPY "DSK1.ISP1161/S"`. Optionally, you can include it within the STUB
segment, so that they will be available to every segment. A PC text
version of this file is available here: [ISP1161.TXT](isp1161.txt). In
addition, here is a small demo program [USBTEST.TXT](usbtest.txt), which
makes use of the above routine to test the USB ports: just connect the
upstream port to downstream port \#1 (the bottom one) and you'll be able
to verify that the host controller and the device controller can talk to
each other through a USB cable. Alternatively, here is a [zip
file](usbdsr1.zip) containing both the text versions and DF80 files to
be transfered to your TI-99/4A.

Host Controller routines:

    HCR    Host Controller Read   (a.k.a. HCR2)
    HCW    Host Controller Write   (a.k.a. HCW2)
    HCWI   Host Controller Write Immediate  (a.k.a. HCWI2)
    HCWIM  Host Controller Write Immediate Multiple

    ATRL   ATL Read
    ATLW   ATL write
    ITLR   ITL Read
    ITLW   ITL Write

Device controller routines:

    DCA    Device Controller Action
    DCR    Device Controller Read
    DCW    Device Controller Write
    DCWI   Device Controller Write Immediate
    DCWIM  Device Controller Write Immediate Multiple
    DCWEPCF  Device Controller Write EndPoint Configuration

    EPST   EndPoint Status
    EPCHK  EndPoint Check status image
    EPERR  EndPoint Error
    EPCFR  EndPoint Configuration Read
    EPSTAL EndPoint Stall
    EPUNST EndPoint Unstall
    EPCLR  EndPoint Clear
    EPVAL  EndPoint Validate
    EPR    EndPoint Read
    EPW    EndPoint Write

####  Host controller read

       DATA register              DATA register   

This routine is used to read from a host controller register. The data
read will be placed into R1 (or in R1 and R2, for 32-bit registers)
after due byte swapping, so that the MSB is in R1's MSB.

HCR2 is an alias for the same routine. It can be used as a reminder,
when addressing 32-bits registers. However, the number of bytes
transfered is detemined by the register number, not by the name of the
routine: registers under \>0020 are 32-bit.

The data word *register* should contain a number from \>0000 through
\>003F, indicating the register number. If the \>0080 flag is present,
it will be removed.

Alternatively, *register* can be a number from \>0100 through \>010F. In
which case, it indicates a workspace register (from 0 through 15) into
which the number of the host controller register is to be found.

       BLWP @HCR           read host controller register into R1
       DATA &gt;0203          register number is in R3           

In case you want the results to go somewhere else than into R1, you may
use the following syntax:

       DATA address,register       

Here, *address* is a pointer to a cpu location, in the range
\>0200-FFEF, into which the contents of the host controller register
will be read. Alternatively, if *address* is in the range \>FFF0-FFFF,
the data will be placed into worspace register R0 through R15.

       DATA &gt;AC02,&gt;0027    Place contents of register &gt;27 into &gt;AC02 `
       BLWP @HCR
       DATA &gt;FFF5,&gt;0027    Ditto, but place them into R5

####  Host Controller Write

       DATA register              DATA register   

This routine writes data from R1 into the host controller's register
specified in the data word. See above for details on *register*. In case
the register is 32-bit, R1 will supply the most significant word and R2
the least significant word. The alias HCW2 can be used as a mnemonic for
32-bit operations, but actually it's the register number that determines
the number of words.

Just like HCR, there is an alternative syntax for HCW that lets you
specify an address (or a workspace register) for the source, other than
R1 and R2:

       DATA address,register          DATA address,register            

####  Host Controller Write Immediate

       DATA register,data(,data)       DATA register,data,data    

This routines takes the value to write into the host controller from the
data word following the register number. For 32-bit registers, two data
words are required. You can use the alias HCWI2 to remind yourself of
this fact.

Exemple:

       DATA &gt;0028,&gt;1234           

####  Host Controller Write Immediate Multiple

       DATA register,data(,data)
      (DATA register,data,(data))
       DATA &gt;0000           

This routine is usefull when you need to set a whole bunch of registers
at a time, for instance during initialization. It is followed with a
list of register numbers together with the data to be placed into this
register. The list is terminated with a \>0000 word (register 0 is
read-only). Remember that 32-bit registers require two words of data!

####  Transfer List Write
These routines are used to write data to the ATL (acknowledged transfer
list) stack, or to the ITL (isochronous transfer list) stack currently
in used.

**R1** should contain a pointer to the data to be written.
**R2** should contain the number of bytes to write, rounded up to the
next even number.

Either routine copies R2 into the TransferCounter register, then passes
the relevant number of words to the required stack.

####  Tansfer List Read
These are the mirror routines from the above ones: they read back data
from the ATL or ITL stack, respectively. The number of bytes to read is
passed to the host controller via the TransferCounter register.

**R1** should point to a buffer large enough to accomodate the data.
**R2** should contain the number of bytes to read, rounded up to the
next even number.
Ok, now for the device controller...

####  Device Controller Action

       DATA register            

This routine is used to send a command to the device controller, by
passing a register number without actually passing any data to/from it.
As no data is passed, R1 is not affected, and there is no alternative
syntax using an address.

*Register* is a the number of a device controller register, from \>00 to
\>FF. Just like with host controller routines, values in the range
\>0100-010F indicates that the register number is to be found in the
rightmost byte of the corresponding workspace register: \>0100 is R0,
\>0101 is R1, etc.

####  Device Controller Read

       DATA register                DATA address,register   

This routine is similar to the HCR routine described above, except that
it addresses the device controller instead. With the first syntax, the
contents of the specified register will be placed into R1. With the
second, they will be placed at the specified address.

Here also, if *register* is in the range \>0100-010F, it means that the
register number is to be taken from workspace register R0 through R15,
respectively.

Similarly, an *address* in the range \>FFF0 through \>FFFF indicates
that the target is workspace register R0 through R15.

####  Device Controller Write

       DATA register                DATA address,register  

This routine sets a register in the device controller, taking its value
from R1 or from the specified address, respectively.

####  Device Controller Write Immediate

       DATA register,data(,data)       DATA register,data(,data)   

This routines takes the value to write into the host controller from the
data word following the register number. For the 32-bit InterruptEnable
register, two data words are required.

####  Device Controller Write Immediate Multiple

       DATA register,data(,data)
      (DATA register,data,(data))
       DATA &gt;0000           

This routine is usefull when you need to set a several registers at a
time. It is followed with a list of register numbers together with the
data to be placed into this register. The list is terminated with a
\>0000 word (register 0 is stack access). Remember that the
InterruptEnable register requires two words of data!

####  Device Controller EndPoint Configure

       BYTE cfg,...,cfg                   DATA address     

This routine is used to set the configuration of the endpoints. As per
the device controller design, you must always set all 16 endpoints
together, in numerical order, hence there must be exactly 16 bytes of
data. The first two endpoints have preset values: the first two bytes
must be \>C3 and \>83.

The alternative syntax provides an address (which must not be \>C383) at
which the 16 bytes of data are to be found.

Since most device controller operations address endpoints, I have also
provided a set of endpoint-oriented routines:

####  EndPoint Status

       DATA endpoint            

This routine reads the status of the specified enpoint into R1.

The data word endpoint should be a number between 0 and 14. However,
since there are two enpoints 0 (one for input, one for output), there is
an ambiguity here. Therefore, the following convention is to be used:

- To specify endpoint 0 input, which receives OUT and SETUP packets from
  the host, use \>0020. Mnemonic: the host writes "to" the endpoint.
- To specify endpoint 0 output, from where data is read by the host with
  IN packets, use \>00C0. Mnemonic: the host "sees" the data inside the
  endpoint.

Optionally, the same conventions can be applied to the other endpoints.
Use \>002x for endpoint you programmed as inputs, and \>00Cx for those
you programmed as outputs. The routines that are direction-sensitive
(i.e. write, read, clear and validate) will ignore operations going in
the wrong direction. This is important as such an error could lock-up
the device controller.

Alternatively, *endpoint* can be a number from \>0100 to \>010F, which
specifies the workspace register (R0 through R15) into which the
endpoint number is to be found.
In case you don't want the data to go into R1, an alternative syntax is
provided:

       DATA address,endpoint    

Where *address* is a pointer to a cpu memory location, in the range
\>0200-FFEF, where the data will to placed. In case address is in the
range \>FFF0-FFFF, the data will be placed in workspace register R0
through R15.

Exemples:

       DATA &gt;0020             `
       BLWP @EPST             Place status of endpoint #0-output into &gt;C840
       DATA &gt;C840,&gt;00C0`
       LI   R3,&gt;0005          Specify endpoint number
       BLWP @EPST             Place satus of endpoint #5 into R1
       DATA &gt;0103             Get endpoint number from R3 `
       LI   R10,&gt;C840         Specify target
       BLWP @EPST             Place status of endpoint #6 into &gt;C840
       DATA &gt;FFFA,&gt;0006       Get destination from R10`
       LI   R3,&gt;0005          Specify endpoint number
       LI   R8,&gt;C840          Specify target
       BLWP @EPST             Place status of endpoint #6 into &gt;C840
       DATA &gt;FFF8,&gt;0103       Get destination from R8, endpoint from R3       

####  EndPoint Check

       DATA endpoint                DATA address,endpoint  

This routine is equivalent to EPST, except that it accessed the
StatusImage register for the given endpoint. This lets you read the
status without resetting the corresponding bit in the Interrupt
register.

The first version reads the status into R1, the second into the
specified address.

####  EndPoint Error

       DATA endpoint                DATA address,endpoint  

This routine read the error register for the specified endpoint. The
first version places it into R1, the second into the specified address.

####  EndPoint Configuration Read

       DATA endpoint                DATA address,endpoint  

This routine is used to read back the configuration of an endpoint. The
first version reads it into R1, the second into the specified address.

####  EndPoint Stall

       DATA endpoint        

This routine stalls the specified endpoint. This is a command, so no
data is transfered. Thus R1 is not affected and there is no alternative
syntax using an address.

####  EndPoint Unstall

       DATA endpoint       

This routine unstalls the specified endpoint. No data is transfered, so
R1 is not affected and there is no alternative syntax using an address.

####  EndPoint Clear

       DATA endpoint           

This routine empties the stack for the corresponding endpoint. This is a
direction-specific routine: only input endpoints should be cleared. If
the output flag \>00Cx is specified, the routine will do nothing
(actually, only the \>004x bit is checked, the \>0080 bit is ignored).

No data is passed with this routine, so R1 is not affected and there is
no alternative syntax using an address.

####  EndPoint Validate

       DATA endpoint          

This routine is used to tell the device controller that the data placed
into an output endpoint may be sent to the host upon reception of the
next IN packet. This is a direction-specific routine: you should never
validate an endpoint programmed as input. If the input flag \>002x is
specified in the endpoint number, the routine will return immediately,
doing nothing.

No data is passed with this routine, so R1 is not affected and there is
no alternative syntax using an address.

####  EndPoint Write

       DATA endpoint       

This routine places data into an endpoint programmed for output (i.e. to
answer IN packets from the host).

**R1** should contain a pointer to the data.
**R2** should contrain the number of data bytes to be written.

The routine places R2 on the endpoint's stack, followed with the data
bytes read from R1. It does not verify that R2 is in the legal range for
this endpoint.

####  EndPoint Read

       DATA endpoint           

This routine reads data from an input endpoint's stack into a cpu
buffer.

**R1** should contain a pointer a buffer large enough to accomodate the
data
**R2** should contain the maximum number of bytes that can be
transfered. Upon return, R2 will contain the number of bytes in the
endpoint.

The routine first reads the number of bytes waiting inside the
endpoint's stack. If this number is smaller than R2, all the bytes are
transfered to the buffer specified by R1 and R2 is updated to reflect
the number of bytes actually transfered.

It the number of bytes on the stack is greater than R2, only the number
specified in R2 will be transfered. R2 will then be updated to reflect
the total number of bytes in the endpoint's stack (not counting the two
size bytes).

------------------------------------------------------------------------

SmartMedia access routines

These routines will let you perform basic read/write/erase functions
with SmartMedia cards. If you feel like using them, include the file
SM/S within your program. with a `COPY "DSKn.SM/S"`. Tip: if you want
these routines to be accessible from any of your segments, make the file
part of the STUB segment. Here is the file, in PC text format:
[SM.TXT](sm.txt) alternatively, here is a [zip file](usbdsr1.zip)
containing both the text version and a DF80 file to be transfered to
your TI-99/4A.

    SMR    SmartMedia Read
    SMV    SmartMedia Verify
    SMVB   SmartMedia Verify Byte
    SMW    SmartMedia Write
    SME    SmartMedia Erase
    SMRX   SmartMedia Read eXtra bytes
    SMWX   SmartMedia Write eXtra bytes
    SMID   SmartMedia Read card ID

Usefull tips:

- Before you begin, check wether a SmartMedia card is inserted with TB
  6.
- Before writing or erasing, check wither the card is write-protected
  with TB 7. Remember that the write-protection may or may not be
  enforced by the hardware, depending on whether the user closed the
  relevant DIP-switch.
- By default, these routines use a workspace located in the SRAM space,
  at \>5FD0, but you can change this.
- All routines expect the USB-SM board's CRU in R12. They enable port
  mapping with SBO 1 and enable the SmartMedia card with SBO 3. None of
  them performs a SBZ 1 nor SBZ 3 when done.

#### SmartMedia Read SMR

This routine lets you read a number of bytes from the SmartMedia card.

**R0**: Pointer to SmartMedia address.
**R1**: Pointer to a buffer in CPU memory.
**R2**: Number of bytes to read. This may be more than one sector:
reading will continue with the extra information bytes for the current
sector, then with the next sector, its info bytes, etc.

A SmartMedia address should consist in three words: two for the sector
number, and one for the byte offset where to start reading within the
sector. Normally, this will be zero, but you can elect the start reading
anywhere within a sector.

    SMADR  DATA >0000,>0000     Sector number
           DATA >0000           Byte offset

The trouble with SmartMedia cards is that some use 2 bytes for the
sector number, others use 3, or even 4. This is a problem, because older
cards are thrown off by extra address bytes. Therefore, you should first
check the card ID (and extended ID) to determine its addressing scheme.
Another way is trial and error: passing the wrong number of address
bytes generally results in reading only \>00 bytes.

Once you know how many bytes to use, follow these rules:

- If the first byte of the sector number is \>FF, it will not be passed
  to the SmartMedia card (i.e. 3-byte cards).
- If the first word of the sector number is \>FFFF, it will not be
  passed to the card (i.e. 2-byte cards).

Another discrepancy between cards is that some have 256-byte sectors
(plus 8 extra bytes), whereas others have 512-byte sectors (plus 16
extra bytes). And how is that for standardization? Thus, the byte offset
can be a number between 0 and 255, or between 0 and 511, depending on
the card.

For your convenience, provision was made for the routine to treat 512
bytes/sector cards as if they had 256 bytes/sector. This is based on the
fact that most cards let you write a sector in at least two chunks. To
trigger this feature, make the first byte of the offset \>FF. This will
cause the sector number to the divided by two before it's passed to the
card. The remainder of the division is used to decide wether to access
the first half, or the second half of the 512-byte sector.

Example:

       LI    R1,BUFFER
       LI    R2,256
       BLWP  @SMR`
SMADR  DATA  &gt;FF00,&gt;1234         3-byte sector number
       DATA  &gt;0000               start at beginning of sector
BUFFER BSS   256                 buffer space   

####  SmartMedia Verify SMV

**R0**: Pointer to SmartMedia address.
**R1**: Pointer to a buffer in CPU memory.
**R2**: Number of bytes to check.

This routine works just like SMR, except that it does not place the
bytes it reads into the buffer. Rather, it compares them with the
contents of the buffer. If all match, the routine returns with the Eq
bit set, a condition which can be trapped with JEQ.

If a mismatch is detected, the routine returns with the Eq bit reset
(use JNE to trap it), and R1 pointing at the first offending byte in the
buffer.

Example:

       LI    R1,BUFFER
       LI    R2,&gt;0006
       BLWP  @SMV
       JEQ   OK
       JNE   UHOH`
SMADR  DATA  &gt;FFFF,&gt;1234         2-byte sector number
       DATA  &gt;0000               start at beginning of sector
BUFFER DATA  &gt;0102,&gt;0204,&gt;0506   data to verify

####  SmartMedia Verify SMVB

**R0**: Pointer to SmartMedia address.
**R1**: Value to verify (in left byte)
**R2**: Number of bytes to check.

This routine works just like SMV, but it expects all bytes to match a
given value. It comes handy to check whether a sector is blank, i.e.
contains only \>FF bytes.

If all bytes match, the routine returns with the Eq bit set, a condition
which can be trapped with JEQ. If a mismatch is detected, the routine
returns with the Eq bit reset (use JNE to trap it), and R2 will contain
the number of the first offending byte (counting from zero on).

Example:

       LI    R1,&gt;FFFF
       LI    R2,&gt;0100
       BLWP  @SMVB
       JEQ   OK
       JNE   UHOH`
SMADR  DATA  &gt;FFFF,&gt;1234         2-byte sector number
       DATA  &gt;0000               start at beginning of sector

####  SmartMedia Write SMW

**R0**: Pointer to SmartMedia address.
**R1**: Pointer to a buffer in CPU memory.
**R2**: Number of bytes to check. Maximum: 256+8 bytes or 512+16 bytes,
depending on the card.

This routine is used to write to the SmartMedia card. Generally, it is
best to write one sector at a time. It is impossible to write more than
one sector, because the card won't accept more bytes than fit in a
sector plus its extra information bytes. It is possible to write less
than a sector, but be aware that it's not reliable to write more than
twice to the same sector. So if you don't cover the whole sector with
two write operations, the remaining bytes should remain untouched.

Once it is done writing, the routine performs a status check. If
everything went well, it returns with the Eq bit set. If the error flag
was raised in the status byte, the routine returns with the Eq bit
reset. This may happened if the card is write protected, for instance.
The routine will not return until the card signals that it is ready to
continue.

Example:

       LI    R1,BUFFER
       LI    R2,256
       BLWP  @SMW
       JNE   ERROR`
SMADR  DATA  &gt;0012,&gt;3456         4-byte sector number
       DATA  &gt;FF00               treat card as a 256 bytes/sector card
BUFFER DATA  &gt;0102,&gt;0204,etc     data to write     

####  SmartMedia Read eXtra bytes SMRX

**R0**: Pointer to SmartMedia address.
**R1**: Pointer to a buffer in CPU memory.
**R2**: Number of bytes to read.

This routine works just like SMR, except that it reads only the extra
info bytes associated with the requested sector. Cards with 256-byte
sectors will have 8 extra bytes per sector, whereas 512 bytes/sector
cards have 16 extra bytes. You can read more that this number, however,
since reading will continue with the extra bytes of the next sector(s).

Note that this routine will never read data from the sector itself. It
is thus usefull to determine the sector size: read 520 bytes with SMR,
then 8 with SMRX and check wether these 8 bytes match bytes 256 through
261, or to bytes 512 through 519.

Example:

       LI    R1,BUFFER
       LI    R2,15
       BLWP  @SMRX`
SMADR  DATA  &gt;FF00,&gt;1234         3-byte sector number
       DATA  &gt;0001               start at second extra byte (dummy example)
BUFFER BSS   16                  buffer space           

####  SmartMedia Write eXtra bytes SMWX

**R0**: Pointer to SmartMedia address.
**R1**: Pointer to a buffer in CPU memory.
**R2**: Number of bytes to write (8 or 16, depending on the card).

This routine is used to write the extra information bytes associated
with the specified sector. It is generally best to write all of them at
a time, and not to exceed the maximum (although some card may accept
more, I'm not sure if they get written properly). Another solution is to
write the extra bytes together with the sector data, using SMW instead
of SMWX.

Just like SMW, this routine performs a status check when done and
returns with the Eq bit set if everything went well, and with the Eq bit
reset if the status reported an error.

Example:

       LI    R1,BUFFER
       LI    R2,8
       BLWP  @SMWX
       JNE   ERROR`
SMADR  DATA  &gt;FF00,&gt;0002                 3-byte sector number
       DATA  &gt;0000                       start from byte 0
BUFFER DATA  &gt;FFFF,&gt;00FF,&gt;FF01,&gt;0203     data to write        

####  SmartMedia Erase SME

**R0**: Pointer to SmartMedia address.

This routine is used to erase a block of sectors. Depending on the card
(again!), there may be 16 or 32 sectors per block. The SmartMedia
address should consist in two words containing the number a a sector
within the block to erase. No need for a byte offset in this case.

Once done, the routine performs a status check and returns with the Eq
bit set if everything went well. If an error occured (most likely, the
card being protected), it returns with the Eq bit reset.

Example:

       BLWP  @SME
       JNE   ERROR`
SMADR  DATA  &gt;FFFF,&gt;1234         2-byte sector number         

####  SmartMedia Identification SMID

**R0**: Pointer to SmartMedia address, which should be zero.
**R1**: Will contain manufacturer & product ID
**R2**: Will contain extended ID, if available.

This routine queries the card for its ID and places it into R1. The
manufacturer ID will be in the most significant byte, the product ID in
the least significant byte. In addition, it also checks the extended ID
code and places it into the most significant byte of R2. If the card
does not support the extended ID function, R2 is most likely to contain
a \>00 byte, although this is not guaranteed.

ID data is usefull to identify the SmartMedia card currently inserted
and decide of its characteristics:

- How many bytes in a sector address?
- How many bytes per sector?
- How many sectors per block?
- Does it support multiple write/erase?
- What's the size of the card?

You will need to answer at least the first 3 questions to properly
access the card.

Example:

       BLWP  @SMID
SMADR  DATA  &gt;FF00,&gt;0000         3-byte sector number: sector 0
       DATA  &gt;0000               byte offset should be zero too          

If the card returns an ID that you don't know, you'll have to answer the
above questions by trial and error. This is easier to do if the card is
blank, as there is no concern about loosing data in the process...

Number of address bytes: Try five, then four, then three with a SMVB
operation on sector 0. If it reads as all \>00, then you probably don't
have the right number of address bytes (unless of course the card is not
blank, and sector zero was filled with \>00s).

Bytes per sector: Read and save 16 info bytes for sector 0 with SMRX.
Write an 8-byte test string to sector 0 info area with SMWX. Read 520
bytes from sector 0 with SMR. Check if your test string appears at bytes
256-263, or at bytes 512-530. Restore 8 or 16 info bytes (depending on
the sector size) with SMWX.

Sectors per block: Save the first 32 sectors with SMR (optional). Write
a test string to sector 17 with SMW. Erase sector 0 with SME. Check if
sector 17 became all \>FF with SMVB, if not there are 16 sectors per
block. Restore 16 or 32 sectors with multiple calls to SMW (optional).

------------------------------------------------------------------------

## Standard memory routines

These routines are to be found in a file named USBSTD/O, which is to be
loaded with the DSR loader. The file defines a number of exports that
you can subsequently REFerenced within your program, and call with BLWP
@SGBLWP.

Technical note: most of these routines are all "level 1" routines (i.e.
do not call other exports), and use L1WS as their workspace. The
exceptions being NEWWS and BURNW which use L2WS as their workspace.

    VSBR   VDP Single Byte Read
    VMBR   VDP Multiple Bytes Read
    VSBW   VDP Single Byte Write
    VMBW   VDP Multiple Bytes Write
    VIMW   VDP Identical Bytes Write
    VWTR   VDP Write To Register

    BURNW  Burn Word into Flash-EEPROM

    MALLOC Memory Allocation in SRAM
    FREE   Free previously allocated memory
    RAMCPY Copy to/from SRAM
    NEWWS  Switch to a new workspace in SRAM, optionally copy from old one
    TOSLOT Save R0 and R1 into our private slot
    ATSLOT Retrieve R0 and R1 from our private slot

####  VDP Single Byte Read VSBR

**R0**: Address in VDP memory, range \>0000-3FFF.
**R1**: Most significant byte will contain the byte read from the
specified VDP location.

Exemple:

       LI   R0,&gt;0020
       LI   R1,&#39;A &#39;
       BLWP @SGBLWP
       DATA VSBW,PG4SEG

####  VDP Multiple Byte Read VMBR

**R0**: Address in VDP memory.
**R1**: Pointer to a buffer in CPU memory, large enough to accomodate
all the data.
**R2**: Number of bytes to read (\>0001-7FFF only, invalid values
abort).
**R3**: SRAM page \# (only relevant if R1 points to the SRAM).

####  VDP single Byte Write VSBW

**R0**: Address in VDP memory, range \>0000-3FFF.
**R1**: Most significant byte contains the byte to be written at the
specified VDP location.

####  VDP Multiple Byte Write VMBW

**R0**: Address in VDP memory.
**R1**: Pointer to a buffer in CPU memory, containing the data.
**R2**: Number of bytes to write (\>0001-7FFF only, invalid values
abort).
**R3**: SRAM page \# (only relevant if R1 points to the SRAM).

####  VDP Identical Bytes Write VIBW

**R0**: Address in VDP memory.
**R1**: Most significant byte contains the byte to be written at all the
specified VDP location.
**R2**: Number of repeats.

Exemple:

       CLR  R0              start from &gt;0000
       LI   R1,&#39;  &#39;         fill with spaces
       LI   R2,&gt;0300        24 lines of 32 bytes
       BLWP @SGBLWP         clear screen
       DATA VIBW,PG4SEG`
    

####  VDP Write To Register VWTR

**R0**: MSB contains register number (\>00-07), LSB contains register
value.

Exemple:

       LI   R0,&gt;01F0       target: VDP register 1
       BLWP @SGBLWP        set text mode
       DATA VWTR,PG4SEG   

####  BURNW

This routine can be used to write a word to the Flash-EEPROM containing
the DSRs. Remember that the user can open a DIP-switch that will prevent
writing to occur, so make sure you prompt him/her for closing that
switch.

**R0**: EEPROM page number (0-7FF).
**R1**: Address in EEPROM memory (\>4000-4FFF)
**R2**: Data word to write.

The routine returns its results in the status register. You can trap it
with the following instructions:

     JEQ    All went well
     JNE    Something went wrong
     JGT    EEPROM complained of an error (e.g. write-protected)
     JLT    Result doesn't match expected value (e.g. '0' bits can't become '1')

Exemple:

       LI   R0,&gt;0005      page 5
       LI   R1,&gt;4822      target address
       LI   R2,&gt;1234      value to write
       BLWP @SGBLWP
       DATA BURNW,PG4SEG
       JNE  ERROR         trap errors        

####  MALLOC

This routine allocates memory space within a SRAM page.

**R0**: Number of bytes needed. Upon return: SRAM page number.
**R1**: Upon return: Address of the required memory.

Upon return, the Eq bit will be set if all went well. You can thus trap
the result with the following instructions:

     JEQ   All went well
     JNE   Error (e.g. memory full)

See [above](#malloc&free) for examples.

####  FREE

This routine is used to free previously allocated memory.

**R0**: SRAM page number.
**R1**: Address in SRAM

Upon return, the Eq bit will be set if all went well.

     JEQ   All went well
     JNE   Error (e.g. wrong page number or address)

####  RAMCPY

This routine copies memory from one page to another within the SRAM, or
to the general CPU memory.

**R0**: Source page number (\>FFFF = current SRAM page).
**R1**: Source address.
**R2**: Number of bytes to copy.
**R3**: Destination page number (\>FFFF = current SRAM page).
**R4**: Destination address.

A page number is only relevant if the corresponding address is in the
range \>5000-5FFF.

NB The routine makes uses a workspace located at \>8300. The contents of
these addresses are saved and restored once the routine is done.

Caution: never use RAMCPY to copy data from the EEPROM: since the
routine is most probably located in a different page than your segments,
the EEPROM data would not be available when copying. You would end up
copying data from the page then RAMCPY is!

#### NEWWS

This routine lets you change workspace, and optionally copy the old one,
to a new workspace located in another SRAM page.

           BLWP @SGBLWP
           DATA NEWWS,PG4SEG
           DATA size

**R0 lsb**: Page for new workspace (most significant byte ignored).
**R1**: New workspace address. R0 and R1 are best obtained by a call to
MALLOC.
***size***: Number of bytes to copy from old workspace to new one (if
any), 32 copies the whole workspace, higher values let you copy extra
data words following your workspace.

When this routine is called, the current SRAM page is placed into the
most significant byte of R0 and the current worspace address is
substracted from R1 (i.e. R1 now contains the differential offset
between the two workspace). If you copy at least 4 bytes into the new
workspace, you will be able to use R0 and R1 to return to the old
workspace, using an alternative syntax:

           BLWP @SGBLWP
           DATA NEWWS,PG4SEG
           DATA size + >8000

**R0 msb**: Page for new workspace (least significant byte ignored).
**R1**: Offset to substract from current workspace pointer to get new
workspace address.
***size***: Number of bytes to copy (if any), plus a \>8000 flag bit to
indicate alternative syntax. In this case, R0 and R1 are left unchanged
by the call.

Exemple:

       LWPI WREGS          current workspace
       LI   R0,36          obtain address for new workspace + 2 words
       BLWP @SGBLWP
       DATA MALLOC,PG4SEG
       JNE  ERROR
       BLWP @SGBLWP
       DATA NEWWS,PG4SEG   switch to new workspace
       DATA 36             copying workspace + 2 data words
 *      CLR  @THIS          this won&#39;t work !!!
       CLR  @THIS(R1)      access the copies
       SETO @THAT(R1)`
       BLWP @SGBLWP
       DATA NEWWS,PG4SEG   switch back to old workspace
       DATA &gt;8000          don&#39;t copy anything back (although we could)
       CEND
* Data segment
       DSEG
WREGS  BSS  32             initial workspace
THIS   DATA 0              variables
THAT   DATA 0
OTHER  DATA 0
       DEND     

If you elect to copy extra data words after your workspace, be aware
that their address may vary according to what MALLOC returned. In the
exemple above, let's say your initial workspace was at \>5040 in page 0;
this means that THIS was at \>5060, and THAT at \>5062. Now let's say
that MALLOC found free space at \>5068 in page 3; this means that THIS
will be copied at \>5088 and THAT at \>508A.

Because the address of the copies is different from that of the
original, you cannot just write `CLR @THIS` or `SETO @THAT`: such
instructions would be hard-coded as `CLR @>5062` and `SETO @>5064`,
which are the correct addresses in SRAM page 0, but who knows what's to
be found there in page 3! By using R1 as an index, you can correct the
addresses and make sure the copies of THIS and THAT are properly
accessed. Of course, you may copy this index to another register, less
frequently used than R1.

Note that if you change workspace twice in a row, you will need to add
up the offsets returned into R1 by each call to NEWWS, so that the
addresses remains correct. For instance, save R1 into R7 after the first
call, and add R1 to R7 for each successive call. To return to the
original workspace, you will also need to save R0 msb after the first
call.

####  TOSLOT

This routine stores R0 and R1 into your private memory slot. The address
of the slot in RAM page 1 should follow the call. This address is
obtained through the REFed label MYSLOT.

**R0**: data to be saved
**R1**: data to be saved

####  ATSLOT

This routine retrieves the contents of your private slot into R0 and R1.
It must be followed with a reference to the MYSLOT label.

**R0**: data retrieved
**R1**: data retrieved

NB This routine and the previous one actually call RAMCPY with preset
parameters.Their main usage is to save the location of a private memory
area, obtained with MALLOC. You can then retrieve this location the next
time your DSR is called.

Example:

       BLWP @SGBLWP         call routine in another page
       DATA ATSLOT,PG4SEG   retrieve contents of our slot
       DATA MYSLOT
       MOV  R0,R0           check if empty (RAM page is never 0)
       JNE  SK1             we already set our private memory
       LI   R0,&gt;0100        size of the private area we need
       BLWP @SGBLWP
       DATA MALLOC,PG4SEG   obtain it: R0 = page, R1 = address
       JNE  ERROR
       BLWP @TOSLOT         save R0 and R1 into our slot
       DATA MYSLOT
       B    *11
SK1    LI   R2,&gt;0100        copy private area (256 bytes)
       SETO R3              into current SRAM page
       LI   R4,HERE         at this location
       BLWP @SGBLWP
       DATA RAMPCY,PG4SEG
       B    *11
Revision 1. 6/24/03 Preliminary

Revision 2. 9/17/03 Added quote marks with CSEG
Revision 3. 11/12/03 Added ISP1161 routines and standard memory
routines.
Revision 4. 12/16/03 Added MYSTUB segment and SmartMedia routines.
Revision 5. 1/19/04 Added EEPROM stucture and segment order. Plus
cosmetic changes.

Revision 6. 1/26/04 Added SGBLX, etc. Changed RAMPG call to BLWP.
Revision 7. 2/9/04 Added slot concept, device name in info, changed
removal info.
Revision 8. 3/1/04 Added notes on workspaces. Fixed calls to memory
subroutines.

Revision 9. 5/5/04 Added SMVB and note on determinige SM card
characteristics.

Revision 10. 5/13/04 Added source files for SM and ISP1161 routines.
Revision 11. 1/2/05 Added R3 for VMBR and VMBW.
Revision 12. 2/6/05 Added NEWWS routine.

[Next page](usb_load.md) (DSR loader and manager)
[Previous page](usb2.md) (Programming the USB-SM card)

[Back to the TI-99/4A Tech Pages](titechpages.md)
